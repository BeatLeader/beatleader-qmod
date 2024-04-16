#pragma once

#include "HMUI/ModalView.hpp"

#include "bsml/shared/BSML-Lite.hpp"
#include "bsml/shared/BSML.hpp"

#include "include/Models/ScoreStats.hpp"

namespace BeatLeader {
    class AdditionalScoreDetails {
        public:
            AdditionalScoreDetails(HMUI::ModalView* modal) noexcept;

            TMPro::TextMeshProUGUI* details;

            TMPro::TextMeshProUGUI* positionsTitle;
            TMPro::TextMeshProUGUI* positions;
            
            void setScore(string platform, ScoreStats score) const;
            void setSelected(bool selected) const;
    };
}