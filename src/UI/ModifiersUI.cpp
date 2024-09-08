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

#include "ModifiersCoreQuest/shared/Utils/ModifierUtils.hpp"
#include "ModifiersCoreQuest/shared/Core/ModifiersManager.hpp"
#include "System/Collections/Generic/Dictionary_2.hpp"

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

        string key;
        // TODO WONT WORK FOR CUSTOMS
        if(!multiActive && ModifiersCoreQuest::ModifierUtils::TryGetBaseModifierIdBySerializedName(self->get_gameplayModifier()->get_modifierNameLocalizationKey(), key)) {
            BeatLeaderLogger.info("Added Modifier {}", key.c_str());
            allModifierToggles[key] = self;
        }
    }

    bool ModifiersAvailable() {
        if (modifiersPanel) {
            return true;
        } else {
            return false;
        }
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
                float modifierValue = ModifiersCoreQuest::ModifiersManager::GetModifierWithId(key).value().Multiplier;
                modifierSubText = (modifierValue > 0 ? "+" : "") + to_string_wprecision(modifierValue * 100.0f, 1) + "%";
            }

            // Set the text underneath the modifier
            value->_multiplierText->SetText(modifierSubText, true);
        }
        return refreshMultiplierAndMaxRank();
    }

    TriangleRating refreshMultiplierAndMaxRank()
    {
        TriangleRating ratingSelected;
        // If we dont have a panel reference we cant do anything
        if (modifiersPanel || songModifiers.empty()) {

            // Now we iterate all modifiers to set the totalMultiplier (% value on top) and the max achievable rank

            float totalMultiplier = 1;

            for (ModifiersCoreQuest::Modifier param : ModifiersCoreQuest::ModifiersManager::get_Modifiers())
            {
                // Base game
                // auto it = ModifiersCoreQuest::ModifiersManager::GameplayModifierParams.find(param.Id);
                // if (it != ModifiersCoreQuest::ModifiersManager::GameplayModifierParams.end()) {
                // BeatLeaderLogger.info("BaseGame Multiplier {}", param.Id);
                //     GlobalNamespace::__GameplayModifiersModelSO__GameplayModifierBoolGetter* getter = NULL;
                //     modifiersPanel->_gameplayModifiersModel->_gameplayModifierGetters->TryGetValue(it->second, getter);
                //     //adding if enabled
                //     if(getter && getter->Invoke(modifiersPanel->_gameplayModifiers)) {
                // BeatLeaderLogger.info("Enabled Multiplier {}", param.Id);
                //     if (songModifierRatings.contains(param.Id)) {
                //         // ModifierRatings apply to star value and have no effect on
                //         // max rank. But we need to return it so that it can be shown
                //         // in the respective labels
                //         ratingSelected = songModifierRatings[param.Id];
                //     } else if (songModifiers.contains(param.Id)) {
                //         totalMultiplier += songModifiers[param.Id];
                //     } else {
                //         totalMultiplier += param.Multiplier;
                //     }
                //     }
                if (ModifiersCoreQuest::ModifiersManager::GetModifierState(param.Id)) {
                    if (songModifierRatings.contains(param.Id)) {
                        // ModifierRatings apply to star value and have no effect on
                        // max rank. But we need to return it so that it can be shown
                        // in the respective labels
                        ratingSelected = songModifierRatings[param.Id];
                    } else if (songModifiers.contains(param.Id)) {
                        totalMultiplier += songModifiers[param.Id];
                    } else {
                        totalMultiplier += param.Multiplier;
                    }
                }
            }

            if (totalMultiplier < 0) totalMultiplier = 0; // thanks Beat Games for Zen mode -1000%

            // Correct texts & color of total multiplier & rank with our values
            modifiersPanel->_totalMultiplierValueText->SetText((totalMultiplier > 1 ? "+" : "") + to_string_wprecision(totalMultiplier * 100.0f, 1) + "%", true);
            modifiersPanel->_maxRankValueText->SetText(getRankForMultiplier(totalMultiplier), true);

            auto color = totalMultiplier >= 1 ? modifiersPanel->_positiveColor : modifiersPanel->_negativeColor;
            modifiersPanel->_totalMultiplierValueText->set_color(color);
            modifiersPanel->_maxRankValueText->set_color(color);
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
                    value->_multiplierText->SetText((modifierValue > 0 ? "+" : "") + to_string_wprecision(modifierValue * 100.0f, 1) + "%", true);
                }
            }
        }
        ssActive = !active;
    }

    void ResetModifiersUI() {
        allModifierToggles = {};
    }
}