#pragma once

#include "GraphPoint.hpp"

#include "UnityEngine/Vector2.hpp"

namespace BeatLeader {
    class GraphSplineSegment {
        public:
        GraphSplineSegment();
        GraphSplineSegment(UnityEngine::Vector2 handleNodeA, UnityEngine::Vector2 handleNodeB, UnityEngine::Vector2 handleNodeC) noexcept;
        BeatLeader::GraphPoint Evaluate(float t);

        private:
        UnityEngine::Vector2 p00;
        UnityEngine::Vector2 p01;
        UnityEngine::Vector2 v00;
        UnityEngine::Vector2 v01;

        UnityEngine::Vector2 p10;
        UnityEngine::Vector2 p11;
        UnityEngine::Vector2 v10;
    };
}