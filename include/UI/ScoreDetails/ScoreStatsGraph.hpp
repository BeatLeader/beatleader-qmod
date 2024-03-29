#pragma once

#include "HMUI/ModalView.hpp"

#include "bsml/shared/BSML-Lite.hpp"
#include "bsml/shared/bsml.hpp"

#include "include/Models/ScoreStats.hpp"

#include "include/UI/ScoreDetails/AccuracyGraph/AccuracyGraph.hpp"

namespace BeatLeader {
    class ScoreStatsGraph {
        public:
            ScoreStatsGraph(HMUI::ModalView* modal) noexcept;

            HMUI::ImageView* graphBackground;
            
            AccuracyGraph* accuracyGraph;
            
            void setScore(ScoreStats score) const;
            void setSelected(bool selected) const;
    };
}