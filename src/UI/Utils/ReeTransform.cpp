#include "UI/Utils/ReeTransform.hpp"

namespace BeatLeader {

ReeTransform::ReeTransform(const UnityEngine::Vector3& position, const UnityEngine::Quaternion& rotation)
    : Position(position), Rotation(rotation) {}

ReeTransform ReeTransform::Identity() {
    return ReeTransform(UnityEngine::Vector3::get_zero(), UnityEngine::Quaternion::get_identity());
}

ReeTransform ReeTransform::PositiveInfinity() {
    return ReeTransform(UnityEngine::Vector3(500, 500, 500), UnityEngine::Quaternion::get_identity());
}

ReeTransform ReeTransform::FromTransform(UnityEngine::Transform* transform) {
    return ReeTransform(transform->get_position(), transform->get_rotation());
}

UnityEngine::Quaternion ReeTransform::GetInverseRotation() const {
    return UnityEngine::Quaternion::Inverse(Rotation);
}

UnityEngine::Vector3 ReeTransform::GetForward() const {
    return LocalToWorldDirection(UnityEngine::Vector3::get_forward());
}

UnityEngine::Vector3 ReeTransform::GetRight() const {
    return LocalToWorldDirection(UnityEngine::Vector3::get_right());
}

ReeTransform ReeTransform::GetParentTransform(const ReeTransform& childWorldTransform, const ReeTransform& childLocalTransform) {
    auto rotation = UnityEngine::Quaternion::op_Multiply(childWorldTransform.Rotation, childLocalTransform.GetInverseRotation());
    return ReeTransform(
        UnityEngine::Vector3::op_Subtraction(
            childWorldTransform.Position, 
            UnityEngine::Quaternion::op_Multiply(rotation, childLocalTransform.Position)
        ),
        rotation
    );
}

ReeTransform ReeTransform::GetChildTransform(const ReeTransform& parentWorldTransform, const ReeTransform& childLocalTransform) {
    return ReeTransform(
        parentWorldTransform.LocalToWorldPosition(childLocalTransform.Position),
        parentWorldTransform.LocalToWorldRotation(childLocalTransform.Rotation)
    );
}

UnityEngine::Vector3 ReeTransform::LocalToWorldPosition(const UnityEngine::Vector3& localPosition) const {
    return UnityEngine::Vector3::op_Addition(
        Position, 
        UnityEngine::Quaternion::op_Multiply(Rotation, localPosition)
    );
}

UnityEngine::Vector3 ReeTransform::LocalToWorldDirection(const UnityEngine::Vector3& localDirection) const {
    return UnityEngine::Quaternion::op_Multiply(Rotation, localDirection);
}

UnityEngine::Quaternion ReeTransform::LocalToWorldRotation(const UnityEngine::Quaternion& localRotation) const {
    return UnityEngine::Quaternion::op_Multiply(Rotation, localRotation);
}

UnityEngine::Vector3 ReeTransform::WorldToLocalPosition(const UnityEngine::Vector3& worldPosition) const {
    return UnityEngine::Quaternion::op_Multiply(
        GetInverseRotation(), 
        UnityEngine::Vector3::op_Subtraction(worldPosition, Position)
    );
}

UnityEngine::Vector3 ReeTransform::WorldToLocalDirection(const UnityEngine::Vector3& worldDirection) const {
    return UnityEngine::Quaternion::op_Multiply(GetInverseRotation(), worldDirection);
}

UnityEngine::Quaternion ReeTransform::WorldToLocalRotation(const UnityEngine::Quaternion& worldRotation) const {
    return UnityEngine::Quaternion::op_Multiply(GetInverseRotation(), worldRotation);
}

bool ReeTransform::operator==(const ReeTransform& other) const {
    return UnityEngine::Quaternion::op_Equality(Rotation, other.Rotation) && 
           UnityEngine::Vector3::op_Equality(Position, other.Position);
}

bool ReeTransform::operator!=(const ReeTransform& other) const {
    return !(*this == other);
}

} // namespace BeatLeader 