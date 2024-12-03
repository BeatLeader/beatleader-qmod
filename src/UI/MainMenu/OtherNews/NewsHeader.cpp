#include "UI/MainMenu/OtherNews/NewsHeader.hpp"
#include "UnityEngine/Vector2.hpp"

DEFINE_TYPE(BeatLeader, NewsHeaderComponent);

namespace BeatLeader {
    void NewsHeaderComponent::SetupData(StringW text) {
        _text->set_text(text);
    }

    void NewsHeader::OnInitialize() {
        LocalComponent()->_text->get_rectTransform()->set_anchorMin(UnityEngine::Vector2::get_zero());
        LocalComponent()->_text->get_rectTransform()->set_anchorMax(UnityEngine::Vector2::get_one());
        LocalComponent()->_text->get_rectTransform()->set_sizeDelta(UnityEngine::Vector2::get_zero());
        LocalComponent()->_image->_skew = 0.18f;
        LocalComponent()->_image->__Refresh();
    }

    StringW NewsHeader::GetContent() {
        return StringW(R"(
        <better-image id="_image" image-color="#00000080" ppu="10" type="Sliced" pref-height="5" vertical-fit="PreferredSize">
            <text id="_text" font-size="4" italics="true" align="Center"/>
        </better-image>
        )");
    }
} 