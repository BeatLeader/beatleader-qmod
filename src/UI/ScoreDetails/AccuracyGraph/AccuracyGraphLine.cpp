#include "include/UI/ScoreDetails/AccuracyGraph/AccuracyGraphLine.hpp"
#include "include/UI/ScoreDetails/AccuracyGraph/GraphMeshHelper.hpp"

#include "UnityEngine/RectTransformUtility.hpp"

#include "main.hpp"

DEFINE_TYPE(BeatLeader, AccuracyGraphLine);

static BeatLeader::GraphMeshHelper graphMeshHelper = BeatLeader::GraphMeshHelper(500, 1, 0.2);

void BeatLeader::AccuracyGraphLine::Setup(ArrayWrapper<UnityEngine::Vector2> points, UnityEngine::Rect viewRect, float canvasRadius) {
    this->viewRect = viewRect;
    this->canvasRadius = canvasRadius;

    graphMeshHelper.SetPoints(points);

    this->SetVerticesDirty();
}

void BeatLeader::AccuracyGraphLine::OnPopulateMesh(UnityEngine::UI::VertexHelper* vh) {
    auto screenRect = UnityEngine::RectTransformUtility::PixelAdjustRect(this->get_rectTransform(), this->get_canvas());
    auto screenViewTransform = ScreenViewTransform(screenRect, this->viewRect);

    graphMeshHelper.PopulateMesh(vh, screenViewTransform, this->canvasRadius);
}