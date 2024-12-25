#include "shared/Models/SerializableQuaternion.hpp"

SerializableQuaternion::SerializableQuaternion() : x(0), y(0), z(0), w(1) {}

SerializableQuaternion::SerializableQuaternion(UnityEngine::Quaternion unityQuaternion)
    : x(unityQuaternion.x), y(unityQuaternion.y), z(unityQuaternion.z), w(unityQuaternion.w) {}

SerializableQuaternion::SerializableQuaternion(const rapidjson::Value& val) {
    if (val.HasMember("x")) x = val["x"].GetFloat();
    if (val.HasMember("y")) y = val["y"].GetFloat();
    if (val.HasMember("z")) z = val["z"].GetFloat();
    if (val.HasMember("w")) w = val["w"].GetFloat();
}

SerializableQuaternion::operator UnityEngine::Quaternion() const { 
    return UnityEngine::Quaternion(x, y, z, w); 
} 