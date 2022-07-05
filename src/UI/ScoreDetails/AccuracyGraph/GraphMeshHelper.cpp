#include "include/UI/ScoreDetails/AccuracyGraph/GraphMeshHelper.hpp"
#include "UnityEngine/Color32.hpp"

#include "main.hpp"

BeatLeader::GraphMeshHelper::GraphMeshHelper(int horizontalResolution, int verticalResolution, float lineThickness) noexcept {
    this->verticalResolution = verticalResolution;
    rowsCount = verticalResolution + 1;
    columnsCount = horizontalResolution + 1;
    this->lineThickness = lineThickness;
    vertexCount = rowsCount * columnsCount;

    uv0 = CreateUvArray();
    pointsArray = vector<BeatLeader::GraphPoint>(columnsCount);

    int quadCount = horizontalResolution * verticalResolution;
    triangles = vector<int>(quadCount * 6);
    PopulateTrianglesArray(verticalResolution, horizontalResolution);
}

void BeatLeader::GraphMeshHelper::SetPoints(ArrayW<UnityEngine::Vector2> points) {
    int size = points.Length();

    if (size <= 1) {
        spline = nullopt;
        return;
    }

    spline.emplace(GraphSpline(size));
    spline->Add(points[0]);
    spline->Add(points[0]);
    for (int i = 0; i < size; i++) {
        spline->Add(points[i]);
    }
    spline->Add(points[size - 1]);
}

void BeatLeader::GraphMeshHelper::PopulateMesh(UnityEngine::UI::VertexHelper* vh, BeatLeader::ScreenViewTransform svt, float canvasRadius) {
    vh->Clear();
    
    if (spline == nullopt) return;
    spline->FillArray(pointsArray);

    for (int columnIndex = 0; columnIndex < columnsCount; columnIndex++) {
        auto node = pointsArray[columnIndex];

        auto screenNodePosition = svt.InverseTransformPoint(node.position);
        auto screenNodeTangent = svt.InverseTransformDirection(node.tangent);
        auto screenNodeNormal = UnityEngine::Vector2(screenNodeTangent.y, -screenNodeTangent.x);

        for (int rowIndex = 0; rowIndex < rowsCount; rowIndex++) {
            float verticalRatio = ((float) rowIndex / verticalResolution - 0.5) * 2.0;
            int vertexIndex = GetVertexIndex(rowIndex, columnIndex);

            auto widthOffset = verticalRatio * lineThickness * screenNodeNormal;
            auto screenVertexPosition = screenNodePosition + widthOffset;
            auto screenNormalizedPosition = svt.NormalizeScreenPosition(screenVertexPosition);

            vh->AddVert(
                UnityEngine::Vector3(screenVertexPosition.x, screenVertexPosition.y, 0),
                UnityEngine::Color32(1, 1, 1, 1),
                uv0[vertexIndex],
                screenNormalizedPosition,
                UnityEngine::Vector2(canvasRadius, 0),
                node.position,
                UnityEngine::Vector3::get_one(),
                UnityEngine::Vector4::get_one()
            );
        }
    }

    for (int i = 0; i < triangles.size(); i += 3) vh->AddTriangle(triangles[i], triangles[i + 1], triangles[i + 2]);
}

int BeatLeader::GraphMeshHelper::GetVertexIndex(int columnIndex, int rowIndex) {
    return rowIndex * rowsCount + columnIndex;
}

vector<UnityEngine::Vector2> BeatLeader::GraphMeshHelper::CreateUvArray() {
    auto tmp = vector<UnityEngine::Vector2>(vertexCount);

    for (int rowIndex = 0; rowIndex < columnsCount; rowIndex++) {
        float horizontalRatio = (float) rowIndex / (columnsCount - 1);
        for (int columnIndex = 0; columnIndex < rowsCount; columnIndex++) {
            float verticalRatio = (float) columnIndex / (rowsCount - 1);
            int vertexIndex = GetVertexIndex(columnIndex, rowIndex);
            tmp[vertexIndex] = UnityEngine::Vector2(verticalRatio, horizontalRatio);
        }
    }

    return tmp;
}

void BeatLeader::GraphMeshHelper::PopulateTrianglesArray(int horizontalResolution, int verticalResolution) {
    for (int i = 0; i < verticalResolution; i++)
    for (int j = 0; j < horizontalResolution; j++) {
        int quadIndex = i * horizontalResolution + j;

        int topLeftVertexIndex = GetVertexIndex(j, i);
        int topRightVertexIndex = GetVertexIndex(j + 1, i);
        int bottomLeftVertexIndex = GetVertexIndex(j, i + 1);
        int bottomRightVertexIndex = GetVertexIndex(j + 1, i + 1);

        int leftTriangleIndex = quadIndex * 6;
        triangles[leftTriangleIndex + 0] = topLeftVertexIndex;
        triangles[leftTriangleIndex + 1] = bottomLeftVertexIndex;
        triangles[leftTriangleIndex + 2] = bottomRightVertexIndex;

        int rightTriangleIndex = leftTriangleIndex + 3;
        triangles[rightTriangleIndex + 0] = bottomRightVertexIndex;
        triangles[rightTriangleIndex + 1] = topRightVertexIndex;
        triangles[rightTriangleIndex + 2] = topLeftVertexIndex;
    }
}
