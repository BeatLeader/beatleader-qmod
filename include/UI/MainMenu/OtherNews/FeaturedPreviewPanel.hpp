#pragma once

#include "HMUI/ImageView.hpp"
#include "TMPro/TextMeshProUGUI.hpp"
#include "UnityEngine/UI/Button.hpp"
#include "UI/ReeUIComponentV2.hpp"

DECLARE_CLASS_CUSTOM(BeatLeader, FeaturedPreviewComponent, BeatLeader::ReeComponent) {
    DECLARE_INSTANCE_FIELD(HMUI::ImageView*, _background);
    DECLARE_INSTANCE_FIELD(HMUI::ImageView*, _image);
    DECLARE_INSTANCE_FIELD(TMPro::TextMeshProUGUI*, _topText);
    DECLARE_INSTANCE_FIELD(TMPro::TextMeshProUGUI*, _bottomText);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Button*, _button);
    DECLARE_INSTANCE_FIELD(System::Action*, _buttonAction);

    DECLARE_INSTANCE_METHOD(void, SetupData, StringW previewUrl, StringW topText, StringW bottomText, StringW buttonText, System::Action* buttonAction);
};

namespace BeatLeader {

class FeaturedPreviewPanel : public ReeUIComponentV2<FeaturedPreviewComponent*> {
public:
    void OnInitialize() override;
    StringW GetContent() override;
};

} 