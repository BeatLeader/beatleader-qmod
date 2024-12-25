#include "shared/Models/TreeStatus.hpp"

DailyTreeStatus::DailyTreeStatus(const rapidjson::Value& val) {
    if (val.HasMember("song")) song = MapDetail(val["song"].GetObject());
    if (val.HasMember("score") && !val["score"].IsNull()) {
        score = Score(val["score"].GetObject());
    }
    if (val.HasMember("bundleId")) bundleId = val["bundleId"].GetInt();
    if (val.HasMember("startTime")) startTime = val["startTime"].GetInt64();
}

BonusOrnament::BonusOrnament(const rapidjson::Value& val) {
    if (val.HasMember("score") && !val["score"].IsNull()) {
        score = Score(val["score"].GetObject());
    }
    if (val.HasMember("bundleId")) bundleId = val["bundleId"].GetInt();
    if (val.HasMember("description")) description = val["description"].GetString();
}

TreeStatus::TreeStatus(const rapidjson::Value& val) {
    if (val.HasMember("today") && !val["today"].IsNull()) {
        today = DailyTreeStatus(val["today"].GetObject());
    }

    if (val.HasMember("previousDays") && val["previousDays"].IsArray()) {
        auto arr = val["previousDays"].GetArray();
        previousDays.reserve(arr.Size());
        for (const auto& day : arr) {
            previousDays.emplace_back(day.GetObject());
        }
    }

    if (val.HasMember("bonusOrnaments") && val["bonusOrnaments"].IsArray()) {
        auto arr = val["bonusOrnaments"].GetArray();
        bonusOrnaments.reserve(arr.Size());
        for (const auto& bonus : arr) {
            bonusOrnaments.emplace_back(bonus.GetObject());
        }
    }
}

std::vector<std::pair<int, std::string>> TreeStatus::GetOrnamentIds() {
    std::vector<std::pair<int, std::string>> result;
    
    if (today && today->score) {
        result.emplace_back(today->bundleId, "");
    }

    for (const auto& prevDay : previousDays) {
        if (prevDay.score) {
            result.emplace_back(prevDay.bundleId, "");
        }
    }

    for (const auto& bonus : bonusOrnaments) {
        result.emplace_back(bonus.bundleId, bonus.description);
    }

    return result;
} 