#pragma once
#include "shared/Models/MapDetail.hpp"
#include "shared/Models/Score.hpp"
#include "beatsaber-hook/shared/config/rapidjson-utils.hpp"
#include <vector>
#include <string>
#include <utility>

struct DailyTreeStatus {
    MapDetail song;
    std::optional<Score> score;
    int bundleId;
    long startTime;

    DailyTreeStatus() = default;
    DailyTreeStatus(const rapidjson::Value& val);
};

struct BonusOrnament {
    std::optional<Score> score;
    int bundleId;
    std::string description;

    BonusOrnament() = default;
    BonusOrnament(const rapidjson::Value& val);
};

struct TreeStatus {
    std::optional<DailyTreeStatus> today;
    std::vector<DailyTreeStatus> previousDays;
    std::vector<BonusOrnament> bonusOrnaments;

    TreeStatus() = default;
    TreeStatus(const rapidjson::Value& val);

    std::vector<std::pair<int, std::string>> GetOrnamentIds();
}; 