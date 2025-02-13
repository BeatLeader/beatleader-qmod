#include "include/Enhancers/UserEnhancer.hpp"
#include "include/API/PlayerController.hpp"

#include "GlobalNamespace/OVRPlugin.hpp"

#include <sys/system_properties.h>

std::string UserEnhancer::hmd = "Unknown";
void UserEnhancer::FetchHMD() {
    auto prop = __system_property_find("ro.product.model");
    if (prop) {
        __system_property_read_callback(prop, [](void* cookie, const char* name, const char* value, uint32_t serial) {
            std::string brand(value);
            UserEnhancer::hmd = brand;
        }, nullptr);
    }
}

void UserEnhancer::Enhance(Replay& replay)
{
    // pointer because easier than optional with reference
    Player* player;
    if (PlayerController::currentPlayer && !PlayerController::currentPlayer->name.empty()) {
        player = &PlayerController::currentPlayer.value();
    }

    if (player != NULL) {
        replay.info.playerName = player->name;
        replay.info.playerID = player->id;
    } else {
        replay.info.playerName = "Anonimus";
        replay.info.playerID = "-1";
    }

    replay.info.platform = "oculus";
    
    if (UserEnhancer::hmd != "Unknown") {
        replay.info.hmd = UserEnhancer::hmd;
    } else {
        switch (GlobalNamespace::OVRPlugin::GetSystemHeadsetType())
        {
        case GlobalNamespace::OVRPlugin::SystemHeadset::Oculus_Quest:
            replay.info.hmd = "Oculus Quest";
            break;
        case GlobalNamespace::OVRPlugin::SystemHeadset::Oculus_Quest_2:
            replay.info.hmd = "Oculus Quest 2";
            break;
        case GlobalNamespace::OVRPlugin::SystemHeadset::Meta_Quest_Pro:
            replay.info.hmd = "Meta Quest Pro";
            break;  
        case GlobalNamespace::OVRPlugin::SystemHeadset::Meta_Quest_3:
            replay.info.hmd = "Meta Quest 3";
            break;
        default:
            replay.info.hmd = "Unknown";
            break;
        }
    }
    switch (GlobalNamespace::OVRPlugin::GetActiveController())
    {
    case GlobalNamespace::OVRPlugin::Controller::Touch:
        replay.info.controller = "Oculus Touch";
        break;
    case GlobalNamespace::OVRPlugin::Controller::Hands:
        replay.info.controller = "Hands";
        break;
    default:
        replay.info.controller = "Unknown";
        break;
    }
    replay.info.trackingSystem = "Oculus";
}