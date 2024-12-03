#include "UI/MainMenu/TextNews/TextNewsPostHeaderPanel.hpp"

#include "Utils/FormatUtils.hpp"
#include "HMUI/ImageView.hpp"

#include "Assets/BundleLoader.hpp"
#include "Assets/Sprites.hpp"

DEFINE_TYPE(BeatLeader, TextNewsPostHeaderPanelComponent);

namespace BeatLeader {

    void TextNewsPostHeaderPanelComponent::SetupData(StringW avatarUrl, StringW name, int timestamp) {
        _nameText->set_text(name);
        _dateText->set_text(FormatUtils::GetRelativeTimeString(std::to_string(timestamp)));
        
        if (((std::string)avatarUrl).length() == 0) {
            _avatarImage->set_sprite(BundleLoader::bundle->unknownIcon);
        } else {
            Sprites::get_Icon((std::string)avatarUrl, [this](UnityEngine::Sprite* sprite) {
                this->_avatarImage->set_sprite(sprite);
            });
        }
    }

    void TextNewsPostHeaderPanel::OnInitialize() {
        LocalComponent()->_avatarImage->set_material(BundleLoader::bundle->roundTexture10Material);

        LocalComponent()->_avatarImage->_skew = 0.18f;
        LocalComponent()->_avatarImage->__Refresh();

        // LocalComponent()->_image->set_sprite(BundleLoader::bundle->WhiteBG);
    } 

    StringW TextNewsPostHeaderPanel::GetContent() {
        return StringW(R"(
        <better-image id="_image" image-color="#00000080" ppu="10" type="Sliced" pref-height="5" vertical-fit="PreferredSize">
            <horizontal spacing="2" pref-height="5">
                <image anchor-pos-x='5' id="_avatarImage" pref-width="5"/>
                <vertical flexible-width="1000" horizontal-fit="Unconstrained">
                    <text text="Loading News..." id="_nameText" overflow="Ellipsis"/>
                </vertical>
                <vertical anchor-pos-x='-5'>
                    <text text="" id="_dateText" font-size="3" font-color="#888888"/>
                </vertical>
            </horizontal>
        </better-image>
        )");
    }
}