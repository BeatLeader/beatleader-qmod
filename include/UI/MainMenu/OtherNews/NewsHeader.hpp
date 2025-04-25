#pragma once

#include "UnityEngine/RectTransform.hpp"
#include "UnityEngine/Vector2.hpp"
#include "HMUI/ImageView.hpp"
#include "TMPro/TextMeshProUGUI.hpp"
#include "UI/ReeUIComponentV2.hpp"

DECLARE_CLASS_CUSTOM(BeatLeader, NewsHeaderComponent, BeatLeader::ReeComponent) {
    DECLARE_INSTANCE_FIELD(HMUI::ImageView*, _image);
    DECLARE_INSTANCE_FIELD(TMPro::TextMeshProUGUI*, _text);

    DECLARE_INSTANCE_METHOD(void, SetupData, StringW text);
};

namespace BeatLeader {

class NewsHeader : public ReeUIComponentV2<NewsHeaderComponent*> {
public:
    void OnInitialize() override;
    StringW GetContent() override;
};

}
