#pragma once

#include "HMUI/ImageView.hpp"
#include "TMPro/TextMeshProUGUI.hpp"
#include "UI/ReeUIComponentV2.hpp"
#include "Models/NewsPost.hpp"
#include "UI/MainMenu/TextNews/TextNewsPostHeaderPanel.hpp"

DECLARE_CLASS_CUSTOM(BeatLeader, TextNewsPostPanelComponent, BeatLeader::ReeComponent) {
    DECLARE_INSTANCE_FIELD(TMPro::TextMeshProUGUI*, _bodyText);
    DECLARE_INSTANCE_FIELD(HMUI::ImageView*, _image);
    DECLARE_INSTANCE_FIELD(TextNewsPostHeaderPanelComponent*, header);

    DECLARE_INSTANCE_METHOD(void, Awake);
};

namespace BeatLeader {

class TextNewsPostPanel : public ReeUIComponentV2<TextNewsPostPanelComponent*> {
public:
    void OnInitialize() override;
    StringW GetContent() override;

    // Public methods
    void SetupData(NewsPost post);
};

}  // namespace BeatLeader 