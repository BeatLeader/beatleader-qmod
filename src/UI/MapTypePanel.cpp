#include "include/UI/MapTypePanel.hpp"

#include "include/Assets/BundleLoader.hpp"
#include "include/Assets/Sprites.hpp"

#include "bsml/shared/BSML-Lite.hpp"

DEFINE_TYPE(BeatLeader, MapTypePanelComponent);

namespace {
    constexpr float HeaderBackgroundAlpha = 221.0f / 255.0f;
}

namespace BeatLeader {

    void MapTypePanel::OnInitialize() {
        LocalComponent()->background->set_raycastTarget(true);
        ApplyBackgroundColor();
        LocalComponent()->_hoverHint = BSML::Lite::AddHoverHint(LocalComponent()->background, "");
        SetActive(false);
    }

    StringW MapTypePanel::GetContent() {
        return StringW(R"(
            <horizontal id="background" pref-height="6" pad-left="1" pad-right="1" horizontal-fit="PreferredSize" bg="round-rect-panel">
                <image id="icon" src="#PlayerIcon" pref-width="3.0" pref-height="3.0" preserve-aspect="true"/>
            </horizontal>
        )");
    }

    void MapTypePanel::SetActive(bool value) {
        LocalComponent()->SetRootActive(value);
    }

    void MapTypePanel::SetValues(MapsTypeDescription const& mapType) {
        if (LocalComponent()->_hoverHint) {
            LocalComponent()->_hoverHint->set_text(mapType.name + " | " + mapType.description);
        }

        if (!LocalComponent()->icon) {
            return;
        }

        if (_iconUrl == mapType.icon && LocalComponent()->icon->get_sprite()) {
            return;
        }

        _iconUrl = mapType.icon;
        LocalComponent()->icon->set_sprite(BundleLoader::bundle->unknownIcon);

        if (_iconUrl.empty()) {
            return;
        }

        auto currentIconUrl = _iconUrl;
        Sprites::get_Icon(currentIconUrl, [this, currentIconUrl](UnityEngine::Sprite* sprite) {
            if (_iconUrl != currentIconUrl || !LocalComponent() || !LocalComponent()->icon) {
                return;
            }

            LocalComponent()->icon->set_sprite(sprite ? sprite : BundleLoader::bundle->unknownIcon);
        }, true);
    }

    void MapTypePanel::ApplyBackgroundColor() {
        auto color = UnityEngine::Color(1.0f, 1.0f, 1.0f, HeaderBackgroundAlpha);
        LocalComponent()->background->set_color(color);
        LocalComponent()->background->set_color0(color);
        LocalComponent()->background->set_color1(color);
    }
}
