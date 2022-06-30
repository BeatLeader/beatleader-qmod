#include "include/UI/ScoreDetails/ScoreDetailsUI.hpp"
#include "include/Utils/FormatUtils.hpp"
#include "include/Assets/Sprites.hpp"
#include "include/UI/EmojiSupport.hpp"
#include "include/UI/ScoreDetails/ScoreStatsGrid.hpp"

#include "UnityEngine/Resources.hpp"
#include "HMUI/ImageView.hpp"
#include "UnityEngine/Component.hpp"

#include "questui/shared/CustomTypes/Components/Backgroundable.hpp"

#include "main.hpp"

#include <sstream>

using namespace QuestUI::BeatSaberUI;
using namespace UnityEngine;
using namespace UnityEngine::UI;
using namespace GlobalNamespace;

BeatLeader::ScoreStatsGrid::ScoreStatsGrid(HMUI::ModalView *modal) noexcept {
    for (int i = 0; i < 12; i++)
    {
        accuracyGridCells[i] = ScoreStatsGridCell(modal, i);
    }
}

void BeatLeader::ScoreStatsGrid::setScore(optional<ScoreStats> scoreStats) {
    float max = 10000;
    float min = -10000;
    auto scores = scoreStats->accuracyTracker.gridAcc;

    for (int i = 0; i < 12; i++)
    {
        float score = scores[i];
        if (score == 0) continue;
        if (score > max) max = score;
        if (score < min) min = score;
    }

    for (int i = 0; i < 12; i++) {
        float score = scores[i];
        float ratio = (score - min) / (max - min);
        accuracyGridCells[i].setScore(score, ratio > 1 ? 1 : (ratio < 0 ? 0 : ratio));
    }
}

        
void BeatLeader::ScoreStatsGrid::setSelected(bool selected) {
    for (int i = 0; i < 12; i++)
    {
        accuracyGridCells[i].setSelected(selected);
    }
}