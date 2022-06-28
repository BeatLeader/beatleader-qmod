#include "include/UI/ScoreDetails/AccuracyGraph/GraphPoint.hpp"
#include "include/UI/ScoreDetails/AccuracyGraph/AccuracyGraphUtils.hpp"

BeatLeader::GraphPoint::GraphPoint(UnityEngine::Vector2 position, UnityEngine::Vector2 tangent) noexcept {
    this->position = position;
    this->tangent = tangent;
}

BeatLeader::GraphPoint::GraphPoint() {}
