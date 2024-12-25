#include "UI/Components/HeaderButton.hpp"
#include "Assets/BundleLoader.hpp"
#include "UI/Components/SmoothHoverController.hpp"
#include "bsml/shared/BSML/Components/ClickableImage.hpp"
#include "bsml/shared/BSML/GameplaySetup/GameplaySetup.hpp"

DEFINE_TYPE(BeatLeader, HeaderButtonComponent);

namespace BeatLeader {

    // static UnityEngine::Color SelectedColor = UnityEngine::Color(0.0f, 0.4f, 1.0f, 1.0f);
    // static UnityEngine::Color FadedColor = UnityEngine::Color(0.8f, 0.8f, 0.8f, 0.2f);

    void HeaderButtonComponent::Awake() {
        buttonSize = 4.0f;
    }

    void HeaderButtonComponent::InitializeImage() {
        imageComponent->set_material(BundleLoader::bundle->UIAdditiveGlowMaterial);
        imageComponent->onClick += [this]() {
            if (_buttonAction) {
                _buttonAction->Invoke();
            }
            // BSML::GameplaySetup::BasicUIAudioManager->HandleButtonClickEvent();
        };

        // SmoothHoverController::Scale(imageComponent->get_gameObject(), 1.0f, 1.2f);
    }

    void HeaderButtonComponent::UpdateColor(float hoverProgress) {
        // auto color = UnityEngine::Color::Lerp(FadedColor, SelectedColor, hoverProgress);
        // imageComponent->set_defaultColor(color);
        // imageComponent->set_highlightColor(color);
    }

    void HeaderButtonComponent::SetupIcon(UnityEngine::Sprite* sprite) {
        imageComponent->set_sprite(sprite);
    }

    void HeaderButton::OnInitialize() {
        LocalComponent()->InitializeImage();
        // LocalComponent()->UpdateColor(0.0f);
    }

    StringW HeaderButton::GetContent() {
        return StringW(R"(
            <horizontal id="root" pref-height="~buttonSize" horizontal-fit="PreferredSize">
                <vertical pref-width="~buttonSize">
                    <clickable-image id="imageComponent" preserve-aspect="true"/>
                </vertical>
            </horizontal>
        )");
    }

} // namespace BeatLeader 