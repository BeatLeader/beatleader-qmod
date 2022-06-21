#pragma once

#include "HMUI/ModalView.hpp"

#include "questui/shared/BeatSaberUI.hpp"
#include "questui/shared/QuestUI.hpp"

#include "include/Models/Score.hpp"
#include "include/UI/PlayerAvatar.hpp"

namespace BeatLeader {
    class GeneralScoreDetails {
        public:
            GeneralScoreDetails(HMUI::ModalView* modal) noexcept;

            TMPro::TextMeshProUGUI* datePlayed;
            
            TMPro::TextMeshProUGUI* modifiedScore;
            TMPro::TextMeshProUGUI* accuracy;
            TMPro::TextMeshProUGUI* scorePp;

            TMPro::TextMeshProUGUI* scoreDetails;
            TMPro::TextMeshProUGUI* sponsorMessage;
            
            void setScore(const Score& score);
            void setSelected(bool selected);
    };
}