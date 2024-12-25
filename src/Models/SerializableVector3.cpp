#include "shared/Models/SerializableVector3.hpp"

SerializableVector3::SerializableVector3() : x(0), y(0), z(0) {}

SerializableVector3::SerializableVector3(UnityEngine::Vector3 unityVector) 
    : x(unityVector.x), y(unityVector.y), z(unityVector.z) {}

SerializableVector3::SerializableVector3(float x, float y, float z) 
    : x(x), y(y), z(z) {}

SerializableVector3::SerializableVector3(const rapidjson::Value& val) {
    if (val.HasMember("x")) x = val["x"].GetFloat();
    if (val.HasMember("y")) y = val["y"].GetFloat();
    if (val.HasMember("z")) z = val["z"].GetFloat();
}

float SerializableVector3::operator[](int idx) const {
    switch(idx) {
        case 0: return x;
        case 1: return y;
        case 2: return z;
        default: throw std::out_of_range("Index out of range");
    }
}

SerializableVector3::operator UnityEngine::Vector3() const { 
    return UnityEngine::Vector3(x, y, z); 
}

SerializableVector3::operator UnityEngine::Vector2() const { 
    return UnityEngine::Vector2(x, y); 
} 