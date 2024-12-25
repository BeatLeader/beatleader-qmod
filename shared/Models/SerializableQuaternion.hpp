#pragma once
#include "UnityEngine/Quaternion.hpp"
#include "beatsaber-hook/shared/config/rapidjson-utils.hpp"

struct SerializableQuaternion {
    float x;
    float y;
    float z;
    float w;

    SerializableQuaternion();
    SerializableQuaternion(UnityEngine::Quaternion unityQuaternion);
    SerializableQuaternion(const rapidjson::Value& val);

    operator UnityEngine::Quaternion() const;
}; 