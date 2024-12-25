#include "shared/Models/ChristmasTreeOrnamentSettings.hpp"

ChristmasTreeOrnamentSettings::ChristmasTreeOrnamentSettings(const rapidjson::Value& val) {
    if (val.HasMember("bundleId")) bundleId = val["bundleId"].GetInt();
    if (val.HasMember("state")) state = val["state"].GetInt();
    if (val.HasMember("pose")) {
        pose = SerializablePose(val["pose"].GetObject());
    }
}

ChristmasTreeOrnamentSettings::ChristmasTreeOrnamentSettings(int bundleId, UnityEngine::Pose pose) : bundleId(bundleId), pose(pose) {}