#pragma once
#include "SerializableVector3.hpp"
#include "SerializableQuaternion.hpp"
#include "UnityEngine/Pose.hpp"
#include "beatsaber-hook/shared/config/rapidjson-utils.hpp"

struct SerializablePose {
    SerializableVector3 position;
    SerializableQuaternion rotation;

    SerializablePose() = default;
    SerializablePose(SerializableVector3 position, SerializableQuaternion rotation);
    SerializablePose(UnityEngine::Pose pose);
    SerializablePose(const rapidjson::Value& val);

    operator UnityEngine::Pose() const;
};

struct FullSerializablePose {
    SerializableVector3 position;
    SerializableVector3 scale;
    SerializableQuaternion rotation;

    FullSerializablePose() = default;
    FullSerializablePose(SerializableVector3 position, SerializableVector3 scale, SerializableQuaternion rotation);
    FullSerializablePose(UnityEngine::Vector3 position, UnityEngine::Vector3 scale, UnityEngine::Quaternion rotation);
    FullSerializablePose(const rapidjson::Value& val);

    std::string ToJSON() const;

    operator UnityEngine::Pose() const;
}; 