#pragma once

#include "HMUI/ModalView.hpp"

#include "bsml/shared/BSML-Lite.hpp"
#include "bsml/shared/bsml.hpp"

#include "include/Models/ScoreStats.hpp"
#include "include/UI/PlayerAvatar.hpp"
#include "include/UI/ScoreDetails/ScoreStatsGridCell.hpp"

namespace BeatLeader {
    class ScoreStatsGrid {
        public:
            ScoreStatsGrid(HMUI::ModalView* modal) noexcept;

            ScoreStatsGridCell accuracyGridCells[12];
            
            void setScore(ScoreStats score);
            void setSelected(bool selected);
    };
}