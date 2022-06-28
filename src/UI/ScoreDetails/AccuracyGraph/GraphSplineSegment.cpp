#include "include/UI/ScoreDetails/AccuracyGraph/GraphSplineSegment.hpp"

BeatLeader::GraphSplineSegment::GraphSplineSegment(UnityEngine::Vector2 handleNodeA, UnityEngine::Vector2 handleNodeB, UnityEngine::Vector2 handleNodeC) noexcept {
    p00 = (handleNodeA + handleNodeB) / 2.0f;
    p01 = handleNodeB;
    auto p02 = (handleNodeB + handleNodeC) / 2.0f;
    v00 = p01 - p00;
    v01 = p02 - p01;
}

BeatLeader::GraphSplineSegment::GraphSplineSegment() {}

BeatLeader::GraphPoint BeatLeader::GraphSplineSegment::Evaluate(float t) {
    p10 = p00 + v00 * t;
    p11 = p01 + v01 * t;
    v10 = p11 - p10;
    return BeatLeader::GraphPoint(
        p10 + v10 * t,
        v10.get_normalized()
    );
}