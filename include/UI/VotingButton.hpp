#pragma once

#include "UnityEngine/MonoBehaviour.hpp"
#include "UnityEngine/Material.hpp"

#include "custom-types/shared/macros.hpp"

#include "bsml/shared/BSML/Components/ClickableImage.hpp"
#include "HMUI/HoverHint.hpp"

#define GET_FIND_METHOD(mPtr) ::i2c::metadata_getter<mPtr>::method_info()

DECLARE_CLASS_CODEGEN(BeatLeader, VotingButton, UnityEngine::MonoBehaviour) {
    DECLARE_INSTANCE_FIELD(BSML::ClickableImage*, imageView);
    DECLARE_INSTANCE_FIELD(UnityEngine::Material*, materialInstance);
    DECLARE_INSTANCE_FIELD(HMUI::HoverHint*, hoverHint);
    DECLARE_INSTANCE_FIELD(int, state);

    DECLARE_INSTANCE_METHOD(void, Init, BSML::ClickableImage* imageView);
    DECLARE_INSTANCE_METHOD(void, SetState, int state);
};