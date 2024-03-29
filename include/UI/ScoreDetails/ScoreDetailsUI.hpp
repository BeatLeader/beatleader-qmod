#pragma once

#include "HMUI/ModalView.hpp"

#include "bsml/shared/BSML-Lite.hpp"
#include "bsml/shared/bsml.hpp"

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

            BSML::ClickableImage* generalButton;
            BSML::ClickableImage* additionalButton;
            BSML::ClickableImage* overviewButton;
            BSML::ClickableImage* gridButton;
            BSML::ClickableImage* graphButton;

            BSML::ClickableImage* replayButton;

            TMPro::TextMeshProUGUI* loadingText;

            bool scoreStatsFetched;
            
            void setScore(const Score& score);
            void updatePlayerDetails(Player player);
            void selectTab(int index);

            void playReplay();

            void setButtonsMaterial() const;
    };
    void initScoreDetailsPopup(ScoreDetailsPopup** modalUI, UnityEngine::Transform* parent, function<void()> const &incognitoCallback);
}