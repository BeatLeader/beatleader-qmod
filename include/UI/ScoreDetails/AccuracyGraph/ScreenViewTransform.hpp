#pragma once

#include "UnityEngine/Rect.hpp"
#include "UnityEngine/Vector2.hpp"
#include "sombrero/shared/FastVector2.hpp"

namespace BeatLeader {
    class ScreenViewTransform {
        public:
        UnityEngine::Rect screen;
        UnityEngine::Rect view;
        
        ScreenViewTransform(UnityEngine::Rect const &screen, UnityEngine::Rect const &view) noexcept;

        UnityEngine::Vector2 NormalizeScreenPosition(UnityEngine::Vector2 const &screenPoint) const;
        UnityEngine::Vector2 NormalizeViewPosition(UnityEngine::Vector2 const &viewPoint) const;
        static UnityEngine::Vector2 GetNormalizedPositionUnclamped(UnityEngine::Rect const &rect, UnityEngine::Vector2 const &point);
        [[nodiscard]] UnityEngine::Vector2 TransformPoint(UnityEngine::Vector2 const &screenPoint) const;
        [[nodiscard]] UnityEngine::Vector2 InverseTransformPoint(UnityEngine::Vector2 const &viewPoint) const;
        static UnityEngine::Vector2 FromToPoint(UnityEngine::Rect const &from, UnityEngine::Rect const &to, UnityEngine::Vector2 const &point);
        UnityEngine::Vector2 TransformVector(UnityEngine::Vector2 const &screenVector) const;
        UnityEngine::Vector2 InverseTransformVector(UnityEngine::Vector2 viewVector) const;
        static UnityEngine::Vector2 FromToVector(UnityEngine::Rect const &from, UnityEngine::Rect const &to, UnityEngine::Vector2 const &vector);
        UnityEngine::Vector2 TransformDirection(UnityEngine::Vector2 const &screenDirection) const;
        UnityEngine::Vector2 InverseTransformDirection(UnityEngine::Vector2 const &viewDirection) const;
        static UnityEngine::Vector2 FromToDirection(UnityEngine::Rect const &from, UnityEngine::Rect const &to, Sombrero::FastVector2 const &direction);
    };
}