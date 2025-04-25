#pragma once

#include "UI/ReeUIComponentV2.hpp"
#include "HMUI/ImageView.hpp"
#include "TMPro/TextMeshProUGUI.hpp"
#include <string_view>

DECLARE_CLASS_CUSTOM(BeatLeader, TextNewsPostHeaderPanelComponent, BeatLeader::ReeComponent) {
    DECLARE_INSTANCE_FIELD(TMPro::TextMeshProUGUI*, _nameText);
    DECLARE_INSTANCE_FIELD(TMPro::TextMeshProUGUI*, _dateText);
    DECLARE_INSTANCE_FIELD(HMUI::ImageView*, _avatarImage);
    DECLARE_INSTANCE_FIELD(HMUI::ImageView*, _image);

    DECLARE_INSTANCE_METHOD(void, SetupData, StringW avatarUrl, StringW name, int timestamp);
};

namespace BeatLeader {

class TextNewsPostHeaderPanel : public ReeUIComponentV2<TextNewsPostHeaderPanelComponent*> {
public:
    void OnInitialize() override;
    StringW GetContent() override;
};

} 