#pragma once

#include "UnityEngine/Vector2.hpp"
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Rect.hpp"
#include <vector>

using namespace std;

namespace AccuracyGraphUtils {
    inline float Clamp01(float value) {
        if (value < 0)
            return 0;
        else if (value > 1)
            return 1;
        else
            return value;
    }

    inline float InverseLerp(float a, float b, float value) {
        if (a != b)
            return Clamp01((value - a) / (b - a));
        else
            return 0.0;
    }

    inline float Lerp(float a, float b, float t) {
        return a + (b - a) * Clamp01(t);
    }

    inline UnityEngine::Vector2 PointToNormalized(UnityEngine::Rect rectangle, UnityEngine::Vector2 point) {
        return UnityEngine::Vector2(
            InverseLerp(rectangle.get_x(), rectangle.get_xMax(), point.x),
            InverseLerp(rectangle.get_y(), rectangle.get_yMax(), point.y)
        );
    }

    inline UnityEngine::Vector2 NormalizedToPoint(UnityEngine::Rect rectangle, UnityEngine::Vector2 normalizedRectCoordinates) {
        return UnityEngine::Vector2(
            Lerp(rectangle.get_x(), rectangle.get_xMax(), normalizedRectCoordinates.x),
            Lerp(rectangle.get_y(), rectangle.get_yMax(), normalizedRectCoordinates.y)
        );
    }

    const float ReduceAngleMargin = 10;
    const float ReduceProximityMargin = 0.1;

    inline void ReducePositionsList(vector<UnityEngine::Vector2> positions, UnityEngine::Rect viewRect) {
        auto startIndex = 1;
        while (startIndex < positions.size() - 1) {
            auto i = startIndex;
            for (; i < positions.size() - 1; i++) {
                auto prev = PointToNormalized(viewRect, positions[i - 1]);
                auto curr = PointToNormalized(viewRect, positions[i]);
                auto next = PointToNormalized(viewRect, positions[i + 1]);

                auto a = next - curr;
                auto b = curr - prev;
                if (a.get_magnitude() > ReduceProximityMargin || b.get_magnitude() > ReduceProximityMargin) continue;
                if (UnityEngine::Vector2::Angle(a, b) > ReduceAngleMargin) continue;
                positions.erase(positions.begin() + i);
                break;
            }

            startIndex = i;
        }
    }

    const float MinimalXForScaling = 0.05;

    inline void PostProcessPoints(ArrayW<float> points, vector<UnityEngine::Vector2>* positions, UnityEngine::Rect* viewRect) {
        auto result = vector<UnityEngine::Vector2>();

        float yMin = 1000000;
        float yMax = -1000000;

        int length = points.Length();

        for (auto i = 0; i < length; i++) {
            auto x = (float) i / (length - 1);
            auto y = points[i];
            result.push_back(UnityEngine::Vector2(x, y));
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

    inline UnityEngine::Vector2 TransformPointFrom3DToCanvas(UnityEngine::Vector3 point, float canvasRadius) {
        if (canvasRadius < 1e-10) return UnityEngine::Vector2(point.x, point.y);
        float x = (float)asin(point.x / canvasRadius) * canvasRadius;
        return UnityEngine::Vector2(x, point.y);
    }
}