#pragma once

#include "HMUI/ModalView.hpp"

#include "questui/shared/BeatSaberUI.hpp"
#include "questui/shared/QuestUI.hpp"

#include "include/Models/Score.hpp"
#include "include/UI/PlayerAvatar.hpp"

namespace BeatLeader {
    class ScoreStatsGraph {
        public:
            ScoreStatsGraph(HMUI::ModalView* modal) noexcept;
            
            void setScore(optional<ScoreStats> score);
            void setSelected(bool selected);
    };
}