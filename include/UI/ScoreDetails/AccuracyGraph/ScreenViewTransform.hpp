#pragma once

#include "UnityEngine/Rect.hpp"
#include "UnityEngine/Vector2.hpp"

namespace BeatLeader {
    class ScreenViewTransform {
        public:
        UnityEngine::Rect screen;
        UnityEngine::Rect view;
        
        ScreenViewTransform(UnityEngine::Rect screen, UnityEngine::Rect view) noexcept;

        UnityEngine::Vector2 NormalizeScreenPosition(UnityEngine::Vector2 screenPoint);
        UnityEngine::Vector2 NormalizeViewPosition(UnityEngine::Vector2 viewPoint);
        UnityEngine::Vector2 GetNormalizedPositionUnclamped(UnityEngine::Rect rect, UnityEngine::Vector2 point);
        UnityEngine::Vector2 TransformPoint(UnityEngine::Vector2 screenPoint);
        UnityEngine::Vector2 InverseTransformPoint(UnityEngine::Vector2 viewPoint);
        UnityEngine::Vector2 FromToPoint(UnityEngine::Rect from, UnityEngine::Rect to, UnityEngine::Vector2 point);
        UnityEngine::Vector2 TransformVector(UnityEngine::Vector2 screenVector);
        UnityEngine::Vector2 InverseTransformVector(UnityEngine::Vector2 viewVector);
        UnityEngine::Vector2 FromToVector(UnityEngine::Rect from, UnityEngine::Rect to, UnityEngine::Vector2 vector);
        UnityEngine::Vector2 TransformDirection(UnityEngine::Vector2 screenDirection);
        UnityEngine::Vector2 InverseTransformDirection(UnityEngine::Vector2 viewDirection);
        UnityEngine::Vector2 FromToDirection(UnityEngine::Rect from, UnityEngine::Rect to, UnityEngine::Vector2 direction);
    };
}