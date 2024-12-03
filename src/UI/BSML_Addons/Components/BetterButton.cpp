#include "UI/BSML_Addons/Components/BetterButton.hpp"
#include "UnityEngine/UI/Navigation.hpp"

DEFINE_TYPE(BeatLeader::UI::BSML_Addons, BetterButton);

namespace BeatLeader::UI::BSML_Addons {
    void BetterButton::ctor() {
        INVOKE_CTOR();
    }

    void BetterButton::Init(UnityEngine::UI::Button* btn, UnityEngine::UI::Image* graphic) {
        button = btn;
        targetGraphic = graphic;
        button->set_targetGraphic(graphic);
        
        UnityEngine::UI::Navigation navigation;
        navigation.mode = UnityEngine::UI::Navigation::Mode::None;
        button->set_navigation(navigation);
    }
} 