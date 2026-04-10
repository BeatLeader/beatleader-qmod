#include "include/API/PlayerController.hpp"
#include "include/Utils/WebUtils.hpp"
#include "include/Utils/ModConfig.hpp"
#include "include/Utils/StringUtils.hpp"
#include "include/main.hpp"
#include "include/UI/LeaderboardUI.hpp"

#include "GlobalNamespace/IPlatformUserModel.hpp"
#include "GlobalNamespace/UserInfo.hpp"
#include "GlobalNamespace/PlatformLeaderboardsModel.hpp"

#include "System/Action_1.hpp"
#include "System/Threading/Tasks/Task.hpp"
#include "System/Threading/Tasks/Task_1.hpp"

#include "UnityEngine/Resources.hpp"

#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "beatsaber-hook/shared/config/rapidjson-utils.hpp"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <thread>

using UnityEngine::Resources;
using namespace GlobalNamespace;
using namespace rapidjson;

optional<Player> PlayerController::currentPlayer = nullopt;
string PlayerController::lastErrorDescription = "";
vector<function<void(optional<Player> const&)>> PlayerController::playerChanged;

namespace {
    constexpr int kMaxRefreshRetries = 5;
    constexpr int kMaxHistoryRetries = 2;
    constexpr auto kBackgroundRefreshRetryDelay = std::chrono::minutes(1);

    std::atomic_bool backgroundRefreshRetryScheduled = false;

    bool IsTransientRefreshFailure(long status, bool parseError) {
        if (status == 401) {
            return false;
        }

        if (parseError) {
            return true;
        }

        if (status == 0 || (status > 0 && status < 100)) {
            return true;
        }

        return status == 408 || status == 425 || status == 429 || status >= 500;
    }

    int GetRetryDelaySeconds(int retry) {
        return std::min(60, (retry + 1) * 10);
    }

    void CompleteRefresh(const function<void(optional<Player> const&, string)>& finished);
    void LoadPlayerHistory(string playerId, int contextId, int retry, const function<void(optional<Player> const&, string)>& finished);
    void ScheduleBackgroundRefreshRetry();
}

void callbackWrapper(optional<Player> const& player) {
    for (auto && fn : PlayerController::playerChanged)
        fn(player);
}

namespace {
    void CompleteRefresh(const function<void(optional<Player> const&, string)>& finished) {
        if (finished) {
            finished(PlayerController::currentPlayer, "");
        }
        callbackWrapper(PlayerController::currentPlayer);
    }

    void LoadPlayerHistory(string playerId, int contextId, int retry, const function<void(optional<Player> const&, string)>& finished) {
        WebUtils::GetJSONAsync(
            WebUtils::API_URL + "player/" + playerId + "/history?leaderboardContext=" + to_string(contextId) + "&count=1",
            [playerId, contextId, retry, finished](long status, bool error, rapidjson::Document const& result) {
                if (status == 200 && !error && result.IsArray() && result.Size() > 0 && result[0].IsObject()) {
                    if (PlayerController::currentPlayer != nullopt && PlayerController::currentPlayer->id == playerId) {
                        PlayerController::currentPlayer->SetHistory(result[0]);
                    }

                    CompleteRefresh(finished);
                    return;
                }

                if (retry < kMaxHistoryRetries && IsTransientRefreshFailure(status, error)) {
                    int delaySeconds = GetRetryDelaySeconds(retry);
                    BeatLeaderLogger.warn(
                        "Failed to refresh player history for {} (status {} parseError {}), retrying in {}s",
                        playerId,
                        status,
                        error,
                        delaySeconds
                    );
                    std::this_thread::sleep_for(std::chrono::seconds(delaySeconds));
                    LoadPlayerHistory(playerId, contextId, retry + 1, finished);
                    return;
                }

                BeatLeaderLogger.warn(
                    "Failed to refresh player history for {} (status {} parseError {}), continuing without history",
                    playerId,
                    status,
                    error
                );
                CompleteRefresh(finished);
            }
        );
    }

