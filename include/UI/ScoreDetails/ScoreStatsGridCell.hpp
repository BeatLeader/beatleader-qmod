#pragma once

#include "HMUI/ModalView.hpp"

#include "bsml/shared/BSML-Lite.hpp"
#include "bsml/shared/bsml.hpp"

#include "include/Models/Score.hpp"
#include "include/UI/PlayerAvatar.hpp"

namespace BeatLeader {
    class ScoreStatsGridCell {
        public:
            ScoreStatsGridCell();
            ScoreStatsGridCell(HMUI::ModalView* modal, int index) noexcept;

            TMPro::TextMeshProUGUI* scoreText;
            HMUI::ImageView* imageView;
            
            void setScore(float score, float ratio) const;
            void setSelected(bool selected) const;
    };
}