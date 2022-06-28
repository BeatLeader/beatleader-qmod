#include "include/UI/ScoreDetails/AccuracyGraph/ScreenViewTransform.hpp"
#include "include/UI/ScoreDetails/AccuracyGraph/AccuracyGraphUtils.hpp"

BeatLeader::ScreenViewTransform::ScreenViewTransform(UnityEngine::Rect screen, UnityEngine::Rect view) noexcept {
    this->screen = screen;
    this->view = view;
}


UnityEngine::Vector2 BeatLeader::ScreenViewTransform::NormalizeScreenPosition(UnityEngine::Vector2 screenPoint) {
    return GetNormalizedPositionUnclamped(screen, screenPoint);
}

UnityEngine::Vector2 BeatLeader::ScreenViewTransform::NormalizeViewPosition(UnityEngine::Vector2 viewPoint) {
    return GetNormalizedPositionUnclamped(view, viewPoint);
}

UnityEngine::Vector2 BeatLeader::ScreenViewTransform::GetNormalizedPositionUnclamped(UnityEngine::Rect rect, UnityEngine::Vector2 point) {
    return UnityEngine::Vector2(
        (point.x - rect.get_xMin()) / rect.get_width(),
        (point.y - rect.get_yMin()) / rect.get_height()
    );
}

UnityEngine::Vector2 BeatLeader::ScreenViewTransform::TransformPoint(UnityEngine::Vector2 screenPoint) {
    return FromToPoint(screen, view, screenPoint);
}

UnityEngine::Vector2 BeatLeader::ScreenViewTransform::InverseTransformPoint(UnityEngine::Vector2 viewPoint) {
    return FromToPoint(view, screen, viewPoint);
}

UnityEngine::Vector2 BeatLeader::ScreenViewTransform::FromToPoint(UnityEngine::Rect from, UnityEngine::Rect to, UnityEngine::Vector2 point) {
    auto normalized = AccuracyGraphUtils::PointToNormalized(from, point);
    return AccuracyGraphUtils::NormalizedToPoint(to, normalized);
}

UnityEngine::Vector2 BeatLeader::ScreenViewTransform::TransformVector(UnityEngine::Vector2 screenVector) {
    return FromToVector(screen, view, screenVector);
}

UnityEngine::Vector2 BeatLeader::ScreenViewTransform::InverseTransformVector(UnityEngine::Vector2 viewVector) {
    return FromToVector(view, screen, viewVector);
}

UnityEngine::Vector2 BeatLeader::ScreenViewTransform::FromToVector(UnityEngine::Rect from, UnityEngine::Rect to, UnityEngine::Vector2 vector) {
    auto scale = to.get_size() / from.get_size();
    return vector * scale;
}

UnityEngine::Vector2 BeatLeader::ScreenViewTransform::TransformDirection(UnityEngine::Vector2 screenDirection) {
    return FromToDirection(screen, view, screenDirection);
}

UnityEngine::Vector2 BeatLeader::ScreenViewTransform::InverseTransformDirection(UnityEngine::Vector2 viewDirection) {
    return FromToDirection(view, screen, viewDirection);
}

UnityEngine::Vector2 BeatLeader::ScreenViewTransform::FromToDirection(UnityEngine::Rect from, UnityEngine::Rect to, UnityEngine::Vector2 direction) {
    auto vector = FromToVector(from, to, direction);
    vector.Normalize();
    return vector;
}