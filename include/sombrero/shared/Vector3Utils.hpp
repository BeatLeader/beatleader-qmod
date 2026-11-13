#pragma once

#include "MiscUtils.hpp"
#include "Concepts.hpp"

#include <utility>

#include "beatsaber-hook/shared/types.hpp"
#ifdef HAS_CODEGEN
#include "UnityEngine/Vector3.hpp"
#endif

#define CONSTEXPR_GETTER(name, ...) \
constexpr static inline FastVector3 name() {\
    return __VA_ARGS__;\
}

#ifndef HAS_CODEGEN
// TODO: Will this break things?
namespace UnityEngine
{
    struct Vector3
    {
        float x, y, z;

        constexpr Vector3(float x = 0.0f, float y = 0.0f, float z = 0.0f) : x(x), y(y), z(z) {}
    };
}
#endif

namespace Sombrero {

    struct FastVector3;

    inline std::string vector3Str(UnityEngine::Vector3 const &vector3)
    {
        return std::to_string(vector3.x) + ", " + std::to_string(vector3.y) + ", " + std::to_string(vector3.z);
    }

    struct FastVector3 : public UnityEngine::Vector3 {
    public:
        // Implicit convert of vector
        constexpr FastVector3(const Vector3& vector) : Vector3(vector.x, vector.y, vector.z) {}
#ifdef USE_SOMBRERO_IMPLICIT_CONVERSIONS
        template<Sombrero::Float T>
        constexpr FastVector3(const T& value) : Vector3(value, value, value) {}

        template<Sombrero::HasRGBA T>
        constexpr FastVector3(const T& color) : Vector3(color.r, color.g, color.b) {}
        
        template<Sombrero::HasRGB T>
        constexpr FastVector3(const T& color) : Vector3(color.r, color.g, color.b) {}

        template<Sombrero::Has4D T>
        constexpr FastVector3(const T& vector) : Vector3(vector.x, vector.y, vector.z) {}

        template<Sombrero::Has3D T>
        constexpr FastVector3(const T& vector) : Vector3(vector.x, vector.y, vector.z) {}
        
        template<Sombrero::Has2D T>
        constexpr FastVector3(const T& vector) : Vector3(vector.x, vector.y, 0.0f) {}
#endif
        constexpr FastVector3(float x = 0.0f, float y = 0.0f, float z = 0.0f) : Vector3(x, y, z) {}


        CONSTEXPR_GETTER(one, {1.0f, 1.0f, 1.0f})
        CONSTEXPR_GETTER(zero, {0.0f, 0.0f, 0.0f})
        
        CONSTEXPR_GETTER(up, {0.0f, 1.0f, 0.0f})
        CONSTEXPR_GETTER(down, {0.0f, -1.0f, 0.0f})
        CONSTEXPR_GETTER(left, {-1.0f, 0.0f, 0.0f})
        CONSTEXPR_GETTER(right, {1.0f, 0.0f, 0.0f})
        CONSTEXPR_GETTER(forward, {0.0f, 0.0f, 1.0f})
        CONSTEXPR_GETTER(back, {0.0f, 0.0f, -1.0f})

        inline std::string toString() const {
            return vector3Str(*this);
        }

        inline static FastVector3 Lerp(FastVector3 const& a, FastVector3 const& b, float const& t)
        {
            return LerpUnclamped(a, b, Clamp01(t));
        }

        static constexpr FastVector3 LerpUnclamped(FastVector3 const& a, FastVector3 const& b, float const& t)
        {
            return FastVector3(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t);
        }

        constexpr float sqrMagnitude() const {
            return ((x * x) + (y * y) + (z * z));
        }

        constexpr float Magnitude() const {
            return ::Sombrero::sqroot(double(x * x) + double(y * y) + double(z * z));
        }
        constexpr float sqrDistance(const FastVector3& b) const {
            float dx = x - b.x;
            float dy = y - b.y;
            float dz = z - b.z;
            return dx * dx + dy * dy + dz * dz;
        }

