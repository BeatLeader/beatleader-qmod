#include "include/UI/LevelInfoUI.hpp"
#include "include/UI/ModifiersUI.hpp"
#include "include/UI/UIUtils.hpp"
#include "include/Utils/StringUtils.hpp"
#include "include/Models/TriangleRating.hpp"

#include "GlobalNamespace/GameplayModifierToggle.hpp"
#include "GlobalNamespace/GameplayModifierParamsSO.hpp"
#include "GlobalNamespace/GameplayModifiersPanelController.hpp"
#include "GlobalNamespace/GameplayModifiersModelSO.hpp"

#include "GlobalNamespace/GameServerLobbyFlowCoordinator.hpp"

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
    unordered_map<string, TriangleRating> songModifierRatings;
    bool ssActive = false;
    bool multiActive = false;

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

    static vector<GameplayModifierToggle*> modifiers;

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

    MAKE_HOOK_MATCH(ActivateMultiplayer, &GameServerLobbyFlowCoordinator::DidActivate, void, GameServerLobbyFlowCoordinator* self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling){
        ActivateMultiplayer(self, firstActivation, addedToHierarchy, screenSystemEnabling);

        multiActive = true;
    }

    MAKE_HOOK_MATCH(DeActivateMultiplayer, &GameServerLobbyFlowCoordinator::DidDeactivate, void, GameServerLobbyFlowCoordinator* self, bool removedFromHierarchy, bool screenSystemDisabling){
        DeActivateMultiplayer(self, removedFromHierarchy, screenSystemDisabling);

        multiActive = false;
    }

    MAKE_HOOK_MATCH(RefreshMultipliers, &GameplayModifiersPanelController::RefreshTotalMultiplierAndRankUI, void, GameplayModifiersPanelController* self) {
        RefreshMultipliers(self);

        modifiersPanel = self;
        // Refresh Rating labels as a rating modifier could have been selected (this also calls refreshAllModifiers)
        if(!ssActive && !multiActive)
            LevelInfoUI::refreshRatingLabels();
    }

    MAKE_HOOK_MATCH(ModifierStart, &GameplayModifierToggle::Start, void, GameplayModifierToggle* self) {
        ModifierStart(self);

        string key = modifierKeyFromName[self->get_gameplayModifier()->get_modifierNameLocalizationKey()];
        BeatLeaderLogger.info("%s", key.c_str());
        if(!multiActive)
            allModifierToggles[key] = self;
    }

    TriangleRating refreshAllModifiers(){
        // Set the map dependant modifier values on the toggles (with colors)
        for(auto& [key, value] : allModifierToggles){
            string modifierSubText;
            if (songModifierRatings.contains(key)) {
                modifierSubText = (key != "SS" ? "<color=#00FF77>" : "<color=#00FFFF>") + (string)"New stars " + to_string_wprecision(UIUtils::getStarsToShow(songModifierRatings[key]), 2);
            }
            else if(songModifiers.contains(key)) {
                float modifierValue = songModifiers[key];
                modifierSubText = (modifierValue > 0 ? "<color=#00FF77>+" : "<color=#00FFFF>") + to_string_wprecision(modifierValue * 100.0f, 1) + "%";
            }
            else {
                float modifierValue = value->gameplayModifier->multiplier;
                modifierSubText = (modifierValue > 0 ? "+" : "") + to_string_wprecision(modifierValue * 100.0f, 1) + "%";
            }

            // Set the text underneath the modifier
            value->multiplierText->SetText(modifierSubText);
        }
        return refreshMultiplierAndMaxRank();
    }

    TriangleRating refreshMultiplierAndMaxRank()
    {
        TriangleRating ratingSelected;
        // If we dont have a panel reference we cant do anything
        if (modifiersPanel) {

            // Now we iterate all modifiers to set the totalMultiplier (% value on top) and the max achievable rank
            auto modifierParams = modifiersPanel->gameplayModifiersModel->CreateModifierParamsList(modifiersPanel->gameplayModifiers);

            float totalMultiplier = 1;

            for (size_t i = 0; i < modifierParams->get_Count(); i++)
            {
                auto param = modifierParams->get_Item(i);
                
                // If parameter is not nofail, we have a received any modifiers from the server and we have a short form of this modifier
                if (!param->multiplierConditionallyValid && !songModifiers.empty() && modifierKeyFromName.contains(param->get_modifierNameLocalizationKey())) {
                    string key = modifierKeyFromName[param->get_modifierNameLocalizationKey()];
                    if (songModifierRatings.contains(key)) {
                        // ModifierRatings apply to star value and have no effect on max rank.
                        // But we need to return it so that it can be shown in the respective labels
                        ratingSelected = songModifierRatings[key];
                    }
                    else if (songModifiers.contains(key)) {
                        totalMultiplier += songModifiers[key];
                    }
                    else {
                        totalMultiplier += param->multiplier;
                    }
                }
            }

            if (totalMultiplier < 0) totalMultiplier = 0; // thanks Beat Games for Zen mode -1000%

            // Correct texts & color of total multiplier & rank with our values
            modifiersPanel->totalMultiplierValueText->SetText((totalMultiplier > 1 ? "+" : "") + to_string_wprecision(totalMultiplier * 100.0f, 1) + "%");
            modifiersPanel->maxRankValueText->SetText(getRankForMultiplier(totalMultiplier));

            auto color = totalMultiplier >= 1 ? modifiersPanel->positiveColor : modifiersPanel->negativeColor;
            modifiersPanel->totalMultiplierValueText->set_color(color);
            modifiersPanel->maxRankValueText->set_color(color);
        }
        return ratingSelected;
    }

    void setup() {
        INSTALL_HOOK(BeatLeaderLogger, ModifierStart);
        INSTALL_HOOK(BeatLeaderLogger, RefreshMultipliers);
        INSTALL_HOOK(BeatLeaderLogger, ActivateMultiplayer);
        INSTALL_HOOK(BeatLeaderLogger, DeActivateMultiplayer);
    }

    void SetModifiersActive(bool active) {
        if (active) {
            refreshAllModifiers();
        } else {
            for (auto& [key, value] : allModifierToggles) {
                if(songModifiers.contains(key)){
                    float modifierValue = value->gameplayModifier->multiplier;
                    value->multiplierText->SetText((modifierValue > 0 ? "+" : "") + to_string_wprecision(modifierValue * 100.0f, 1) + "%");
                }
            }
        }
        ssActive = !active;
    }

    void ResetModifiersUI() {
        allModifierToggles = {};
    }
}