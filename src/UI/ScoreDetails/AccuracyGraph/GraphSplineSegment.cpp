#include "include/UI/ScoreDetails/AccuracyGraph/GraphSplineSegment.hpp"

BeatLeader::GraphSplineSegment::GraphSplineSegment(UnityEngine::Vector2 handleNodeA, UnityEngine::Vector2 handleNodeB, UnityEngine::Vector2 handleNodeC) noexcept {
    p00 = UnityEngine::Vector2::op_Division(UnityEngine::Vector2::op_Addition(handleNodeA, handleNodeB), 2.0f);
    p01 = handleNodeB;
    auto p02 = UnityEngine::Vector2::op_Division(UnityEngine::Vector2::op_Addition(handleNodeB, handleNodeC), 2.0f);
    v00 = UnityEngine::Vector2::op_Subtraction(p01, p00);
    v01 = UnityEngine::Vector2::op_Subtraction(p02, p01);
}

BeatLeader::GraphSplineSegment::GraphSplineSegment() {}

BeatLeader::GraphPoint BeatLeader::GraphSplineSegment::Evaluate(float t) {
    p10 = UnityEngine::Vector2::op_Addition(p00, UnityEngine::Vector2::op_Multiply(v00, t));
    p11 = UnityEngine::Vector2::op_Addition(p01, UnityEngine::Vector2::op_Multiply(v01, t));
    v10 = UnityEngine::Vector2::op_Subtraction(p11, p10);
    return BeatLeader::GraphPoint(
        UnityEngine::Vector2::op_Addition(p10, UnityEngine::Vector2::op_Multiply(v10, t)),
        v10.get_normalized()
    );
}