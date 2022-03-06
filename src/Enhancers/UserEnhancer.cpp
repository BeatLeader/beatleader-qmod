#include "include/Enhancers/UserEnhancer.hpp"
#include "include/API/PlayerController.hpp"

#include "GlobalNamespace/IPlatformUserModel.hpp"
#include "GlobalNamespace/UserInfo.hpp"
#include "GlobalNamespace/PlatformLeaderboardsModel.hpp"

#include "System/Action_1.hpp"
#include "System/Threading/Tasks/Task.hpp"
#include "System/Threading/Tasks/Task_1.hpp"

#include "UnityEngine/Resources.hpp"

#include "beatsaber-hook/shared/utils/hooking.hpp"

using namespace GlobalNamespace;
using UnityEngine::Resources;

UserEnhancer::UserEnhancer(/* args */)
{
}

UserEnhancer::~UserEnhancer()
{
}

void UserEnhancer::EnhanceLocal(Replay* replay) {
    IPlatformUserModel* userModel = NULL;
    ::Array<PlatformLeaderboardsModel *>* pmarray = Resources::FindObjectsOfTypeAll<PlatformLeaderboardsModel*>();
    for (size_t i = 0; i < pmarray->Length(); i++)
    {
        if (pmarray->get(i)->platformUserModel != NULL) {
            userModel = pmarray->get(i)->platformUserModel;
            break;
        }
    }

    if (userModel == NULL) { return; }

    auto userInfoTask = userModel->GetUserInfo();

    auto action = il2cpp_utils::MakeDelegate<System::Action_1<System::Threading::Tasks::Task*>*>(classof(System::Action_1<System::Threading::Tasks::Task*>*), (std::function<void(System::Threading::Tasks::Task_1<GlobalNamespace::UserInfo*>*)>)[&](System::Threading::Tasks::Task_1<GlobalNamespace::UserInfo*>* userInfoTask) {
            UserInfo *ui = userInfoTask->get_Result();
            if (ui != nullptr) {
                replay->info->playerName = to_utf8(csstrtostr(ui->userName));
                replay->info->playerID = to_utf8(csstrtostr(ui->platformUserId));
                replay->info->platform = "oculus";
            }
        }
    );
    reinterpret_cast<System::Threading::Tasks::Task*>(userInfoTask)->ContinueWith(action);
}

void UserEnhancer::EnhanceBL(Replay* replay) {
    replay->info->playerName = PlayerController::currentPlayer->name;
    replay->info->playerID = PlayerController::currentPlayer->id;
    replay->info->platform = "oculus";
}

void UserEnhancer::Enhance(Replay* replay)
{
    if (PlayerController::currentPlayer != NULL && PlayerController::currentPlayer->name.length() > 0) {
        EnhanceBL(replay);
    } else {
        EnhanceLocal(replay);
    }

    // ¯\_(ツ)_/¯
    replay->info->hmd = "Oculus Quest";
    replay->info->trackingSytem = "Oculus";
    replay->info->controller = "Oculus Touch";
}