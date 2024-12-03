#pragma once

#include "UnityEngine/MonoBehaviour.hpp"
#include "UI/BSML_Addons/Components/FixedImageView.hpp"
#include "custom-types/shared/macros.hpp"

DECLARE_CLASS_CODEGEN(BeatLeader::UI::BSML_Addons, BetterImage, UnityEngine::MonoBehaviour,
    DECLARE_INSTANCE_FIELD(BeatLeader::UI::BSML_Addons::FixedImageView*, image);
) 