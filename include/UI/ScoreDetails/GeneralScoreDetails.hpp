#pragma once

#include "HMUI/ModalView.hpp"

#include "bsml/shared/BSML-Lite.hpp"
#include "bsml/shared/BSML.hpp"

#include "shared/Models/Score.hpp"
#include "include/UI/PlayerAvatar.hpp"

#include <memory>
#include <string>

namespace BeatLeader {
    struct HoverableText {
        TMPro::TextMeshProUGUI* text = nullptr;
        std::string normalText;
        std::string hoverText;
        bool hoverEnabled = false;
        bool hovered = false;

        void Refresh() const;
    };

    class GeneralScoreDetails {
        public:
            GeneralScoreDetails(HMUI::ModalView* modal) noexcept;

            TMPro::TextMeshProUGUI* datePlayed;

            TMPro::TextMeshProUGUI* modifiedScore;
            TMPro::TextMeshProUGUI* accuracy;
            TMPro::TextMeshProUGUI* scorePp;

            TMPro::TextMeshProUGUI* scoreDetails;

            std::shared_ptr<HoverableText> accuracyHover;
            std::shared_ptr<HoverableText> ppHover;

            void setScore(const Score& score) const;
            void setSelected(bool selected) const;
    };
}