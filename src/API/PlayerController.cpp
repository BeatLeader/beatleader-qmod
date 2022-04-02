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

Player* PlayerController::currentPlayer = NULL;
Player* PlayerController::platformPlayer = NULL;
string PlayerController::lastErrorDescription = "";
vector<function<void(Player*)>> PlayerController::playerChanged;

void callbackWrapper(Player* player) {
    for (auto && fn : PlayerController::playerChanged)
        fn(player);
}

string PlayerController::RefreshOnline() {
    string result = "";
    WebUtils::Get(WebUtils::API_URL + "user/id", result);
    if (result.length() > 0) {
        currentPlayer = new Player();
        currentPlayer->id = result;

        WebUtils::GetJSONAsync(WebUtils::API_URL + "player/" + result, [](long status, bool error, rapidjson::Document& result){
            if (status == 200) {
                auto player = result.GetObject();
                currentPlayer = new Player(player);
                callbackWrapper(currentPlayer);
            }
        });
    }
    return result;
}

void PlayerController::RefreshPlatform() {
    IPlatformUserModel* userModel = NULL;
}

string PlayerController::Refresh() {
    if (platformPlayer == NULL) {
        RefreshPlatform();
    }
    
    return RefreshOnline();
}

void PlayerController::SignUp(string login, string password, std::function<void(std::string)> finished) {
    lastErrorDescription = "";

    WebUtils::PostFormAsync(WebUtils::API_URL + "signinoculus", "signup", login, password, [finished] (long statusCode, string error) {
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

void PlayerController::LogIn(string login, string password, std::function<void(std::string)> finished) {
    lastErrorDescription = "";

    WebUtils::PostFormAsync(WebUtils::API_URL + "signinoculus", "login", login, password, [finished] (long statusCode, string error) {
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
        currentPlayer = NULL;
        callbackWrapper(currentPlayer);
        return true;
    } else {
        return false;
    }
}
