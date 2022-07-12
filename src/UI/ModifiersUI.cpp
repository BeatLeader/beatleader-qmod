
#include "include/UI/ModifiersUI.hpp"
#include "include/Utils/ModifiersManager.hpp"
#include "include/Utils/StringUtils.hpp"

#include "GlobalNamespace/GameplayModifierToggle.hpp"
#include "GlobalNamespace/GameplayModifierParamsSO.hpp"
#include "GlobalNamespace/GameplayModifiersPanelController.hpp"
#include "GlobalNamespace/GameplayModifiersModelSO.hpp"

#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "beatsaber-hook/shared/utils/logging.hpp"

#include "main.hpp"

#include "TMPro/TMP_Text.hpp"
#include "TMPro/TextMeshProUGUI.hpp"

#include <string>

using namespace GlobalNamespace;
using namespace std;

namespace ModifiersUI {
    static UnityEngine::Color positiveColor = UnityEngine::Color(1.0, 0.0, 0.73, 1.0);
    static UnityEngine::Color negativeColor = UnityEngine::Color(0.65, 0.49, 1.0, 1.0);

    string modifierKeyFromName(string name) {
        if (name == "MODIFIER_PRO_MODE") return "PM";
        if (name == "MODIFIER_NO_BOMBS") return "NB";
        if (name == "MODIFIER_DISAPPEARING_ARROWS") return "DA";
        if (name == "MODIFIER_GHOST_NOTES") return "GN";
        if (name == "MODIFIER_ONE_LIFE") return "OL";
        if (name == "MODIFIER_NO_OBSTACLES") return "NO";
        if (name == "MODIFIER_FASTER_SONG") return "FS";
        if (name == "MODIFIER_SUPER_FAST_SONG") return "SF";
        if (name == "MODIFIER_SMALL_CUBES") return "SC";
        if (name == "MODIFIER_STRICT_ANGLES") return "SA";
        if (name == "MODIFIER_NO_ARROWS") return "NA";
        if (name == "MODIFIER_FOUR_LIVES") return "FL";
        if (name == "MODIFIER_SLOWER_SONG") return "SS";
        if (name == "MODIFIER_ZEN_MODE") return "ZM";
        if (name == "MODIFIER_NO_FAIL_ON_0_ENERGY") return "NF";

        return "UD";
    }

    string_view getRankForMultiplier(float modifier) {
        if (modifier > 0.9) {
            return "SS";
        }
        if (modifier > 0.8) {
            return "S";
        }
        if (modifier > 0.65) {
            return "A";
        }
        if (modifier > 0.5) {
            return "B";
        }
        if (modifier > 0.35) {
            return "C";
        }
        if (modifier > 0.2) {
            return "D";
        }
        return "E";
    }

    MAKE_HOOK_MATCH(RefreshMultipliers, &GameplayModifiersPanelController::RefreshTotalMultiplierAndRankUI, void, GameplayModifiersPanelController* self) {
        auto modifierParams = self->gameplayModifiersModel->CreateModifierParamsList(self->gameplayModifiers);

        float totalMultiplier = 1;

        for (size_t i = 0; i < modifierParams->get_Count(); i++)
        {
            auto param = modifierParams->get_Item(i);

            if (!param->multiplierConditionallyValid) { // for now only NoFail being ignored
                string key = modifierKeyFromName(param->get_modifierNameLocalizationKey());
                if (ModifiersManager::modifiers.count(key)) {
                    totalMultiplier += ModifiersManager::modifiers[key];
                } else {
                    totalMultiplier += param->multiplier;
                }
            }
        }

        if (totalMultiplier < 0) totalMultiplier = 0;  // thanks Beat Games for Zen mode -1000%

        self->totalMultiplierValueText->SetText((totalMultiplier > 1 ? "+" : "") + to_string_wprecision(totalMultiplier * 100.0f, 1) + "%");
        //self->totalMultiplierValueText->set_color(totalMultiplier > 1 ? positiveColor : negativeColor);
        
        self->maxRankValueText->SetText(getRankForMultiplier(totalMultiplier));
        //self->maxRankValueText->set_color(totalMultiplier > 1 ? positiveColor : negativeColor);
    }

    MAKE_HOOK_MATCH(ModifierStart, &GameplayModifierToggle::Start, void, GameplayModifierToggle* self) {
        ModifierStart(self);

        string key = modifierKeyFromName(self->get_gameplayModifier()->get_modifierNameLocalizationKey());
        getLogger().info("%s", key.c_str());
        if (ModifiersManager::modifiers.count(key)) {
            float value = ModifiersManager::modifiers[key];
            self->multiplierText->SetText((value > 0 ? "<color=#00FF77>+" : "<color=#00FFFF>") + to_string_wprecision(value * 100.0f, 1) + "%");
        }
    }

    void setup() {
        LoggerContextObject logger = getLogger().WithContext("load");

        INSTALL_HOOK(logger, ModifierStart);
        INSTALL_HOOK(logger, RefreshMultipliers);
    }
}