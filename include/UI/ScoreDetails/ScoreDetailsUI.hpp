#pragma once

#include "HMUI/ModalView.hpp"

#include "questui/shared/BeatSaberUI.hpp"
#include "questui/shared/QuestUI.hpp"

#include "include/Models/Score.hpp"
#include "include/Models/ScoreStats.hpp"
#include "include/UI/PlayerAvatar.hpp"

#include "include/UI/ScoreDetails/GeneralScoreDetails.hpp"
#include "include/UI/ScoreDetails/ScoreStatsOverview.hpp"
#include "include/UI/ScoreDetails/ScoreStatsGrid.hpp"
#include "include/UI/ScoreDetails/ScoreStatsGraph.hpp"

namespace BeatLeader {
    class ScoreDetailsPopup {
        public:
            HMUI::ModalView* modal;

            BeatLeader::PlayerAvatar* playerAvatar;

            TMPro::TextMeshProUGUI* rank;
            TMPro::TextMeshProUGUI* name;
            TMPro::TextMeshProUGUI* pp;
            TMPro::TextMeshProUGUI* sponsorMessage;

            GeneralScoreDetails general;
            ScoreStatsOverview overview;
            ScoreStatsGrid grid;
            ScoreStatsGraph graph;

            QuestUI::ClickableImage* generalButton;
            QuestUI::ClickableImage* overviewButton;
            QuestUI::ClickableImage* gridButton;
            QuestUI::ClickableImage* graphButton;

            TMPro::TextMeshProUGUI* loadingText;

            int scoreId;
            bool scoreStatsFetched;
            ScoreStats scoreStats;
            
            void setScore(const Score& score);
            void selectTab(int index);

            void setButtonsMaterial();
    };
    void initScoreDetailsPopup(ScoreDetailsPopup** modalUI, UnityEngine::Transform* parent);
}