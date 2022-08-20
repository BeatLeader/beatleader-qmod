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

optional<Player> PlayerController::currentPlayer = nullopt;
string PlayerController::lastErrorDescription = "";
vector<function<void(optional<Player> const&)>> PlayerController::playerChanged;

void callbackWrapper(optional<Player> const& player) {
    for (auto && fn : PlayerController::playerChanged)
        fn(player);
}

void PlayerController::Refresh(const function<void(optional<Player> const&, string)>& finished) {
    WebUtils::GetJSONAsync(WebUtils::API_URL + "user/modinterface", [finished](long status, bool error, rapidjson::Document const& result){
        if (status == 200 && !error) {
            currentPlayer = Player(result.GetObject());
            if (finished) finished(currentPlayer, "");
            callbackWrapper(currentPlayer);
        } else {
            currentPlayer = nullopt;
            if (finished) finished(nullopt, "Failed to retrieve player");
        }
    });
}

void PlayerController::SignUp(string login, string password, const function<void(optional<Player> const&, string)>& finished) {
    lastErrorDescription = "";

    WebUtils::PostFormAsync(WebUtils::API_URL + "signinoculus", password, login, "signup",
                            [finished](long statusCode, string error) {
        if (statusCode == 200) {
            Refresh(finished);
        } else {
            lastErrorDescription = error;
            getLogger().error("BeatLeader %s",
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
            Refresh(finished);
        } else {
            lastErrorDescription = error;
            getLogger().error("BeatLeader %s",
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
