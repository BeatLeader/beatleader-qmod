#include "UI/Christmas/ChristmasTreeLevel.hpp"

DEFINE_TYPE(BeatLeader, ChristmasTreeLevel);

namespace BeatLeader {
    void ChristmasTreeLevel::Draw(float factor) {
        const int segments = 32; // Number of segments to approximate the circles
        const float angleStep = 360.0f / segments;

        float scaledTopHeight = topHeight * factor;
        float scaledBottomHeight = bottomHeight * factor;
        float scaledTopRadius = topRadius * factor;
        float scaledBottomRadius = bottomRadius * factor;

        auto position = get_transform()->get_position();
        auto topCenter = UnityEngine::Vector3::op_Addition(position, UnityEngine::Vector3::op_Multiply(UnityEngine::Vector3::get_up(), scaledTopHeight));
        auto bottomCenter = UnityEngine::Vector3::op_Addition(position, UnityEngine::Vector3::op_Multiply(UnityEngine::Vector3::get_up(), scaledBottomHeight));

        // Draw the top circle in red
        UnityEngine::Gizmos::set_color(UnityEngine::Color::get_red());
        DrawCircle(topCenter, scaledTopRadius, segments);

        // Draw the bottom circle in blue
        UnityEngine::Gizmos::set_color(UnityEngine::Color::get_blue());
        DrawCircle(bottomCenter, scaledBottomRadius, segments);

        // Draw the sides of the truncated cone
        UnityEngine::Gizmos::set_color(UnityEngine::Color::get_white());
        auto previousTopPoint = UnityEngine::Vector3::op_Addition(topCenter, UnityEngine::Vector3::op_Multiply(UnityEngine::Vector3::get_right(), scaledTopRadius));
        auto previousBottomPoint = UnityEngine::Vector3::op_Addition(bottomCenter, UnityEngine::Vector3::op_Multiply(UnityEngine::Vector3::get_right(), scaledBottomRadius));

        for (int i = 1; i <= segments; i++) {
            float angle = angleStep * i * (3.14159265358979f * 2.0f / 360.0f);

            // Calculate the points for this segment
            auto newTopPoint = UnityEngine::Vector3::op_Addition(topCenter, UnityEngine::Vector3(
                UnityEngine::Mathf::Cos(angle) * scaledTopRadius,
                0,
                UnityEngine::Mathf::Sin(angle) * scaledTopRadius
            ));
            
            auto newBottomPoint = UnityEngine::Vector3::op_Addition(bottomCenter, UnityEngine::Vector3(
                UnityEngine::Mathf::Cos(angle) * scaledBottomRadius,
                0,
                UnityEngine::Mathf::Sin(angle) * scaledBottomRadius
            ));

            // Connect the previous points to the new points
            UnityEngine::Gizmos::DrawLine(previousTopPoint, newTopPoint);         // Top circle
            UnityEngine::Gizmos::DrawLine(previousBottomPoint, newBottomPoint);   // Bottom circle
            UnityEngine::Gizmos::DrawLine(previousTopPoint, previousBottomPoint); // Side line

            // Update the previous points
            previousTopPoint = newTopPoint;
            previousBottomPoint = newBottomPoint;
        }
    }

    void ChristmasTreeLevel::DrawCircle(UnityEngine::Vector3 center, float radius, int segments) {
        float angleStep = 360.0f / segments;
        auto previousPoint = UnityEngine::Vector3::op_Addition(center, UnityEngine::Vector3::op_Multiply(UnityEngine::Vector3::get_right(), radius));

        for (int i = 1; i <= segments; i++) {
            float angle = angleStep * i * (3.14159265358979f * 2.0f / 360.0f);
            auto newPoint = UnityEngine::Vector3::op_Addition(center, UnityEngine::Vector3(
                UnityEngine::Mathf::Cos(angle) * radius,
                0,
                UnityEngine::Mathf::Sin(angle) * radius
            ));

            UnityEngine::Gizmos::DrawLine(previousPoint, newPoint);
            previousPoint = newPoint;
        }
    }
}