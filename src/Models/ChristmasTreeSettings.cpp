#include "shared/Models/ChristmasTreeSettings.hpp"

ChristmasTreeSettings::ChristmasTreeSettings(const rapidjson::Value& val) {
    if (val.HasMember("gameTreePose")) {
        gameTreePose = FullSerializablePose(val["gameTreePose"].GetObject());
    }
    
    if (val.HasMember("ornaments") && val["ornaments"].IsArray()) {
        auto arr = val["ornaments"].GetArray();
        ornaments.reserve(arr.Size());
        for (const auto& ornament : arr) {
            ornaments.emplace_back(ornament.GetObject());
        }
    }
} 