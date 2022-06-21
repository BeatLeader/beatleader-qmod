#pragma once

#include "HMUI/ModalView.hpp"

#include "questui/shared/BeatSaberUI.hpp"
#include "questui/shared/QuestUI.hpp"

#include "include/Models/Score.hpp"
#include "include/UI/PlayerAvatar.hpp"

namespace BeatLeader {
    class ScoreStatsGridCell {
        public:
            ScoreStatsGridCell();
            ScoreStatsGridCell(HMUI::ModalView* modal, int index) noexcept;

            TMPro::TextMeshProUGUI* scoreText;
            HMUI::ImageView* imageView;
            
            void setScore(float score, float ratio);
            void setSelected(bool selected);
    };
}