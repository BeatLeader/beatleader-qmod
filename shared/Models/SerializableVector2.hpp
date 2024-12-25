#pragma once
#include "UnityEngine/Vector2.hpp"
#include "UnityEngine/Vector3.hpp"
#include "beatsaber-hook/shared/config/rapidjson-utils.hpp"
#include <stdexcept>

struct SerializableVector2 {
    float x;
    float y;

    SerializableVector2();
    SerializableVector2(UnityEngine::Vector2 unityVector);
    SerializableVector2(float x, float y);
    SerializableVector2(const rapidjson::Value& val);

    float operator[](int idx) const;

    operator UnityEngine::Vector3() const;
    operator UnityEngine::Vector2() const;
}; 