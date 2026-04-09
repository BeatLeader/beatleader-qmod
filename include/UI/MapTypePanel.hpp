#pragma once

#include "UI/ReeUIComponentV2.hpp"
#include "shared/Models/MapsTypeDescription.hpp"

#include "HMUI/HoverHint.hpp"
#include "HMUI/ImageView.hpp"

#include <string>

DECLARE_CLASS_CUSTOM(BeatLeader, MapTypePanelComponent, BeatLeader::ReeComponent) {
    DECLARE_INSTANCE_FIELD(HMUI::ImageView*, background);
    DECLARE_INSTANCE_FIELD(HMUI::ImageView*, icon);
    DECLARE_INSTANCE_FIELD(HMUI::HoverHint*, _hoverHint);
};

namespace BeatLeader {

class MapTypePanel : public ReeUIComponentV2<MapTypePanelComponent*> {
public:
    void OnInitialize() override;
    StringW GetContent() override;

    void SetActive(bool value);
    void SetValues(MapsTypeDescription const& mapType);

private:
    void ApplyBackgroundColor();

    std::string _iconUrl;
};

}
