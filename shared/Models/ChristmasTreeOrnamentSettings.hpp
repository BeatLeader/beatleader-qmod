#pragma once
#include "shared/Models/SerializablePose.hpp"
#include "UnityEngine/Pose.hpp"
#include "beatsaber-hook/shared/config/rapidjson-utils.hpp"

struct ChristmasTreeOrnamentSettings {
    int bundleId;
    int state;
    SerializablePose pose;

    ChristmasTreeOrnamentSettings() = default;
    ChristmasTreeOrnamentSettings(const rapidjson::Value& val);
    ChristmasTreeOrnamentSettings(int bundleId, UnityEngine::Pose pose);
}; 