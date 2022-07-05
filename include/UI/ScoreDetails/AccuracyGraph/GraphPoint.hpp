#pragma once

#include "UnityEngine/Vector2.hpp"

namespace BeatLeader {
    class GraphPoint {
        public:
        UnityEngine::Vector2 position;
        UnityEngine::Vector2 tangent;
        
        GraphPoint();
        GraphPoint(UnityEngine::Vector2 position, UnityEngine::Vector2 tangent) noexcept;
    };
}