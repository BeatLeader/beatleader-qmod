
#include "include/UI/ModifiersUI.hpp"
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
#include <unordered_map>
#include <vector>

using namespace GlobalNamespace;
using namespace std;

namespace ModifiersUI {
    static UnityEngine::Color positiveColor = UnityEngine::Color(1.0, 0.0, 0.73, 1.0);
    static UnityEngine::Color negativeColor = UnityEngine::Color(0.65, 0.49, 1.0, 1.0);
    SafePtrUnity<GameplayModifiersPanelController> modifiersPanel;
    unordered_map<string, GameplayModifierToggle*> allModifierToggles;
    unordered_map<string, float> songModifiers;

    static unordered_map<string, string> modifierKeyFromName = {
        {"MODIFIER_PRO_MODE", "PM"},
        {"MODIFIER_NO_BOMBS", "NB"},
        {"MODIFIER_DISAPPEARING_ARROWS", "DA"},
        {"MODIFIER_GHOST_NOTES", "GN"},
        {"MODIFIER_ONE_LIFE", "OL"},
        {"MODIFIER_NO_OBSTACLES", "NO"},
        {"MODIFIER_FASTER_SONG", "FS"},
        {"MODIFIER_SUPER_FAST_SONG", "SF"},
        {"MODIFIER_SMALL_CUBES", "SC"},
        {"MODIFIER_STRICT_ANGLES", "SA"},
        {"MODIFIER_NO_ARROWS", "NA"},
        {"MODIFIER_FOUR_LIVES", "FL"},
        {"MODIFIER_SLOWER_SONG", "SS"},
        {"MODIFIER_ZEN_MODE", "ZM"},
        {"MODIFIER_NO_FAIL_ON_0_ENERGY", "NF"},
    };

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
        RefreshMultipliers(self);

        modifiersPanel = self;
        refreshMultiplierAndMaxRank();
    }

    MAKE_HOOK_MATCH(ModifierStart, &GameplayModifierToggle::Start, void, GameplayModifierToggle* self) {
        ModifierStart(self);

        string key = modifierKeyFromName[self->get_gameplayModifier()->get_modifierNameLocalizationKey()];
        getLogger().info("%s", key.c_str());
        allModifierToggles[key] = self;
    }

    void refreshAllModifiers(){
        // Set the map dependant modifier values on the toggles (with colors)
        for(auto& [key, value] : allModifierToggles){
            if(songModifiers.contains(key)){
                float modifierValue = songModifiers[key];
                value->multiplierText->SetText((modifierValue > 0 ? "<color=#00FF77>+" : "<color=#00FFFF>") + to_string_wprecision(modifierValue * 100.0f, 1) + "%");
            }
        }
        refreshMultiplierAndMaxRank();
    }

    void refreshMultiplierAndMaxRank()
    {
        // If we dont have a panel reference we cant do anything
        if (modifiersPanel) {

            // Now we iterate all modifiers to set the totalMultiplier (% value on top) and the max achievable rank
            auto modifierParams = modifiersPanel->gameplayModifiersModel->CreateModifierParamsList(modifiersPanel->gameplayModifiers);

            float totalMultiplier = 1;

            for (size_t i = 0; i < modifierParams->get_Count(); i++)
            {
                auto param = modifierParams->get_Item(i);

                if (!param->multiplierConditionallyValid) { // for now only NoFail being ignored
                    string key;
                    if (!songModifiers.empty() && modifierKeyFromName.contains(param->get_modifierNameLocalizationKey()) && songModifiers.contains(key = modifierKeyFromName[param->get_modifierNameLocalizationKey()])) {
                        totalMultiplier += songModifiers[key];
                    }
                    else {
                        totalMultiplier += param->multiplier;
                    }
                }
            }

            if (totalMultiplier < 0) totalMultiplier = 0; // thanks Beat Games for Zen mode -1000%

            modifiersPanel->totalMultiplierValueText->SetText((totalMultiplier > 1 ? "+" : "") + to_string_wprecision(totalMultiplier * 100.0f, 1) + "%");
            //modifiersPanel->totalMultiplierValueText->set_color(totalMultiplier > 1 ? positiveColor : negativeColor);

            modifiersPanel->maxRankValueText->SetText(getRankForMultiplier(totalMultiplier));
            // self->maxRankValueText->set_color(totalMultiplier > 1 ? positiveColor : negativeColor);
        }
    }

    void setup() {
        LoggerContextObject logger = getLogger().WithContext("load");

        INSTALL_HOOK(logger, ModifierStart);
        INSTALL_HOOK(logger, RefreshMultipliers);
    }
}