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
optional<Player> PlayerController::platformPlayer = nullopt;
string PlayerController::lastErrorDescription = "";
vector<function<void(optional<Player> const&)>> PlayerController::playerChanged;

void callbackWrapper(optional<Player> const& player) {
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
        currentPlayer = nullopt;
    }
    return result;
}

void PlayerController::RefreshPlatform() {
    IPlatformUserModel* userModel = NULL;
    ::ArrayW<PlatformLeaderboardsModel *> pmarray = Resources::FindObjectsOfTypeAll<PlatformLeaderboardsModel*>();
    for (size_t i = 0; i < pmarray.Length(); i++)
    {
        if (pmarray.get(i)->platformUserModel != NULL) {
            userModel = pmarray.get(i)->platformUserModel;
            break;
        }
    }

    if (userModel == NULL) { return; }

    auto userInfoTask = userModel->GetUserInfo();

    auto action = il2cpp_utils::MakeDelegate<System::Action_1<System::Threading::Tasks::Task*>*>(classof(System::Action_1<System::Threading::Tasks::Task*>*), (std::function<void(System::Threading::Tasks::Task_1<GlobalNamespace::UserInfo*>*)>)[&](System::Threading::Tasks::Task_1<GlobalNamespace::UserInfo*>* userInfoTask) {
            UserInfo *ui = userInfoTask->get_Result();
            if (ui != nullptr) {
                platformPlayer = new Player();
                platformPlayer->name = (string)ui->userName;
                platformPlayer->id = (string)ui->platformUserId;
            }
        }
    );
    reinterpret_cast<System::Threading::Tasks::Task*>(userInfoTask)->ContinueWith(action);
}

string PlayerController::Refresh() {
    if (platformPlayer == nullopt) {
        RefreshPlatform();
    }
    
    return RefreshOnline();
}

void PlayerController::SignUp(string login, string password, const function<void(string)>& finished) {
    lastErrorDescription = "";

    WebUtils::PostFormAsync(WebUtils::API_URL + "signinoculus", password, login, "signup",
                            [finished](long statusCode, string error) {
                                string result = "";
                                if (statusCode == 200) {
                                    result = Refresh();
                                } else {
                                    lastErrorDescription = error;
                                    getLogger().error("BeatLeader %s",
                                                      ("signup error" + to_string(statusCode)).c_str());
                                }
                                finished(result);
                            });
}

void PlayerController::LogIn(string login, string password, const function<void(string)>& finished) {
    lastErrorDescription = "";

    WebUtils::PostFormAsync(WebUtils::API_URL + "signinoculus", password, login, "login",
                            [finished](long statusCode, string error) {
                                string result = "";
                                if (statusCode == 200) {
                                    result = Refresh();
                                } else {
                                    lastErrorDescription = error;
                                    getLogger().error("BeatLeader %s",
                                                      ("signup error" + to_string(statusCode)).c_str());
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
        currentPlayer = nullopt;
        callbackWrapper(currentPlayer);
        return true;
    } else {
        return false;
    }
}