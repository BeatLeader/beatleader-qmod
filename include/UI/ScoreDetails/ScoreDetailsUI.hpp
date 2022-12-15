#pragma once

#include "HMUI/ModalView.hpp"

#include "questui/shared/BeatSaberUI.hpp"
#include "questui/shared/QuestUI.hpp"

#include "include/Models/Score.hpp"
#include "include/UI/PlayerAvatar.hpp"

#include "include/UI/ScoreDetails/GeneralScoreDetails.hpp"
#include "include/UI/ScoreDetails/AdditionalScoreDetails.hpp"
#include "include/UI/ScoreDetails/ScoreStatsOverview.hpp"
#include "include/UI/ScoreDetails/ScoreStatsGrid.hpp"
#include "include/UI/ScoreDetails/ScoreStatsGraph.hpp"
#include "include/UI/ScoreDetails/PlayerButtons.hpp"

namespace BeatLeader {
    class ScoreDetailsPopup {
        public:
            HMUI::ModalView* modal;

            BeatLeader::PlayerAvatar* playerAvatar;

            TMPro::TextMeshProUGUI* rank;
            TMPro::TextMeshProUGUI* name;
            TMPro::TextMeshProUGUI* pp;
            TMPro::TextMeshProUGUI* sponsorMessage;

            PlayerButtons playerButtons;
            GeneralScoreDetails general;
            AdditionalScoreDetails additional;
            ScoreStatsOverview overview;
            ScoreStatsGrid grid;
            ScoreStatsGraph graph;

            QuestUI::ClickableImage* generalButton;
            QuestUI::ClickableImage* additionalButton;
            QuestUI::ClickableImage* overviewButton;
            QuestUI::ClickableImage* gridButton;
            QuestUI::ClickableImage* graphButton;

            QuestUI::ClickableImage* replayButton;

            TMPro::TextMeshProUGUI* loadingText;

            int scoreId;
            string platform;
            bool scoreStatsFetched;
            
            void setScore(const Score& score);
            void updatePlayerDetails(Player player);
            void selectTab(int index);

            void playReplay();

            void setButtonsMaterial() const;
    };
    void initScoreDetailsPopup(ScoreDetailsPopup** modalUI, UnityEngine::Transform* parent, function<void()> const &incognitoCallback);
}