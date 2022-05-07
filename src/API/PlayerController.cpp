#include "include/API/PlayerController.hpp"
#include "include/Utils/WebUtils.hpp"
#include "include/main.hpp"

#include "GlobalNamespace/IPlatformUserModel.hpp"
#include "GlobalNamespace/UserInfo.hpp"
#include "GlobalNamespace/PlatformLeaderboardsModel.hpp"

#include "System/Action_1.hpp"
#include "System/Threading/Tasks/Task.hpp"
#include "System/Threading/Tasks/Task_1.hpp"

#include "UnityEngine/Resources.hpp"

#include "beatsaber-hook/shared/utils/hooking.hpp"

using UnityEngine::Resources;
using namespace GlobalNamespace;

std::optional<Player> PlayerController::currentPlayer = std::nullopt;
std::optional<Player> PlayerController::platformPlayer = std::nullopt;
string PlayerController::lastErrorDescription = "";
vector<function<void(std::optional<Player> const&)>> PlayerController::playerChanged;

void callbackWrapper(std::optional<Player> const& player) {
    for (auto && fn : PlayerController::playerChanged)
        fn(player);
}

string PlayerController::RefreshOnline() {
    string result = "";
    WebUtils::Get(WebUtils::API_URL + "user/id", result);
    if (result.length() > 0) {
        currentPlayer = Player();
        currentPlayer->id = result;

        WebUtils::GetJSONAsync(WebUtils::API_URL + "player/" + result, [](long status, bool error, rapidjson::Document const& result){
            if (status == 200) {
                currentPlayer = Player(result);
                callbackWrapper(currentPlayer);
            }
        });
    } else {
        currentPlayer = std::nullopt;
    }
    return result;
}

void PlayerController::RefreshPlatform() {
    IPlatformUserModel* userModel = NULL;
}

string PlayerController::Refresh() {
    if (platformPlayer == std::nullopt) {
        RefreshPlatform();
    }
    
    return RefreshOnline();
}

void PlayerController::SignUp(string_view login, string_view password, const std::function<void(std::string_view)>& finished) {
    lastErrorDescription = "";

    WebUtils::PostFormAsync(WebUtils::API_URL + "signinoculus", "signup", login, password, [finished] (long statusCode, string_view error) {
        string result = "";
        if (statusCode == 200) {
            result = Refresh();
        } else {
            lastErrorDescription = error;
            getLogger().error("BeatLeader %s", ("signup error" + to_string(statusCode)).c_str());
        }
        finished(result);
    });
}

void PlayerController::LogIn(string_view login, string_view password, const std::function<void(std::string_view)>& finished) {
    lastErrorDescription = "";

    WebUtils::PostFormAsync(WebUtils::API_URL + "signinoculus", "login", login, password, [finished] (long statusCode, string_view error) {
        string result = "";
        if (statusCode == 200) {
            result = Refresh();
        } else {
            lastErrorDescription = error;
            getLogger().error("BeatLeader %s", ("signup error" + to_string(statusCode)).c_str());
        }

        finished(result);
    });
}

bool PlayerController::LogOut() {
    string result = "";
    WebUtils::Get(WebUtils::API_URL + "signout", result);
    lastErrorDescription = result;
    WebUtils::Get(WebUtils::API_URL + "user/id", result);
    if (result.length() == 0) {
        currentPlayer = std::nullopt;
        callbackWrapper(currentPlayer);
        return true;
    } else {
        return false;
    }
}
