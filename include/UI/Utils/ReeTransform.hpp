#pragma once

#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Quaternion.hpp"
#include "UnityEngine/Transform.hpp"

namespace BeatLeader {

struct ReeTransform {
    UnityEngine::Quaternion Rotation;
    UnityEngine::Vector3 Position;

    // Constructors
    ReeTransform() = default;
    ReeTransform(const UnityEngine::Vector3& position, const UnityEngine::Quaternion& rotation);
    
    static ReeTransform Identity();
    static ReeTransform PositiveInfinity();
    static ReeTransform FromTransform(UnityEngine::Transform* transform);

    // Properties
    UnityEngine::Quaternion GetInverseRotation() const;
    UnityEngine::Vector3 GetForward() const;
    UnityEngine::Vector3 GetRight() const;

    // Child/Parent operations
    static ReeTransform GetParentTransform(const ReeTransform& childWorldTransform, const ReeTransform& childLocalTransform);
    static ReeTransform GetChildTransform(const ReeTransform& parentWorldTransform, const ReeTransform& childLocalTransform);

    // LocalToWorld transformations
    UnityEngine::Vector3 LocalToWorldPosition(const UnityEngine::Vector3& localPosition) const;
    UnityEngine::Vector3 LocalToWorldDirection(const UnityEngine::Vector3& localDirection) const;
    UnityEngine::Quaternion LocalToWorldRotation(const UnityEngine::Quaternion& localRotation) const;

    // WorldToLocal transformations
    UnityEngine::Vector3 WorldToLocalPosition(const UnityEngine::Vector3& worldPosition) const;
    UnityEngine::Vector3 WorldToLocalDirection(const UnityEngine::Vector3& worldDirection) const;
    UnityEngine::Quaternion WorldToLocalRotation(const UnityEngine::Quaternion& worldRotation) const;

    // Equality operators
    bool operator==(const ReeTransform& other) const;
    bool operator!=(const ReeTransform& other) const;
};

} // namespace BeatLeader 