#pragma once

#include "UnityEngine/UI/Button.hpp"
#include "UnityEngine/UI/Image.hpp"
#include "UnityEngine/MonoBehaviour.hpp"
#include "custom-types/shared/macros.hpp"
#include "UI/BSML_Addons/Tags/BetterButtonTag.hpp"

namespace BeatLeader::UI::BSML_Addons {
    class BetterButtonTag;
}

DECLARE_CLASS_CODEGEN(BeatLeader::UI::BSML_Addons, BetterButton, UnityEngine::MonoBehaviour,
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Button*, button);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Image*, targetGraphic);
    DECLARE_CTOR(ctor);

    DECLARE_INSTANCE_METHOD(void, Init, UnityEngine::UI::Button* btn, UnityEngine::UI::Image* graphic);
    
    private:
        friend class ::BeatLeader::UI::BSML_Addons::BetterButtonTag;
) 