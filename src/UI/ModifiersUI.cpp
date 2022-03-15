
#include "include/UI/ModifiersUI.hpp"
#include "include/Utils/ModifiersManager.hpp"
#include "include/Utils/StringUtils.hpp"

#include "GlobalNamespace/GameplayModifierToggle.hpp"
#include "GlobalNamespace/GameplayModifierParamsSO.hpp"

#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "beatsaber-hook/shared/utils/logging.hpp"

#include "main.hpp"

#include "TMPro/TMP_Text.hpp"
#include "TMPro/TextMeshProUGUI.hpp"

#include <string>

using namespace GlobalNamespace;
using namespace std;

string ModifierKeyFromName(string name) {
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

MAKE_HOOK_MATCH(ModifierStart, &GameplayModifierToggle::Start, void, GameplayModifierToggle* self) {
    ModifierStart(self);

    string key = ModifierKeyFromName(self->get_gameplayModifier()->get_modifierNameLocalizationKey());
    getLogger().info("%s", key.c_str());
    if (ModifiersManager::modifiers.count(key)) {
        float value = ModifiersManager::modifiers[key];
        self->multiplierText->SetText((value > 0 ? "+" : "") + to_string_wprecision(value * 100.0f, 1) + "%");
    }
}

void SetupModifiersUI() {
    LoggerContextObject logger = getLogger().WithContext("load");

    INSTALL_HOOK(logger, ModifierStart);
}