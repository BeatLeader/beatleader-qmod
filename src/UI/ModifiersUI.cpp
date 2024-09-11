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
#include "Core/SpeedModifiers.hpp"

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

    bool ModifiersAvailable() {
        if (modifiersPanel) {
            return true;
        } else {
            return false;
        }
    }

    TriangleRating refreshAllModifiers(){
        // Set the map dependant modifier values on the toggles (with colors)
        for(auto& [toggle, modifier] : ModifiersCoreQuest::ModifiersManager::Toggles()){
            string modifierSubText;
            if (songModifierRatings.contains(modifier.Id)) {
                modifierSubText = (string)"<color=yellow>New stars " + to_string_wprecision(UIUtils::getStarsToShow(songModifierRatings[modifier.Id]), 2);
            }
            else if(songModifiers.contains(modifier.Id)) {
                float modifierValue = songModifiers[modifier.Id];
                modifierSubText = (modifierValue > 0 ? "<color=#00FF77>+" : "<color=#00FFFF>") + to_string_wprecision(modifierValue * 100.0f, 1) + "%";
            }
            else {
                float modifierValue = modifier.Multiplier;
                modifierSubText = (modifierValue > 0 ? "+" : "") + to_string_wprecision(modifierValue * 100.0f, 1) + "%";
            }

            toggle->_multiplierText->SetText(modifierSubText, true);
        }
        return refreshMultiplierAndMaxRank();
    }

    TriangleRating refreshMultiplierAndMaxRank()
    {
        TriangleRating ratingSelected = {};
        // If we dont have a panel reference we cant do anything
        if (modifiersPanel && !songModifiers.empty()) {

            float totalMultiplier = 1;
            for (ModifiersCoreQuest::Modifier& param : ModifiersCoreQuest::ModifiersManager::get_Modifiers())
            {
                if (ModifiersCoreQuest::ModifiersManager::GetModifierState(param.Id) && param.Id != "NF") {
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
        INSTALL_HOOK(BeatLeaderLogger, RefreshMultipliers);
        INSTALL_HOOK(BeatLeaderLogger, ActivateMultiplayer);
        INSTALL_HOOK(BeatLeaderLogger, DeActivateMultiplayer);
    }

    void SetModifiersActive(bool active) {
        if (active) {
            refreshAllModifiers();
        } else {
            for (auto& [toggle, modifier] : ModifiersCoreQuest::ModifiersManager::Toggles()) {
                if(songModifiers.contains(modifier.Id)){
                    float modifierValue = toggle->gameplayModifier->multiplier;
                    toggle->_multiplierText->SetText((modifierValue > 0 ? "+" : "") + to_string_wprecision(modifierValue * 100.0f, 1) + "%", true);
                }
            }
        }
        ssActive = !active;
    }
}