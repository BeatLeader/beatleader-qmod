#pragma once
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Vector2.hpp"
#include "beatsaber-hook/shared/config/rapidjson-utils.hpp"
#include <stdexcept>

struct SerializableVector3 {
    float x;
    float y;
    float z;

    SerializableVector3();
    SerializableVector3(UnityEngine::Vector3 unityVector);
    SerializableVector3(float x, float y, float z);
    SerializableVector3(const rapidjson::Value& val);

    float operator[](int idx) const;

    operator UnityEngine::Vector3() const;
    operator UnityEngine::Vector2() const;
}; 