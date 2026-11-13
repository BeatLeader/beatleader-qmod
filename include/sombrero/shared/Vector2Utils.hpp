#pragma once

#include "MiscUtils.hpp"
#include "Concepts.hpp"

#include <utility>

#include "beatsaber-hook/shared/types.hpp"
#ifdef HAS_CODEGEN
#include "UnityEngine/Vector2.hpp"
#endif

#define CONSTEXPR_GETTER(name, ...) \
constexpr static inline FastVector2 name() {\
    return __VA_ARGS__;\
}

#ifndef HAS_CODEGEN
// TODO: Will this break things?
namespace UnityEngine
{
    struct Vector2
    {
        float x, y;
        
        constexpr Vector2(float x = 0.0f, float y = 0.0f) : x(x), y(y) {}
    };
}
#endif

namespace Sombrero {

    struct FastVector2;

    inline static std::string vector2Str(UnityEngine::Vector2 const &vector2)
    {
        return std::to_string(vector2.x) + ", " + std::to_string(vector2.y);
    }

    struct FastVector2 : public UnityEngine::Vector2 {
    public:
        // Implicit convert of vector
        constexpr FastVector2(const Vector2& vector) : Vector2(vector.x, vector.y) {}
        constexpr FastVector2(float x = 0.0f, float y = 0.0f) : Vector2(x, y) {}

#ifdef USE_SOMBRERO_IMPLICIT_CONVERSIONS
        template<Sombrero::Float T>
        constexpr FastVector2(const T& value) : Vector2(value, value) {}

        template<Sombrero::HasRGBA T>
        constexpr FastVector2(const T& color) : Vector2(color.r, color.g) {}
        
        template<Sombrero::HasRGB T>
        constexpr FastVector2(const T& color) : Vector2(color.r, color.g) {}

        template<Sombrero::Has4D T>
        constexpr FastVector2(const T& vector) : Vector2(vector.x, vector.y) {}

        template<Sombrero::Has3D T>
        constexpr FastVector2(const T& vector) : Vector2(vector.x, vector.y) {}

        template<Sombrero::Has2D T>
        constexpr FastVector2(const T& vector) : Vector2(vector.x, vector.y) {}
#endif
        CONSTEXPR_GETTER(one, {1.0f, 1.0f})
        CONSTEXPR_GETTER(zero, {0.0f, 0.0f})

        CONSTEXPR_GETTER(down, {0.0f, -1.0f})
        CONSTEXPR_GETTER(up, {0.0f, 1.0f})
        CONSTEXPR_GETTER(left, {-1.0f, 0.0f})
        CONSTEXPR_GETTER(right, {1.0f, 0.0f})

        inline std::string toString() const {
            return vector2Str(*this);
        }

        inline static FastVector2 Lerp(FastVector2 const& a, FastVector2 const& b, float const& t)
        {
            return LerpUnclamped(a, b, Clamp01(t));
        }

        static constexpr FastVector2 LerpUnclamped(FastVector2 const& a, FastVector2 const& b, float const& t)
        {
            return FastVector2(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t);
        }

        constexpr float Magnitude() const {
            return ::Sombrero::sqroot((x * x) + (y * y));
        }

        constexpr float Distance(const FastVector2& b) const {
            float dx = x - b.x;
            float dy = y - b.y;
            return ::Sombrero::sqroot(dx * dx + dy * dy);
        }

        static constexpr FastVector2 Normalize(FastVector2& vec) {
            float magnitude = vec.Magnitude();
            if (magnitude == 0.0f) return {0.0f, 0.0f};
            return vec / magnitude;
        }

        constexpr void Normalize() {
            NormalizeFast();
        }

        // In case codegen method takes over
        constexpr void NormalizeFast() {
            float magnitude = Magnitude();
            if (magnitude == 0.0f) {
                x = 0.0f;
                y = 0.0f;
            }
            *this /= magnitude;
        }

#define operatorOverload(name, operatore) \
        constexpr FastVector2 operator operatore(const FastVector2& b) const { \
            return FastVector2(this->x operatore b.x, this->y operatore b.y); \
        }                                \
        constexpr FastVector2 operator operatore(const UnityEngine::Vector2& b) const { \
            return FastVector2(this->x operatore b.x, this->y operatore b.y); \
        }                                \
        constexpr FastVector2 operator operatore(float const& b) const { \
            return FastVector2(this->x operatore b, this->y operatore b); \
        }                                 \
        constexpr FastVector2& operator operatore##=(float const& bb) {  \
            x operatore##= bb;                       \
            y operatore##= bb;                        \
            return *this; \
        }                                 \
        constexpr FastVector2& operator operatore##=(const FastVector2& bb) {  \
            x operatore##= bb.x;                       \
            y operatore##= bb.y;                        \
            return *this; \
        } \
        constexpr FastVector2& operator operatore##=(const UnityEngine::Vector2& bb) {  \
            x operatore##= bb.x;                       \
            y operatore##= bb.y;                        \
            return *this; \
        }


        operatorOverload(add, +)
        operatorOverload(subtract, -)
        operatorOverload(multiply, *)
        operatorOverload(divide, /)

#undef operatorOverload

        constexpr FastVector2 operator-() const {
            return FastVector2(-x, -y);
        }


        constexpr bool operator ==(const UnityEngine::Vector2& lhs) const {
            return lhs.x == x && lhs.y == y;
        }

        constexpr bool operator !=(const UnityEngine::Vector2& lhs) const {
            return lhs.x != x || lhs.y != y;
        }

        constexpr float& operator[](int i) {
            return (&x)[i];
        }
    };

#define operatorOverload(name, operator) \
    constexpr static FastVector2 vector2##name(UnityEngine::Vector2 const &a, UnityEngine::Vector2 const &b) \
    { \
        return FastVector2(a.x operator b.x, a.y operator b.y); \
    } \
    constexpr static FastVector2 vector2##name (UnityEngine::Vector2 const& a, float const& b) { \
        return FastVector2(a.x operator b, a.y operator b); \
    } \

    operatorOverload(add, +)
    operatorOverload(subtract, -)
    operatorOverload(multiply, *)
    operatorOverload(scale, *)
    operatorOverload(divide, /)

#undef operatorOverload

#ifdef HAS_CODEGEN
    static_assert(sizeof(UnityEngine::Vector2) == sizeof(FastVector2));
#endif
}
DEFINE_IL2CPP_CLASS(Sombrero::FastVector2, "UnityEngine", "Vector2");
#undef CONSTEXPR_GETTER

namespace std {
    template <> 
    struct hash<Sombrero::FastVector2>
    {
        size_t operator()(const Sombrero::FastVector2 & v) const
        {
            std::hash<float> h;
            return h(v.x) ^ h(v.y);
        }
    };
}