#include "shared/Models/SerializableVector2.hpp"

SerializableVector2::SerializableVector2() : x(0), y(0) {}

SerializableVector2::SerializableVector2(UnityEngine::Vector2 unityVector) 
    : x(unityVector.x), y(unityVector.y) {}

SerializableVector2::SerializableVector2(float x, float y) : x(x), y(y) {}

SerializableVector2::SerializableVector2(const rapidjson::Value& val) {
    if (val.HasMember("x")) x = val["x"].GetFloat();
    if (val.HasMember("y")) y = val["y"].GetFloat();
}

float SerializableVector2::operator[](int idx) const {
    switch(idx) {
        case 0: return x;
        case 1: return y;
        default: throw std::out_of_range("Index out of range");
    }
}

SerializableVector2::operator UnityEngine::Vector3() const { 
    return UnityEngine::Vector3(x, y, 0); 
}

SerializableVector2::operator UnityEngine::Vector2() const { 
    return UnityEngine::Vector2(x, y); 
} 