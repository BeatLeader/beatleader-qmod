#pragma once

#include "HMUI/ModalView.hpp"

#include "questui/shared/BeatSaberUI.hpp"
#include "questui/shared/QuestUI.hpp"

#include "include/Models/Score.hpp"
#include "include/UI/PlayerAvatar.hpp"
#include "include/UI/ScoreDetails/ScoreStatsGridCell.hpp"

namespace BeatLeader {
    class ScoreStatsGrid {
        public:
            ScoreStatsGrid(HMUI::ModalView* modal) noexcept;

            ScoreStatsGridCell accuracyGridCells[12];
            
            void setScore(optional<ScoreStats> score);
            void setSelected(bool selected);
    };
}