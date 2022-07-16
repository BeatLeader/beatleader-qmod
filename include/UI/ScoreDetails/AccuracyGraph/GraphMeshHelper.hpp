#pragma once

#include "ScreenViewTransform.hpp"
#include "GraphPoint.hpp"
#include "GraphSpline.hpp"

#include "UnityEngine/Vector2.hpp"
#include "UnityEngine/UI/VertexHelper.hpp"

#include <vector>

using namespace std;

namespace BeatLeader {
    class GraphMeshHelper {
        public:
        GraphMeshHelper(int horizontalResolution, int verticalResolution, float lineThickness) noexcept;

        void SetPoints(ArrayW<UnityEngine::Vector2> points);
        void PopulateMesh(UnityEngine::UI::VertexHelper* vh, ScreenViewTransform svt, float canvasRadius);

        private:
        int verticalResolution;
        int rowsCount;
        int columnsCount;
        int vertexCount;
        float lineThickness;

        vector<UnityEngine::Vector2> uv0;
        vector<GraphPoint> pointsArray;

        optional<GraphSpline> spline;
        vector<UnityEngine::Vector2> CreateUvArray();
        void PopulateTrianglesArray(int horizontalResolution, int verticalResolution);
        int GetVertexIndex(int columnIndex, int rowIndex) const;

        vector<int> triangles;
    };
}