    void ScheduleBackgroundRefreshRetry() {
        bool expected = false;
        if (!backgroundRefreshRetryScheduled.compare_exchange_strong(expected, true)) {
            return;
        }

        std::thread([] {
            std::this_thread::sleep_for(kBackgroundRefreshRetryDelay);
            backgroundRefreshRetryScheduled.store(false);
            PlayerController::Refresh(0, nullptr, true);
        }).detach();
    }
}

void PlayerController::Refresh(int retry, const function<void(optional<Player> const&, string)>& finished, bool keepTrying) {
    // Error Handler
    auto handleError = [retry, finished, keepTrying](long status, bool parseError) {
        if (status == 401) {
            backgroundRefreshRetryScheduled.store(false);
            currentPlayer = nullopt;
            callbackWrapper(currentPlayer);
            if (finished) {
                finished(nullopt, "Unauthorized");
            }
            return;
        }

        if (retry < kMaxRefreshRetries && IsTransientRefreshFailure(status, parseError)) {
            int delaySeconds = GetRetryDelaySeconds(retry);
            BeatLeaderLogger.warn(
                "Failed to refresh player (status {} parseError {}), retrying in {}s ({}/{})",
                status,
                parseError,
                delaySeconds,
                retry + 1,
                kMaxRefreshRetries + 1
            );
            std::this_thread::sleep_for(std::chrono::seconds(delaySeconds));
            Refresh(retry + 1, finished, keepTrying);
            return;
        }

        if (keepTrying && currentPlayer == nullopt && std::filesystem::exists(WebUtils::getCookieFile())) {
            BeatLeaderLogger.warn(
                "Failed to refresh player after {} attempts, scheduling another retry in {}s",
                retry + 1,
                std::chrono::duration_cast<std::chrono::seconds>(kBackgroundRefreshRetryDelay).count()
            );
            ScheduleBackgroundRefreshRetry();
        } else {
            BeatLeaderLogger.warn("Failed to refresh player (status {} parseError {})", status, parseError);
        }

        if (finished) {
            finished(currentPlayer, currentPlayer == nullopt ? "Failed to retrieve player" : "Failed to refresh player");
        }
    };

    // Replace beatleader.xyz in cookie file with beatleader.com if it exists
    string cookieFile = WebUtils::getCookieFile();
    if (std::filesystem::exists(cookieFile)) {
        FILE* file = fopen(cookieFile.c_str(), "r");
        if (file) {
            string contents;
            char buffer[1024];
            while (fgets(buffer, sizeof(buffer), file)) {
                contents += buffer;
            }
            fclose(file);

            // Replace all instances of beatleader.xyz with beatleader.com
            size_t pos = 0;
            string from = "beatleader.xyz";
            string to = "beatleader.com";
            while ((pos = contents.find(from, pos)) != string::npos) {
                contents.replace(pos, from.length(), to);
                pos += to.length();
            }

            // Write back to file
            file = fopen(cookieFile.c_str(), "w");
            if (file) {
                fputs(contents.c_str(), file);
                fclose(file);
            }
        }
    }

    // Get new userdata and refresh the interface with it
    WebUtils::GetJSONAsync(WebUtils::API_URL + "user/modinterface", [finished, handleError](long status, bool error, rapidjson::Document const& result) {
        if (status == 200 && !error && result.IsObject()) {
            backgroundRefreshRetryScheduled.store(false);
            currentPlayer = Player(result.GetObject());

            // Refresh the cookie to keep player logged in
            WebUtils::PostJSONAsync(WebUtils::API_URL + "cookieRefresh", "", [](long status, string error){ });

            // History is only used for rank-delta UI, so auth should remain successful even if this request fails.
            LoadPlayerHistory(currentPlayer->id, getModConfig().Context.GetValue(), 0, finished);
        }
        else {
            handleError(status, error || !result.IsObject());
        }
    });
}

void PlayerController::SignUp(string login, string password, const function<void(optional<Player> const&, string)>& finished) {
    lastErrorDescription = "";

    WebUtils::PostFormAsync(WebUtils::API_URL + "signinoculus", password, login, "signup",
                            [finished](long statusCode, string error) {
        if (statusCode == 200) {
            Refresh(0, finished);
        } else {
            lastErrorDescription = error;
            BeatLeaderLogger.error("BeatLeader {}",
                                ("signup error" + to_string(statusCode)).c_str());
            finished(nullopt, error);
        }
    });
}

