#pragma once

#include "UI/ReeUIComponentV2.hpp"
#include "UnityEngine/Sprite.hpp"
#include "bsml/shared/BSML/Components/ClickableImage.hpp"

#include <functional>
#include <utility>

DECLARE_CLASS_CUSTOM(BeatLeader, HeaderButtonComponent, BeatLeader::ReeComponent) {
    DECLARE_INSTANCE_FIELD(BSML::ClickableImage*, _imageComponent);

    DECLARE_INSTANCE_METHOD(void, OnClick);
};

namespace BeatLeader {

class HeaderButton : public ReeUIComponentV2<HeaderButtonComponent*> {
public:
    void OnInitialize() override;
    StringW GetContent() override;

    void Setup(UnityEngine::Sprite* sprite);
    void SetOnClick(std::function<void()> callback);
    void InvokeClick();

private:
    std::function<void()> _onClick;
};

}
