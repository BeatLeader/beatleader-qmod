#include "UI/MainMenu/OtherNews/FeaturedPreviewPanel.hpp"
#include "Assets/BundleLoader.hpp"
#include "UnityEngine/Material.hpp"
#include "TMPro/TextOverflowModes.hpp"
#include "HMUI/ImageView.hpp"
#include "UnityEngine/UI/Button.hpp"
#include "System/Action.hpp"
#include "custom-types/shared/delegate.hpp"
#include "include/Assets/Sprites.hpp"

DEFINE_TYPE(BeatLeader, FeaturedPreviewComponent);

namespace BeatLeader {
    void FeaturedPreviewPanel::OnInitialize() {
        auto component = LocalComponent();
        component->_background->_skew = 0.18f;
        component->_background->__Refresh();
        component->_image->set_material(BundleLoader::bundle->roundTexture10Material);
        component->_image->_skew = 0.18f;
        component->_image->__Refresh();

        component->_topText->set_overflowMode(TMPro::TextOverflowModes::Ellipsis);
        component->_bottomText->set_overflowMode(TMPro::TextOverflowModes::Ellipsis);

        // Setup button click handler
        component->_button->get_onClick()->AddListener(custom_types::MakeDelegate<UnityEngine::Events::UnityAction*>(
            (std::function<void()>)[this] { 
                if (LocalComponent()->_buttonAction) {
                    LocalComponent()->_buttonAction->Invoke();
                }
            }
        ));
    }

    void FeaturedPreviewComponent::SetupData(StringW previewUrl, StringW topText, StringW bottomText, StringW buttonText, System::Action* buttonAction) {
        _topText->set_text(" " + topText);
        _bottomText->set_text(bottomText);

        TMPro::TextMeshProUGUI* textMesh = _button->GetComponentInChildren<TMPro::TextMeshProUGUI*>();
        if (textMesh) {
            textMesh->set_text(buttonText);
        }
        _buttonAction = buttonAction;
        
        Sprites::get_Icon(previewUrl, [this](UnityEngine::Sprite* sprite) {
            this->_image->set_sprite(sprite);
        });
    }

    StringW FeaturedPreviewPanel::GetContent() {
        return StringW(R"(
            <horizontal id="_background" pad="1" spacing="1" horizontal-fit="PreferredSize" bg="round-rect-panel" bg-color="#ffffff88">
                <image id="_image" pref-width="10" pref-height="10" horizontal-fit="PreferredSize" vertical-fit="PreferredSize"/>
                <vertical pref-width="38">
                    <text id="_topText" font-size="4" italics="true" overflow-mode="Ellipsis" align="Left" word-wrapping="false"/>
                    <text id="_bottomText" font-size="3" italics="true" overflow-mode="Ellipsis" font-color="#888888" word-wrapping="false"/>
                </vertical>
                <button id="_button" pref-height="8" pref-width="12" text="Details" on-click="OnButtonPressed" pad="0"/>
            </horizontal>
        )");
    }
} 