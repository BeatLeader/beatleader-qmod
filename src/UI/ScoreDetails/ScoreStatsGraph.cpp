#include "include/UI/ScoreDetails/ScoreDetailsUI.hpp"

#include "include/Assets/Sprites.hpp"
#include "include/Assets/BundleLoader.hpp"
#include "include/UI/EmojiSupport.hpp"
#include "include/UI/QuestUI.hpp"

#include "include/UI/ScoreDetails/ScoreStatsGraph.hpp"
#include "include/UI/ScoreDetails/AccuracyGraph/AccuracyGraphUtils.hpp"

#include "include/Utils/FormatUtils.hpp"
#include "include/Utils/Range.hpp"

#include "UnityEngine/Resources.hpp"
#include "UnityEngine/Component.hpp"

#include "HMUI/ImageView.hpp"
#include "HMUI/CurvedCanvasSettingsHelper.hpp"
#include "HMUI/CurvedCanvasSettings.hpp"

#include "bsml/shared/BSML/Components/Backgroundable.hpp"

#include "main.hpp"

#include <sstream>

using namespace QuestUI;
using namespace UnityEngine;
using namespace UnityEngine::UI;
using namespace GlobalNamespace;

BeatLeader::ScoreStatsGraph::ScoreStatsGraph(HMUI::ModalView *modal) noexcept {
    graphBackground = CreateImage(modal->get_transform(), Sprites::get_TransparentPixel(), UnityEngine::Vector2(0, -1), UnityEngine::Vector2(60, 28));
    GameObject* gameObj = GameObject::New_ctor("AccuracyGraph");
    
    this->accuracyGraph = gameObj->AddComponent<BeatLeader::AccuracyGraph*>();
    auto graphLine = gameObj->AddComponent<BeatLeader::AccuracyGraphLine*>();
    graphLine->get_transform()->SetParent(graphBackground->get_transform(), false);
    
    RectTransform* rectTransform = graphLine->get_transform().cast<RectTransform>();
    rectTransform->set_anchorMin(UnityEngine::Vector2(0.5f, 0.5f));
    rectTransform->set_anchorMax(UnityEngine::Vector2(0.5f, 0.5f));
    rectTransform->set_anchoredPosition(UnityEngine::Vector2(0, 0));
    rectTransform->set_sizeDelta(UnityEngine::Vector2(60, 30));
    gameObj->AddComponent<LayoutElement*>();

    this->accuracyGraph->Construct(graphBackground, graphLine, modal);
}

void BeatLeader::ScoreStatsGraph::setScore(ScoreStats score) const {
    auto points = score.scoreGraphTracker.graph;
    
    accuracyGraph->Setup(il2cpp_utils::vectorToArray(points), score.winTracker.endTime);
}

void BeatLeader::ScoreStatsGraph::setSelected(bool selected) const {
    graphBackground->get_gameObject()->SetActive(selected);
}