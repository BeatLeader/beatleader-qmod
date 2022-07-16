#include "include/UI/ScoreDetails/AccuracyGraph/ScreenViewTransform.hpp"
#include "include/UI/ScoreDetails/AccuracyGraph/AccuracyGraphUtils.hpp"

BeatLeader::ScreenViewTransform::ScreenViewTransform(UnityEngine::Rect const &screen, UnityEngine::Rect const &view) noexcept {
    this->screen = screen;
    this->view = view;
}


UnityEngine::Vector2 BeatLeader::ScreenViewTransform::NormalizeScreenPosition(UnityEngine::Vector2 const &screenPoint) const {
    return GetNormalizedPositionUnclamped(screen, screenPoint);
}

UnityEngine::Vector2 BeatLeader::ScreenViewTransform::NormalizeViewPosition(UnityEngine::Vector2 const &viewPoint) const {
    return GetNormalizedPositionUnclamped(view, viewPoint);
}

UnityEngine::Vector2 BeatLeader::ScreenViewTransform::GetNormalizedPositionUnclamped(UnityEngine::Rect const &rect, UnityEngine::Vector2 const &point) {
    return UnityEngine::Vector2(
        (point.x - rect.m_XMin) / rect.m_Width,
        (point.y - rect.m_YMin) / rect.m_Height
    );
}

UnityEngine::Vector2 BeatLeader::ScreenViewTransform::TransformPoint(UnityEngine::Vector2 const &screenPoint) const {
    return FromToPoint(screen, view, screenPoint);
}

UnityEngine::Vector2 BeatLeader::ScreenViewTransform::InverseTransformPoint(UnityEngine::Vector2 const &viewPoint) const {
    return FromToPoint(view, screen, viewPoint);
}

UnityEngine::Vector2 BeatLeader::ScreenViewTransform::FromToPoint(UnityEngine::Rect const &from, UnityEngine::Rect const &to, UnityEngine::Vector2 const &point) {
    auto normalized = AccuracyGraphUtils::PointToNormalized(from, point);
    return AccuracyGraphUtils::NormalizedToPoint(to, normalized);
}

UnityEngine::Vector2 BeatLeader::ScreenViewTransform::TransformVector(UnityEngine::Vector2 const &screenVector) const {
    return FromToVector(screen, view, screenVector);
}

UnityEngine::Vector2 BeatLeader::ScreenViewTransform::InverseTransformVector(UnityEngine::Vector2 viewVector) const {
    return FromToVector(view, screen, viewVector);
}

UnityEngine::Vector2 BeatLeader::ScreenViewTransform::FromToVector(UnityEngine::Rect const &from, UnityEngine::Rect const &to, UnityEngine::Vector2 const &vector) {
    auto scaleFn = [](auto&& r) constexpr {return Sombrero::FastVector2(r.m_Width, r.m_Height);};
    auto scale = scaleFn(to) / scaleFn(from);
    return vector * scale;
}

UnityEngine::Vector2 BeatLeader::ScreenViewTransform::TransformDirection(UnityEngine::Vector2 const &screenDirection) const {
    return FromToDirection(screen, view, screenDirection);
}

UnityEngine::Vector2 BeatLeader::ScreenViewTransform::InverseTransformDirection(
        UnityEngine::Vector2 const &viewDirection) const {
    return FromToDirection(view, screen, viewDirection);
}

UnityEngine::Vector2 BeatLeader::ScreenViewTransform::FromToDirection(UnityEngine::Rect const &from, UnityEngine::Rect const &to, Sombrero::FastVector2 const &direction) {
    auto vector = FromToVector(from, to, direction);
    vector.Normalize();
    return vector;
}