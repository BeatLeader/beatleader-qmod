#include "include/Enhancers/UserEnhancer.hpp"
#include "include/API/PlayerController.hpp"

UserEnhancer::UserEnhancer(/* args */)
{
}

UserEnhancer::~UserEnhancer()
{
}

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

    // ¯\_(ツ)_/¯
    replay->info->hmd = "Oculus Quest";
    replay->info->trackingSytem = "Oculus";
    replay->info->controller = "Oculus Touch";
}