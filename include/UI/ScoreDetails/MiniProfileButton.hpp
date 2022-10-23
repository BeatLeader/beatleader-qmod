#pragma once

#include "HMUI/ModalView.hpp"

#include "questui/shared/BeatSaberUI.hpp"
#include "questui/shared/QuestUI.hpp"

#include "include/UI/Components/SmoothHoverController.hpp"

namespace BeatLeader {
    enum struct MiniProfileButtonState {
        NonInteractable = 1,
        InteractableFaded = 2,
        InteractableGlowing = 3,
        Hidden = 4
    };

    class MiniProfileButton {
        public:
            MiniProfileButton() = default;
            MiniProfileButton(std::string hint, UnityEngine::Color glowColor, bool labelOnLeft, QuestUI::ClickableImage* button) noexcept;

            QuestUI::ClickableImage* button;
            HMUI::HoverHint* hint;
            BeatLeader::SmoothHoverController* hoverController;
            MiniProfileButtonState state;
            UnityEngine::Color glowColor;
            bool labelOnLeft;
            
            void setHint(std::string hint) const;
            void setState(MiniProfileButtonState state) const;
            void Callback(bool isHovered, float progress);
            void UpdateLabelOffset();
            void RegisterCallback();
    };
}