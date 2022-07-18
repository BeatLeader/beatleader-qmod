#pragma once

#include "HMUI/ModalView.hpp"

#include "questui/shared/BeatSaberUI.hpp"
#include "questui/shared/QuestUI.hpp"

#include "include/Models/ScoreStats.hpp"

#include "include/UI/ScoreDetails/AccuracyGraph/AccuracyGraph.hpp"

namespace BeatLeader {
    class ScoreStatsGraph {
        public:
            ScoreStatsGraph(HMUI::ModalView* modal) noexcept;

            HMUI::ImageView* graphBackground;
            
            AccuracyGraph* accuracyGraph;
            
            void setScore(optional<ScoreStats> score) const;
            void setSelected(bool selected) const;
    };
}