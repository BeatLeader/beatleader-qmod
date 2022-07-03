#include "include/UI/ScoreDetails/AccuracyGraph/GraphSpline.hpp"

#include "main.hpp"

BeatLeader::GraphSpline::GraphSpline(int capacity) noexcept : segments(capacity), handles(3) {
}

bool BeatLeader::GraphSpline::Add(UnityEngine::Vector2 node) {
    if (!handles.Add(node)) return false;
    auto buffer = handles.GetBuffer();
    segments.Add(BeatLeader::GraphSplineSegment(
        buffer[0],
        buffer[1],
        buffer[2]
    ));
    return true;
}

void BeatLeader::GraphSpline::FillArray(std::vector<BeatLeader::GraphPoint>& destination) {
    auto splinesBuffer = segments.GetBuffer();

    for (int i = 0; i < destination.size(); i++) {
        auto t = (float) i / (destination.size() - 1);
        destination[i] = this->Evaluate(splinesBuffer, t);
    }
}

BeatLeader::GraphPoint BeatLeader::GraphSpline::Evaluate(std::vector<BeatLeader::GraphSplineSegment>& buffer, float t) {
    auto tPerSpline = 1.0 / segments.size;
    auto splineIndex = (int) (t / tPerSpline);
    if (splineIndex >= segments.size) splineIndex = segments.size - 1;
    auto splineT = (t - tPerSpline * splineIndex) / tPerSpline;
    return buffer[splineIndex].Evaluate(splineT);
}
