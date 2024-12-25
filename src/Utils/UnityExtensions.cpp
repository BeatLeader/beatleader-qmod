#include "Utils/UnityExtensions.hpp"
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Quaternion.hpp"

namespace BeatLeader::UnityExtensions {

    bool TryDestroy(UnityEngine::Object* obj) {
        if (!obj) return false;
        UnityEngine::Object::Destroy(obj);
        return true;
    }

    void SetLocalPose(UnityEngine::Transform* transform, UnityEngine::Pose pose) {
        transform->set_localPosition(pose.position);
        transform->set_localRotation(pose.rotation);
    }

    void SetLocalPose(UnityEngine::Transform* transform, SerializablePose pose) {
        SetLocalPose(transform, static_cast<UnityEngine::Pose>(pose));
    }

    UnityEngine::Pose GetLocalPose(UnityEngine::Transform* transform) {
        return UnityEngine::Pose(transform->get_localPosition(), transform->get_localRotation());
    }

    SerializablePose Lerp(SerializablePose a, SerializablePose b, float f) {
        return SerializablePose(UnityEngine::Vector3::Lerp(a.position, b.position, f),
            UnityEngine::Quaternion::Lerp(a.rotation, b.rotation, f));
    }
} 