void PlayerController::LogIn(string login, string password, const function<void(optional<Player> const&, string)>& finished) {
    lastErrorDescription = "";

    WebUtils::PostFormAsync(WebUtils::API_URL + "signinoculus", password, login, "login",
                            [finished](long statusCode, string error) {
        if (statusCode == 200) {
            Refresh(0, finished);
        } else {
            lastErrorDescription = error;
            BeatLeaderLogger.error("BeatLeader {}",
                                ("signup error" + to_string(statusCode)).c_str());
            finished(nullopt, error);
        }
    });
}

void PlayerController::LogOut() {
    WebUtils::GetAsync(WebUtils::API_URL + "signout", [](long statusCode, string error) {});
    remove(WebUtils::getCookieFile().data());

    currentPlayer = nullopt;
    callbackWrapper(currentPlayer);
}

bool PlayerController::IsFriend(Player anotherPlayer) {
    if (currentPlayer == nullopt) return false;

    return std::find(currentPlayer->friends.begin(), currentPlayer->friends.end(), anotherPlayer.id) != currentPlayer->friends.end();
}

bool PlayerController::InClan(string tag) {
    if (currentPlayer == nullopt) return false;

    auto it = std::find_if(currentPlayer->clans.begin(), currentPlayer->clans.end(), 
                           [&tag](const auto& clan) { return toLower(clan.tag) == toLower(tag); });

    return it != currentPlayer->clans.end();
}

bool PlayerController::IsMainClan(string tag) {
    if (currentPlayer == nullopt || currentPlayer->clans.size() == 0) return false;

    return toLower(currentPlayer->clans[0].tag) == toLower(tag);
}

bool PlayerController::IsIncognito(Player anotherPlayer) {
    Document incognitoList;
    incognitoList.Parse(getModConfig().IncognitoList.GetValue().c_str());

    if (incognitoList.HasParseError() || !incognitoList.IsObject()) {
        getModConfig().IncognitoList.SetValue("{}");
        return false;
    }

    auto incognitoArray = incognitoList.GetArray();

    for (int index = 0; index < (int)incognitoArray.Size(); ++index) {
        auto const& id = incognitoArray[index].GetString();
        if (strcmp(id, anotherPlayer.id.c_str()) == 0) {
            return true;
        }
    }
    
    return false;
}

void PlayerController::SetIsIncognito(Player anotherPlayer, bool value) {
    Document incognitoList;
    incognitoList.Parse(getModConfig().IncognitoList.GetValue().c_str());

    if (incognitoList.HasParseError() || !incognitoList.IsObject()) {
        getModConfig().IncognitoList.SetValue("{}");
        return;
    }

    auto incognitoArray = incognitoList.GetArray();
    
    rapidjson::Document::AllocatorType& allocator = incognitoList.GetAllocator();
    if (value) {
        Value rj_key;
        rj_key.SetString(anotherPlayer.id.c_str(), anotherPlayer.id.length(), allocator);
        incognitoList.PushBack(rj_key, allocator);
    } else {
        for (int idx = 0 ; idx < (int) incognitoList.Size() ; idx++) {
            if (strcmp(incognitoList[idx].GetString(), anotherPlayer.id.c_str()) == 0) {
                incognitoList.Erase(incognitoList.Begin() + idx--);
                break;
            }
        }
    }

    StringBuffer buffer;
    buffer.Clear();
    Writer<StringBuffer> writer(buffer);
    if (incognitoList.Accept(writer)) {
        string incognitoListString = string(buffer.GetString());
        getModConfig().IncognitoList.SetValue(incognitoListString);
    }
}

bool PlayerController::IsPatron(Player anotherPlayer) {
    return 
        anotherPlayer.role.find("tipper") != string::npos ||
        anotherPlayer.role.find("supporter") != string::npos ||
        anotherPlayer.role.find("sponsor") != string::npos;
}
