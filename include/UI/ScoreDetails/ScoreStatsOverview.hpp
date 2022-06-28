#pragma once

#include "HMUI/ModalView.hpp"

#include "questui/shared/BeatSaberUI.hpp"
#include "questui/shared/QuestUI.hpp"

#include "include/Models/Score.hpp"
#include "include/UI/PlayerAvatar.hpp"

namespace BeatLeader {
    class ScoreStatsOverview {
        public:
            ScoreStatsOverview(HMUI::ModalView* modal) noexcept;

            TMPro::TextMeshProUGUI* leftPreScore;
            TMPro::TextMeshProUGUI* leftAccScore;
            TMPro::TextMeshProUGUI* leftPostScore;

            TMPro::TextMeshProUGUI* leftScore;
            HMUI::ImageView* leftPieImage;
            
            TMPro::TextMeshProUGUI* rightScore;
            HMUI::ImageView* rightPieImage;

            TMPro::TextMeshProUGUI* rightPreScore;
            TMPro::TextMeshProUGUI* rightAccScore;
            TMPro::TextMeshProUGUI* rightPostScore;

            TMPro::TextMeshProUGUI* tdTitle;
            HMUI::ImageView* tdBackground;
            TMPro::TextMeshProUGUI* preTitle;
            HMUI::ImageView* preBackground;
            TMPro::TextMeshProUGUI* postTitle;
            HMUI::ImageView* postBackground;

            TMPro::TextMeshProUGUI* leftTd;
            TMPro::TextMeshProUGUI* leftPre;
            TMPro::TextMeshProUGUI* leftPost;

            TMPro::TextMeshProUGUI* rightTd;
            TMPro::TextMeshProUGUI* rightPre;
            TMPro::TextMeshProUGUI* rightPost;
            
            void setScore(optional<ScoreStats> score);
            void setSelected(bool selected);
    };
}