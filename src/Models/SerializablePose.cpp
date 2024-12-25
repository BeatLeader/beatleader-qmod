#include "shared/Models/SerializablePose.hpp"
#include "main.hpp"

// SerializablePose implementations
SerializablePose::SerializablePose(SerializableVector3 position, SerializableQuaternion rotation)
    : position(position), rotation(rotation) {}

SerializablePose::SerializablePose(const rapidjson::Value& val) {
    if (val.HasMember("position")) position = SerializableVector3(val["position"].GetObject());
    if (val.HasMember("rotation")) rotation = SerializableQuaternion(val["rotation"].GetObject());
}

SerializablePose::operator UnityEngine::Pose() const { 
    return UnityEngine::Pose(position, rotation); 
}

SerializablePose::SerializablePose(UnityEngine::Pose pose) : position(pose.position), rotation(pose.rotation) {}

// FullSerializablePose implementations
FullSerializablePose::FullSerializablePose(SerializableVector3 position, SerializableVector3 scale, SerializableQuaternion rotation)
    : position(position), scale(scale), rotation(rotation) {}

FullSerializablePose::FullSerializablePose(UnityEngine::Vector3 position, UnityEngine::Vector3 scale, UnityEngine::Quaternion rotation)
    : position(position), scale(scale), rotation(rotation) {}

FullSerializablePose::FullSerializablePose(const rapidjson::Value& val) {
    if (val.HasMember("position")) position = SerializableVector3(val["position"].GetObject());
    if (val.HasMember("scale")) scale = SerializableVector3(val["scale"].GetObject());
    if (val.HasMember("rotation")) rotation = SerializableQuaternion(val["rotation"].GetObject());
}

FullSerializablePose::operator UnityEngine::Pose() const { 
    return UnityEngine::Pose(position, rotation); 
} 

void writeDocumentToString(const rapidjson::Document& document,
                           std::string& output)
{
  class StringHolder
  {
  public:
    typedef char Ch;
    StringHolder(std::string& s) : s_(s) { s_.reserve(4096); }
    void Put(char c) { s_.push_back(c); }
    void Flush() { return; }

  private:
    std::string& s_;
  };

  StringHolder os(output);
  rapidjson::Writer<StringHolder> writer(os);
  document.Accept(writer);
}

std::string FullSerializablePose::ToJSON() const {
    rapidjson::Document doc;
    doc.SetObject();
    auto& allocator = doc.GetAllocator();
    rapidjson::Value positionObj(rapidjson::kObjectType);
    positionObj.AddMember("x", position.x, allocator);
    positionObj.AddMember("y", position.y, allocator);
    positionObj.AddMember("z", position.z, allocator);
    doc.AddMember("position", positionObj, allocator);

    rapidjson::Value scaleObj(rapidjson::kObjectType);
    scaleObj.AddMember("x", scale.x, allocator);
    scaleObj.AddMember("y", scale.y, allocator);
    scaleObj.AddMember("z", scale.z, allocator);
    doc.AddMember("scale", scaleObj, allocator);

    rapidjson::Value rotationObj(rapidjson::kObjectType);
    rotationObj.AddMember("x", rotation.x, allocator);
    rotationObj.AddMember("y", rotation.y, allocator);
    rotationObj.AddMember("z", rotation.z, allocator);
    rotationObj.AddMember("w", rotation.w, allocator);
    doc.AddMember("rotation", rotationObj, allocator);

    std::string output;
    writeDocumentToString(doc, output);

    BeatLeaderLogger.info("FullSerializablePose: {}", output);
    return output;
}
