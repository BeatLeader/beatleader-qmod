#pragma once

#include "UI/ReeUIComponentV2.hpp"
#include "UnityEngine/Color.hpp"
#include "UnityEngine/Sprite.hpp"
#include "bsml/shared/BSML-Lite.hpp"
#include "custom-types/shared/delegate.hpp"

DECLARE_CLASS_CUSTOM(BeatLeader, HeaderButtonComponent, BeatLeader::ReeComponent,
    DECLARE_INSTANCE_FIELD(BSML::ClickableImage*, imageComponent);
    DECLARE_INSTANCE_FIELD(float, buttonSize);
    DECLARE_INSTANCE_FIELD(System::Action*, _buttonAction);

    DECLARE_INSTANCE_METHOD(void, Awake);
    DECLARE_INSTANCE_METHOD(void, InitializeImage);
    DECLARE_INSTANCE_METHOD(void, UpdateColor, float hoverProgress);
    DECLARE_INSTANCE_METHOD(void, SetupIcon, UnityEngine::Sprite* sprite);
)

namespace BeatLeader {

class HeaderButton : public ReeUIComponentV2<HeaderButtonComponent*> {
public:
    void OnInitialize() override;
    StringW GetContent() override;
};

} // namespace BeatLeader 