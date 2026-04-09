#include "include/UI/HeaderButton.hpp"

#include "include/Assets/BundleLoader.hpp"
#include "include/UI/Components/SmoothHoverController.hpp"

DEFINE_TYPE(BeatLeader, HeaderButtonComponent);

namespace {
    UnityEngine::Color SelectedColor = UnityEngine::Color(0.0f, 0.4f, 1.0f, 1.0f);
    UnityEngine::Color FadedColor = UnityEngine::Color(0.8f, 0.8f, 0.8f, 0.2f);
}

namespace BeatLeader {

    void HeaderButtonComponent::OnClick() {
        auto* button = reinterpret_cast<HeaderButton*>(nativeComponent);
        if (button) {
            button->InvokeClick();
        }
    }

    void HeaderButton::OnInitialize() {
        LocalComponent()->_imageComponent->set_material(BundleLoader::bundle->UIAdditiveGlowMaterial);
        LocalComponent()->_imageComponent->set_defaultColor(FadedColor);
        LocalComponent()->_imageComponent->set_highlightColor(SelectedColor);

        SmoothHoverController::Scale(LocalComponent()->_imageComponent->get_gameObject(), 1.0f, 1.2f);
    }

    StringW HeaderButton::GetContent() {
        return StringW(R"(
            <horizontal pref-height="4" horizontal-fit="PreferredSize">
                <vertical pref-width="4">
                    <clickable-image id="_imageComponent" src="#PlayerIcon" preserve-aspect="true" on-click="OnClick"/>
                </vertical>
            </horizontal>
        )");
    }

    void HeaderButton::Setup(UnityEngine::Sprite* sprite) {
        if (LocalComponent() && LocalComponent()->_imageComponent) {
            LocalComponent()->_imageComponent->set_sprite(sprite);
        }
    }

    void HeaderButton::SetOnClick(std::function<void()> callback) {
        _onClick = std::move(callback);
    }

    void HeaderButton::InvokeClick() {
        if (_onClick) {
            _onClick();
        }
    }
}
