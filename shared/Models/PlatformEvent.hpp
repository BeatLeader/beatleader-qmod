#pragma once
#include <string>
#include <optional>
#include "beatsaber-hook/shared/rapidjson.hpp"

class PlatformEvent {
public:
    int id = 0;
    std::string image;
    std::string name;
    int64_t endDate;
    bool downloadable;
    int playerCount;
    int playlistId;
    int eventType = 0;
    std::string description;

    PlatformEvent(rapidjson::Value const& document);

    bool IsHappening() const;
};
