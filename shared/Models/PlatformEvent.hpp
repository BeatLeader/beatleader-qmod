#pragma once
#include <string>
#include <optional>
#include "beatsaber-hook/shared/config/rapidjson-utils.hpp"

class PlatformEvent {
public:
    std::string image;
    std::string name;
    int64_t endDate;
    bool downloadable;
    int playerCount;
    int playlistId;
    std::string description;

    PlatformEvent(rapidjson::Value const& document);
}; 