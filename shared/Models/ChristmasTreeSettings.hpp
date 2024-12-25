#pragma once
#include "shared/Models/SerializablePose.hpp"
#include "shared/Models/ChristmasTreeOrnamentSettings.hpp"
#include "beatsaber-hook/shared/config/rapidjson-utils.hpp"
#include <vector>

struct ChristmasTreeSettings {
    FullSerializablePose gameTreePose;
    std::vector<ChristmasTreeOrnamentSettings> ornaments;

    ChristmasTreeSettings() = default;
    ChristmasTreeSettings(const rapidjson::Value& val);
}; 