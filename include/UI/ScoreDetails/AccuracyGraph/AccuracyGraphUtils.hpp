#pragma once

#include "UnityEngine/Vector2.hpp"
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Rect.hpp"
#include <vector>

#include "sombrero/shared/FastVector2.hpp"

using namespace std;

namespace AccuracyGraphUtils {
    constexpr float Clamp01(float value) {
        if (value < 0)
            return 0;
        else if (value > 1)
            return 1;
        else
            return value;
    }

    constexpr float InverseLerp(float a, float b, float value) {
        if (a != b)
            return Clamp01((value - a) / (b - a));
        else
            return 0.0;
    }

    constexpr float Lerp(float a, float b, float t) {
        return a + (b - a) * Clamp01(t);
    }

    inline Sombrero::FastVector2 PointToNormalized(UnityEngine::Rect const& rectangle, Sombrero::FastVector2 const& point) {
        return Sombrero::FastVector2(
            InverseLerp(rectangle.m_XMin, rectangle.m_Width + rectangle.m_XMin, point.x),
            InverseLerp(rectangle.m_YMin, rectangle.m_Height + rectangle.m_YMin, point.y)
        );
    }

    inline Sombrero::FastVector2 NormalizedToPoint(UnityEngine::Rect rectangle, Sombrero::FastVector2 normalizedRectCoordinates) {
        return Sombrero::FastVector2(
            Lerp(rectangle.m_XMin, rectangle.m_Width + rectangle.m_XMin, normalizedRectCoordinates.x),
            Lerp(rectangle.m_YMin, rectangle.m_Height + rectangle.m_YMin, normalizedRectCoordinates.y)
        );
    }

    const float ReduceAngleMargin = 10;
    const float ReduceProximityMargin = 0.1;

    inline void ReducePositionsList(vector<Sombrero::FastVector2>& positions, UnityEngine::Rect const& viewRect) {
        auto startIndex = 1;
        while (startIndex < positions.size() - 1) {
            auto i = startIndex;
            for (; i < positions.size() - 1; i++) {
                auto prev = PointToNormalized(viewRect, positions[i - 1]);
                auto curr = PointToNormalized(viewRect, positions[i]);
                auto next = PointToNormalized(viewRect, positions[i + 1]);

                auto a = next - curr;
                auto b = curr - prev;
                if (a.Magnitude() > ReduceProximityMargin || b.Magnitude() > ReduceProximityMargin) continue;
                if (Sombrero::FastVector2::Angle(a, b) > ReduceAngleMargin) continue;
                positions.erase(positions.begin() + i);
                break;
            }

            startIndex = i;
        }
    }

    const float MinimalXForScaling = 0.05;

    inline void PostProcessPoints(ArrayW<float> points, vector<Sombrero::FastVector2>* positions, UnityEngine::Rect* viewRect) {
        auto result = vector<Sombrero::FastVector2>();

        float yMin = 1000000;
        float yMax = -1000000;

        int length = points.Length();
        result.reserve(length);

        for (auto i = 0; i < length; i++) {
            auto x = (float) i / (length - 1);
            auto y = points[i];
            result.emplace_back(x, y);
            if (x < MinimalXForScaling) continue;
            if (y > yMax) yMax = y;
            if (y < yMin) yMin = y;
        }

        auto margin = (yMax - yMin) * 0.2f;
        auto resultViewRect = UnityEngine::Rect::MinMaxRect(-0.04f, yMin - margin, 1.04f, yMax + margin);

        ReducePositionsList(result, resultViewRect);

        *positions = result;
        *viewRect = resultViewRect;
    }

    inline Sombrero::FastVector2 TransformPointFrom3DToCanvas(UnityEngine::Vector3 point, float canvasRadius) {
        if (canvasRadius < 1e-10) return Sombrero::FastVector2(point.x, point.y);
        float x = (float)asin(point.x / canvasRadius) * canvasRadius;
        return Sombrero::FastVector2(x, point.y);
    }
}