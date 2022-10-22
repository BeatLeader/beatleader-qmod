#include "include/UI/ScoreDetails/MiniProfileButton.hpp"

#include "include/Assets/Sprites.hpp"
#include "include/Assets/BundleLoader.hpp"

#include "include/UI/EmojiSupport.hpp"
#include "include/API/PlayerController.hpp"

#include "include/Utils/FormatUtils.hpp"
#include "include/Utils/Range.hpp"

#include "UnityEngine/Resources.hpp"
#include "UnityEngine/Component.hpp"

#include "HMUI/ImageView.hpp"
#include "HMUI/CurvedCanvasSettingsHelper.hpp"
#include "HMUI/CurvedCanvasSettings.hpp"

#include "main.hpp"

#include <sstream>

using namespace QuestUI::BeatSaberUI;
using namespace UnityEngine;
using namespace UnityEngine::UI;
using namespace GlobalNamespace;

static UnityEngine::Color InactiveColor = UnityEngine::Color(0.4f, 0.4f, 0.4f, 0.2f);
static UnityEngine::Color FadedColor = UnityEngine::Color(0.8f, 0.8f, 0.8f, 0.2f);

BeatLeader::MiniProfileButton::MiniProfileButton(
        string hint, 
        UnityEngine::Color glowColor, 
        bool labelOnLeft,
        QuestUI::ClickableImage* button) noexcept {
    this->button = button;
    this->glowColor = glowColor;
    this->labelOnLeft = labelOnLeft;
    this->hint = ::QuestUI::BeatSaberUI::AddHoverHint(button, hint);
    // this->hint = CreateText(button->get_transform(), hint, UnityEngine::Vector2(0.0, 0.0));
    // UpdateLabelOffset();
}

void BeatLeader::MiniProfileButton::setHint(string hint) const {
    this->hint->set_text(hint);
}

void BeatLeader::MiniProfileButton::UpdateLabelOffset() {
    // float offset = 4.0f + hint->get_preferredWidth() / 2;
    // hint->get_transform()->set_localPosition(Vector3(labelOnLeft ? -offset : offset, 0, 0));
}

void BeatLeader::MiniProfileButton::setState(MiniProfileButtonState state) const {
    button->get_gameObject()->SetActive(true);

    switch (state) {
        case MiniProfileButtonState::NonInteractable:
            button->set_defaultColor(InactiveColor);
            button->set_highlightColor(InactiveColor);
            break;
        case MiniProfileButtonState::InteractableFaded: 
            button->set_defaultColor(FadedColor);
            button->set_highlightColor(FadedColor);
            break;
        case MiniProfileButtonState::InteractableGlowing:
            button->set_defaultColor(this->glowColor);
            button->set_highlightColor(this->glowColor);
            break;
        case MiniProfileButtonState::Hidden:
            button->get_gameObject()->SetActive(false);
            break;
    }
}

void BeatLeader::MiniProfileButton::RegisterCallback(){
    // auto self = this;
    // hoverController = button->get_gameObject()->AddComponent<BeatLeader::SmoothHoverController*>();
    // hoverController->get_hoverStateChangedEvent() += [self](bool isHovered, float progress){ 
    //     self->Callback(isHovered, progress);
    // };
}

void BeatLeader::MiniProfileButton::Callback(bool isHovered, float progress) {
    // float scale = state == MiniProfileButtonState::NonInteractable ? 0.8f : 1.0f + 0.5f * progress;
    // button->get_transform()->set_localScale(Vector3(scale, scale, scale));

    // float maxAlpha = state == MiniProfileButtonState::NonInteractable ? 0.5f : 1.0f;
    // hint->set_alpha(maxAlpha * progress);
    // hint->get_transform()->set_localScale(Vector3(1.0f, progress, 1.0f));
}