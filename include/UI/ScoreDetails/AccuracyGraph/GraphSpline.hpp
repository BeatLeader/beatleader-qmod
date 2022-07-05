#pragma once

#include "GraphSplineSegment.hpp"
#include "GraphPoint.hpp"
#include "include/Utils/CyclicBuffer.hpp"

#include "UnityEngine/Vector2.hpp"

namespace BeatLeader {
    class GraphSpline {
        public:

        GraphSpline(int capacity) noexcept;
        bool Add(UnityEngine::Vector2 node);
        void FillArray(std::vector<GraphPoint>& destination);
        GraphPoint Evaluate(std::vector<GraphSplineSegment>& buffer, float t);

        CyclicBuffer<GraphSplineSegment> segments;
        CyclicBuffer<UnityEngine::Vector2> handles;
    };
}