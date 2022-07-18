#pragma once

#include "HMUI/ModalView.hpp"

#include "questui/shared/BeatSaberUI.hpp"
#include "questui/shared/QuestUI.hpp"

namespace BeatLeader {
    class LinksContainerPopup {
        public:
            HMUI::ModalView* modal;

            TMPro::TextMeshProUGUI* versionText;

            QuestUI::ClickableImage* profile;
            QuestUI::ClickableImage* discord;
            QuestUI::ClickableImage* patreon;
    };
    void initLinksContainerPopup(LinksContainerPopup** modalUI, UnityEngine::Transform* parent);
}