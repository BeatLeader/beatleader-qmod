#pragma once

#include "HMUI/ModalView.hpp"

#include "bsml/shared/BSML-Lite.hpp"
#include "bsml/shared/BSML.hpp"

namespace BeatLeader {
    class LinksContainerPopup {
        public:
            HMUI::ModalView* modal;

            TMPro::TextMeshProUGUI* versionText;

            BSML::ClickableImage* profile;
            BSML::ClickableImage* discord;
            BSML::ClickableImage* patreon;

            UnityEngine::UI::Button* nominated;
            UnityEngine::UI::Button* qualified;
            UnityEngine::UI::Button* ranked;
    };
    void initLinksContainerPopup(LinksContainerPopup** modalUI, UnityEngine::Transform* parent);
    void SetButtonSize(UnityEngine::UI::Button* button, UnityEngine::Vector2 sizeDelta);
}