#pragma once

#include "beatsaber-hook/shared/config/rapidjson-utils.hpp"

#include <functional>
#include <optional>
#include <string>
#include <vector>

class DanceEventMap {
public:
    std::string leaderboardId;
    std::string hash;
    float points = 0;
    bool eligible = false;
    bool dancing = false;
    bool counted = false;

    DanceEventMap(rapidjson::Value const& document);
};

class DanceEventPlayer {
public:
    float points = 0;
    int rank = 0;
    int countryRank = 0;
    int dancingMaps = 0;
    std::vector<DanceEventMap> maps;

    DanceEventPlayer(rapidjson::Value const& document);
};

class DanceEventStatus {
public:
    std::string eventName;
    int countedMaps = 5;
    int playersCount = 0;
    std::optional<DanceEventPlayer> player;
    std::optional<DanceEventMap> score;

    DanceEventStatus(rapidjson::Value const& document);

    static void Load(int eventId, std::optional<int> scoreId, std::function<void(std::optional<DanceEventStatus>)> const& completion);
};
