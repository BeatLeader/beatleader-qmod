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

using UnityEngine::Resources;
using namespace GlobalNamespace;
using namespace rapidjson;

optional<Player> PlayerController::currentPlayer = nullopt;
string PlayerController::lastErrorDescription = "";
vector<function<void(optional<Player> const&)>> PlayerController::playerChanged;

void callbackWrapper(optional<Player> const& player) {
    for (auto && fn : PlayerController::playerChanged)
        fn(player);
}

static map<LeaderboardUI::Context, string> contextToPlayerUrlString = {
    {LeaderboardUI::Context::Standard, "general"},
    {LeaderboardUI::Context::NoMods, "nomods"},
    {LeaderboardUI::Context::NoPause, "nopause"},
    {LeaderboardUI::Context::Golf, "golf"},
    {LeaderboardUI::Context::SCPM, "scpm"},
};

void PlayerController::Refresh(int retry, const function<void(optional<Player> const&, string)>& finished) {
    // Error Handler
    auto handleError = [retry, finished](){
        if (retry < 3) {
            Refresh(retry + 1, finished);
        } else {
            currentPlayer = nullopt;
            if (finished) finished(nullopt, "Failed to retrieve player");
        }
    };

    // Get new userdata and refresh the interface with it
    WebUtils::GetJSONAsync(WebUtils::API_URL + "user/modinterface", [retry, finished, handleError](long status, bool error, rapidjson::Document const& result){
        if (status == 200 && !error) {
            currentPlayer = Player(result.GetObject());
            // We also need the history so we can display the ranking change
            // Leaderboard Context on Server is a bit flag and ours just a normal enum. Therefor we need to calculate 2^Context to get the right parameter
            WebUtils::GetJSONAsync(WebUtils::API_URL + "player/" + currentPlayer->id + "/history?leaderboardContext="+contextToPlayerUrlString[static_cast<LeaderboardUI::Context>(getModConfig().Context.GetValue())]+"&count=1", [finished, handleError](long historyStatus, bool historyError, rapidjson::Document const& historyResult){
                // Only do stuff if we are successful
                if(historyStatus == 200 && !historyError){
                    // Set the new Historydata on the player
                    currentPlayer->SetHistory(historyResult.GetArray()[0]);

                    // Call callbacks
                    if (finished) finished(currentPlayer, "");
                    callbackWrapper(currentPlayer);
                }
                else {
                    handleError();
                }
            });
        }
        else{
            handleError();
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