        constexpr float Distance(const FastVector3& b) const {
            float dx = x - b.x;
            float dy = y - b.y;
            float dz = z - b.z;
            return ::Sombrero::sqroot(dx * dx + dy * dy + dz * dz);
        }

        static constexpr FastVector3 Normalize(const FastVector3& vec) {
            float magnitude = vec.Magnitude();
            if (magnitude == 0.0f) return zero();
            return vec / magnitude;
        }

        constexpr FastVector3 get_normalized() const
        {
            return FastVector3::Normalize(*this);
        }

        constexpr void Normalize() {
            NormalizeFast();
        }

        // In case codegen method takes over
        constexpr void NormalizeFast() {
            float magnitude = Magnitude();
            if (magnitude < 1E-5f) {
                x = 0.0f;
                y = 0.0f;
                z = 0.0f;
                return;
            }
            *this /= magnitude;
        }

        static constexpr float Dot(FastVector3 const& lhs, FastVector3 const& rhs)
		{
			return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
		}

#define operatorOverload(name, operatore) \
        constexpr FastVector3 operator operatore(const FastVector3& b) const { \
            return FastVector3(this->x operatore b.x, this->y operatore b.y, this->z operatore b.z); \
        }                                \
        constexpr FastVector3 operator operatore(const UnityEngine::Vector3& b) const { \
            return FastVector3(this->x operatore b.x, this->y operatore b.y, this->z operatore b.z); \
        }                                \
        constexpr FastVector3 operator operatore(float const& b) const { \
            return FastVector3(this->x operatore b, this->y operatore b, this->z operatore b); \
        }                                 \
        constexpr FastVector3& operator operatore##=(float const& bb) {  \
            x operatore##= bb;                       \
            y operatore##= bb;                        \
            z operatore##= bb;                        \
            return *this; \
        }                                 \
        constexpr FastVector3& operator operatore##=(const FastVector3& bb) {  \
            x operatore##= bb.x;                       \
            y operatore##= bb.y;                        \
            z operatore##= bb.z;                        \
            return *this; \
        } \
        constexpr FastVector3& operator operatore##=(const UnityEngine::Vector3& bb) {  \
            x operatore##= bb.x;                       \
            y operatore##= bb.y;                        \
            z operatore##= bb.z;                        \
            return *this; \
        }

        operatorOverload(add, +)
        operatorOverload(subtract, -)
        operatorOverload(multiply, *)
        operatorOverload(divide, /)

#undef operatorOverload

        constexpr FastVector3 operator-() const {
            return FastVector3(-x, -y, -z);
        }


        constexpr bool operator ==(const UnityEngine::Vector3& lhs) const {
            return lhs.x == x && lhs.y == y && lhs.z == z;
        }

        constexpr bool operator !=(const UnityEngine::Vector3& lhs) const {
            return lhs.x != x || lhs.y != y || lhs.z != z;
        }

        constexpr float& operator[](int i) {
            return (&x)[i];
        }
    };


// Method operators
#define operatorOverload(name, operator) \
    inline static FastVector3 vector3##name (UnityEngine::Vector3 const& a, UnityEngine::Vector3 const& b) { \
        return FastVector3(a.x operator b.x, a.y operator b.y, a.z operator b.z); \
    } \
    inline static FastVector3 vector3##name (UnityEngine::Vector3 const& a, float const& b) { \
        return FastVector3(a.x operator b, a.y operator b, a.z operator b); \
    } \


    operatorOverload(add, +)
    operatorOverload(subtract, -)
    operatorOverload(multiply, *)
    operatorOverload(scale, *)
    operatorOverload(divide, /)

#undef operatorOverload

#ifdef HAS_CODEGEN
    static_assert(sizeof(UnityEngine::Vector3) == sizeof(FastVector3));
#endif
}
DEFINE_IL2CPP_CLASS(Sombrero::FastVector3, "UnityEngine", "Vector3");
#undef CONSTEXPR_GETTER

namespace std {
    template <> 
    struct hash<Sombrero::FastVector3>
    {
        size_t operator()(const Sombrero::FastVector3 & v) const
        {
            std::hash<float> h;
            return h(v.x) ^ h(v.y) ^ h(v.z);
        }
    };
}