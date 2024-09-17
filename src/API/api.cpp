#include "main.hpp"

#include "include/API/PlayerController.hpp"
#include "bsml/shared/BSML/MainThreadScheduler.hpp"

#include "conditional-dependencies/shared/main.hpp"

using namespace std;

// warning spam
#pragma GCC diagnostic ignored "-Wreturn-type"
#pragma GCC diagnostic push

EXPOSE_API(LoggedInPlayer, optional<Player>) {
    return PlayerController::currentPlayer;
}

EXPOSE_API(AddPlayerCallback, void, function<void(std::optional<Player>)> callback) {
    PlayerController::playerChanged.emplace_back([callback](std::optional<Player> const& updated) {
        BSML::MainThreadScheduler::Schedule([callback, updated] {
            if (callback) {
                callback(updated);
            }
        });
    });
}

EXPOSE_API(LoggedInPlayerId, optional<string>) {
    auto const& player = PlayerController::currentPlayer;
    if (player) {
        return player->id;
    } else {
        return nullopt;
    }
}

EXPOSE_API(LoggedInPlayerQuestId, optional<string>) {
    auto const& player = PlayerController::currentPlayer;
    return player ? player->questId : nullopt;
}