#include "shared/Models/DanceEventStatus.hpp"

#include "include/Utils/WebUtils.hpp"
#include "include/main.hpp"

#include "bsml/shared/BSML/MainThreadScheduler.hpp"

static rapidjson::Value const* Member(rapidjson::Value const& document, const char* name) {
    if (!document.IsObject()) return nullptr;
    auto it = document.FindMember(name);
    if (it == document.MemberEnd() || it->value.IsNull()) return nullptr;
    return &it->value;
}

static int GetInt(rapidjson::Value const& document, const char* name) {
    auto value = Member(document, name);
    return value && value->IsNumber() ? (int)value->GetDouble() : 0;
}

static float GetFloat(rapidjson::Value const& document, const char* name) {
    auto value = Member(document, name);
    return value && value->IsNumber() ? (float)value->GetDouble() : 0;
}

static bool GetBool(rapidjson::Value const& document, const char* name) {
    auto value = Member(document, name);
    return value && value->IsBool() && value->GetBool();
}

static std::string GetString(rapidjson::Value const& document, const char* name) {
    auto value = Member(document, name);
    return value && value->IsString() ? value->GetString() : "";
}

DanceEventMap::DanceEventMap(rapidjson::Value const& document) {
    leaderboardId = GetString(document, "leaderboardId");
    hash = GetString(document, "hash");
    points = GetFloat(document, "points");
    eligible = GetBool(document, "eligible");
    dancing = GetBool(document, "dancing");
    counted = GetBool(document, "counted");
}

DanceEventPlayer::DanceEventPlayer(rapidjson::Value const& document) {
    points = GetFloat(document, "points");
    rank = GetInt(document, "rank");
    countryRank = GetInt(document, "countryRank");
    dancingMaps = GetInt(document, "dancingMaps");
    if (auto list = Member(document, "maps"); list && list->IsArray()) {
        for (auto const& map : list->GetArray()) {
            if (map.IsObject()) maps.emplace_back(map);
        }
    }
}

DanceEventStatus::DanceEventStatus(rapidjson::Value const& document) {
    if (auto description = Member(document, "eventDescription")) eventName = GetString(*description, "name");
    if (auto rules = Member(document, "rules")) {
        int counted = GetInt(*rules, "countedMaps");
        if (counted > 0) countedMaps = counted;
    }
    playersCount = GetInt(document, "playersCount");
    if (auto value = Member(document, "player"); value && value->IsObject()) player = DanceEventPlayer(*value);
    if (auto value = Member(document, "score"); value && value->IsObject()) score = DanceEventMap(*value);
}

void DanceEventStatus::Load(int eventId, std::optional<int> scoreId, std::function<void(std::optional<DanceEventStatus>)> const& completion) {
    auto url = WebUtils::API_URL + "event/" + std::to_string(eventId) + "/dance/status";
    if (scoreId) url += "?scoreId=" + std::to_string(*scoreId);
    WebUtils::GetJSONAsync(url, [completion, url](long status, bool error, rapidjson::Document const& response) {
        std::optional<DanceEventStatus> result;
        if (status == 200 && !error && response.IsObject()) {
            result = DanceEventStatus(response);
        } else {
            BeatLeaderLogger.warn("Dance event status failed: {} {}", url, status);
        }
        BSML::MainThreadScheduler::Schedule([completion, result = std::move(result)] {
            completion(result);
        });
    });
}
