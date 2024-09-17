#pragma once

#include "HMUI/ModalView.hpp"

#include "bsml/shared/BSML-Lite.hpp"
#include "bsml/shared/BSML.hpp"

#include "shared/Models/Score.hpp"
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
            
            void setScore(const Score& score) const;
            void setSelected(bool selected) const;
    };
}