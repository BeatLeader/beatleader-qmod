#include "include/Enhancers/UserEnhancer.hpp"
#include "include/API/PlayerController.hpp"

#include "GlobalNamespace/OVRPlugin.hpp"
#include "GlobalNamespace/OVRPlugin_Controller.hpp"
#include "GlobalNamespace/OVRPlugin_SystemHeadset.hpp"

void UserEnhancer::Enhance(Replay* replay)
{
    Player* player;
    if (PlayerController::currentPlayer != NULL && PlayerController::currentPlayer->name.length() > 0) {
        player = PlayerController::currentPlayer;
    } else {
        player = PlayerController::platformPlayer;
    }

    if (player != NULL) {
        replay->info->playerName = player->name;
        replay->info->playerID = player->id;
        replay->info->platform = "oculus";
    }
    
    switch (GlobalNamespace::OVRPlugin::GetSystemHeadsetType())
    {
    case GlobalNamespace::OVRPlugin::SystemHeadset::Oculus_Quest:
        replay->info->hmd = "Oculus Quest";
        break;
    case GlobalNamespace::OVRPlugin::SystemHeadset::Oculus_Quest_2:
        replay->info->hmd = "Oculus Quest 2";
        break;
    default:
        replay->info->hmd = "Unknown";
        break;
    }
    switch (GlobalNamespace::OVRPlugin::GetActiveController())
    {
    case GlobalNamespace::OVRPlugin::Controller::Touch:
        replay->info->controller = "Oculus Touch";
        break;
    case GlobalNamespace::OVRPlugin::Controller::Hands:
        replay->info->controller = "Hands";
        break;
    default:
        replay->info->controller = "Unknown";
        break;
    }
    replay->info->trackingSytem = "Oculus";
}