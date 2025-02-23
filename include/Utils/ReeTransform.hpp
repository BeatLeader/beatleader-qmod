#pragma once
#include "Sombrero/shared/FastVector3.hpp"
#include "Sombrero/shared/FastQuaternion.hpp"

namespace BeatLeader {
    class ReeTransform {
        private:
            Sombrero::FastQuaternion _rotation;
            Sombrero::FastQuaternion _inverseRotation;

        public:
            Sombrero::FastVector3 Position;

            static Sombrero::FastQuaternion Inverse(Sombrero::FastQuaternion rotation) {
                float lengthSq = rotation.x * rotation.x + rotation.y * rotation.y + rotation.z * rotation.z + rotation.w * rotation.w;
                if (lengthSq != 0.0)
                {
                    float i = 1.0f / lengthSq;
                    return Sombrero::FastQuaternion(rotation.x * -i, rotation.y * -i, rotation.z * -i, rotation.w * i);
                }
                return rotation;
            }

            static Sombrero::FastQuaternion InverseTransformRotation(Sombrero::FastQuaternion rotation, Sombrero::FastQuaternion worldRotation) {
                return Inverse(rotation) * worldRotation;
            }

            const Sombrero::FastQuaternion& GetRotation() const { return _rotation; }
            void SetRotation(const Sombrero::FastQuaternion& value) {
                _rotation = value;
                _inverseRotation = ReeTransform::Inverse(value);
            }

            ReeTransform(
                const Sombrero::FastVector3& position,
                const Sombrero::FastQuaternion& rotation
            ) : Position(position) {
                SetRotation(rotation);
            }

            // Local to World transformations
            Sombrero::FastVector3 LocalToWorldPosition(const Sombrero::FastVector3& localPosition) const {
                return Position + (_rotation * localPosition);
            }

            Sombrero::FastVector3 LocalToWorldDirection(const Sombrero::FastVector3& localDirection) const {
                return _rotation * localDirection;
            }

            Sombrero::FastQuaternion LocalToWorldRotation(const Sombrero::FastQuaternion& localRotation) const {
                return _rotation * localRotation;
            }

            // World to Local transformations  
            Sombrero::FastVector3 WorldToLocalPosition(const Sombrero::FastVector3& worldPosition) const {
                return _inverseRotation * (worldPosition - Position);
            }

            Sombrero::FastVector3 WorldToLocalDirection(const Sombrero::FastVector3& worldDirection) const {
                return _inverseRotation * worldDirection;
            }

            Sombrero::FastQuaternion WorldToLocalRotation(const Sombrero::FastQuaternion& worldRotation) const {
                return _inverseRotation * worldRotation;
            }
    };
}
