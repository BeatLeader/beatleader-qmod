#include "UI/MainMenu/TextNews/TextNewsPostPanel.hpp"

#include "include/Assets/Sprites.hpp"

#include "UnityEngine/Resources.hpp"
#include "UnityEngine/Material.hpp"
#include "UnityEngine/GameObject.hpp"

#include "System/String.hpp"

DEFINE_TYPE(BeatLeader, TextNewsPostPanelComponent);

namespace BeatLeader {
    void TextNewsPostPanelComponent::Awake() {
        header = TextNewsPostHeaderPanel::Instantiate<TextNewsPostHeaderPanel>(get_transform())->LocalComponent();
    }

    void TextNewsPostPanel::SetupData(NewsPost post) {
        LocalComponent()->_bodyText->set_text(post.body);
        LocalComponent()->header->SetupData(post.ownerIcon, post.owner, post.timepost);
        
        bool hasImage = !System::String::IsNullOrEmpty(post.image);
        LocalComponent()->_image->get_gameObject()->SetActive(hasImage);
        
        if (hasImage) {
            Sprites::get_Icon(post.image, [this](UnityEngine::Sprite* sprite) {
                LocalComponent()->_image->set_sprite(sprite);
            });
        }
    }

    void TextNewsPostPanel::OnInitialize() {
        auto materials = UnityEngine::Resources::FindObjectsOfTypeAll<UnityEngine::Material*>();
        for (auto material : materials) {
            if (material->get_name()->Equals("UINoGlow")) {
                LocalComponent()->_image->set_material(UnityEngine::Object::Instantiate(material));
                break;
            }
        }
    }

    StringW TextNewsPostPanel::GetContent() {
        return StringW(R"(
            <vertical spacing="2" pref-width="62" horizontal-fit="PreferredSize">
                <macro.as-host host="header">
                    <macro.reparent transform="_uiComponent"/>
                </macro.as-host>
                <horizontal>
                    <text text="Loading..." id="_bodyText" font-size="3" word-wrapping="true"/>
                </horizontal>
                <img active="false" id="_image" preserve-aspect="true" pref-height="44"/>
            </vertical>
        )");
    }
} 