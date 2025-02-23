#pragma once

#include "shared/Models/Player.hpp"
#include "conditional-dependencies/shared/main.hpp"

namespace BeatLeader {
    inline std::optional<Player> LoggedInPlayer() {
        static auto func = CondDeps::Find<std::optional<Player>>("bl", "LoggedInPlayer");
        if(func)
            return func.value()();
        return std::nullopt;
    }

    inline void AddPlayerCallback(function<void(std::optional<Player>)> callback) {
        static auto func = CondDeps::Find<void, function<void(std::optional<Player>)>>("bl", "AddPlayerCallback");
        if(func)
            func.value()(callback);
    }

    inline std::optional<string> LoggedInPlayerId() {
        static auto func = CondDeps::Find<std::optional<string>>("bl", "LoggedInPlayerId");
        if(func)
            return func.value()();
        return std::nullopt;
    }

    inline std::optional<string> LoggedInPlayerQuestId() {
        static auto func = CondDeps::Find<std::optional<string>>("bl", "LoggedInPlayerQuestId");
        if(func)
            return func.value()();
        return std::nullopt;
    }

    inline void AddReplayCustomDataProvider(std::string name, function<void(std::string, int*, void**)> callback) {
        static auto func = CondDeps::Find<void, std::string, function<void(std::string, int*, void**)>>("bl", "AddReplayCustomDataProvider");
        if(func)
            func.value()(name, callback);
    }
}