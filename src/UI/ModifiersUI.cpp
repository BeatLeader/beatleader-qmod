#include "beatsaber-hook/shared/safeptr.hpp"
#include "include/Managers/LeaderboardHeaderManager.hpp"
#include "include/Managers/LeaderboardContextSwitcher.hpp"
#include "include/Core/CustomModifiers.hpp"
#include "include/UI/ModifiersUI.hpp"
#include "include/UI/UIUtils.hpp"
#include "include/Utils/StringUtils.hpp"
#include "shared/Models/TriangleRating.hpp"

#include "GlobalNamespace/GameplayModifierToggle.hpp"
#include "GlobalNamespace/GameplayModifierParamsSO.hpp"
#include "GlobalNamespace/GameplayModifiersPanelController.hpp"
#include "GlobalNamespace/GameplayModifiersModelSO.hpp"

#include "GlobalNamespace/GameServerLobbyFlowCoordinator.hpp"

#include "beatsaber-hook/shared/hooking.hpp"
#include "paper2_scotland2/shared/logger.hpp"

#include "ModifiersCoreQuest/shared/Core/ModifiersManager.hpp"

#include "main.hpp"

#include "TMPro/TMP_Text.hpp"
#include "TMPro/TextMeshProUGUI.hpp"

#include <cmath>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

using namespace GlobalNamespace;
using namespace std;

namespace ModifiersUI {
    static UnityEngine::Color positiveColor = UnityEngine::Color(1.0, 0.0, 0.73, 1.0);
    static UnityEngine::Color negativeColor = UnityEngine::Color(0.65, 0.49, 1.0, 1.0);
    safe_ptr<GameplayModifiersPanelController*> modifiersPanel;
    unordered_map<string, float> songModifiers;
    unordered_map<string, TriangleRating> songModifierRatings;
    static int songStatus = 0;
    bool ssActive = false;
    bool multiActive = false;

    static constexpr int RankedStatus = 3;
    static constexpr int EventStatus = 6;

    static unordered_map<string, string> defaultToggleTexts;

    static const unordered_map<string, string> baseSpeedModifiers = {
        { "BFS", "FS" },
        { "BSF", "SF" }
    };

    static const vector<string> ratedSpeedModifiers = { "SS", "BFS", "BSF", "FS", "SF" };

    static bool HasSongModifiers() {
        return !songModifiers.empty() || !songModifierRatings.empty();
    }

    static float GetMultiplier(string const& id) {
        auto base = baseSpeedModifiers.find(id);
        bool isCustomSpeed = base != baseSpeedModifiers.end();

        auto it = songModifiers.find(isCustomSpeed ? base->second : id);
        if (it != songModifiers.end()) {
            return it->second;
        }
        // The game's own multipliers, like Zen Mode's -1000%, don't count on BeatLeader
        if (isCustomSpeed || ModifiersCoreQuest::ModifiersManager::GameplayModifierParams.contains(id)) {
            return 0;
        }
        auto modifier = ModifiersCoreQuest::ModifiersManager::GetModifierWithId(id);
        return modifier ? modifier->Multiplier : 0;
    }

    struct ModifierEffects {
        float totalMultiplier = 1.0f;
        optional<TriangleRating> ratingSelected;
    };

    ModifierEffects GetModifierEffects() {
        ModifierEffects effects {};
        if (!HasSongModifiers()) {
            return effects;
        }

        for (auto const& id : ratedSpeedModifiers) {
            auto ratingIt = songModifierRatings.find(id);
            if (ratingIt != songModifierRatings.end() && CustomModifiers::IsEnabled(id)) {
                effects.ratingSelected = ratingIt->second;
                break;
            }
        }

        for (auto const& modifier : ModifiersCoreQuest::ModifiersManager::get_Modifiers()) {
            if (!CustomModifiers::IsEnabled(modifier.Id)) {
                continue;
            }

            // A modifier's own rating replaces its multiplier, same as on the server
            if (songModifierRatings.contains(modifier.Id)) {
                continue;
            }

            // NoFail only counts once the energy reaches zero
            if (modifier.Id != "NF") {
                effects.totalMultiplier += GetMultiplier(modifier.Id);
            }
        }

        if (effects.totalMultiplier < 0.0f) {
            effects.totalMultiplier = 0.0f;
        }

        return effects;
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
        BeatLeader::LeaderboardContextSwitcher::HandleModifiersChanged(self->gameplayModifiers);
        // Refresh Rating labels as a rating modifier could have been selected (this also calls refreshAllModifiers)
        if(!ssActive && !multiActive) {
            BeatLeader::LeaderboardHeaderManagerNS::Instance.RefreshMapStatus();
        }
    }

    bool ModifiersAvailable() {
        if (modifiersPanel) {
            return true;
        } else {
            return false;
        }
    }

    void SetSongModifiers(unordered_map<string, float> const& modifiers, unordered_map<string, TriangleRating> const& ratings, int status) {
        songModifiers = modifiers;
        songModifierRatings = ratings;
        songStatus = status;
        NegativeModifiers::UpdateExclusiveModifiers(songModifiers);
        refreshAllModifiers();
    }

    void ClearSongModifiers() {
        songModifiers.clear();
        songModifierRatings.clear();
        songStatus = 0;
        NegativeModifiers::UpdateExclusiveModifiers(songModifiers);
        refreshAllModifiers();
    }

    static string FormatToggleText(float multiplier) {
        bool isPositive = multiplier > 0;
        string text = (isPositive ? "+" : "") + to_string((int)std::round(multiplier * 100.0f)) + "%";
        return (isPositive ? "<color=#00FF77>" : "<color=#00FFFF>") + text + "</color>";
    }

    static void SetToggleText(GameplayModifierToggle* toggle, string const& id, string const& text) {
        if (!defaultToggleTexts.contains(id)) {
            defaultToggleTexts[id] = (string)toggle->_multiplierText->text;
        }
        toggle->_multiplierText->SetText(text, true);
    }

    static void RestoreToggleText(GameplayModifierToggle* toggle, string const& id) {
        auto it = defaultToggleTexts.find(id);
        if (it != defaultToggleTexts.end()) {
            toggle->_multiplierText->SetText(it->second, true);
        }
    }

    void refreshAllModifiers(){
        bool overridden = !ssActive && HasSongModifiers();
        // Set the map dependant modifier values on the toggles (with colors)
        for(auto& [toggle, modifier] : ModifiersCoreQuest::ModifiersManager::Toggles()){
            if (!toggle) {
                continue;
            }
            if (!overridden) {
                RestoreToggleText(toggle.ptr(), modifier.Id);
                continue;
            }

            auto ratingIt = songModifierRatings.find(modifier.Id);
            if (ratingIt != songModifierRatings.end()) {
                SetToggleText(toggle.ptr(), modifier.Id, "<color=yellow>★ " + to_string_wprecision(UIUtils::getStarsToShow(ratingIt->second), 2) + "</color>");
                continue;
            }
            if (modifier.Id == "NF") {
                if (songStatus == RankedStatus || songStatus == EventStatus) {
                    SetToggleText(toggle.ptr(), modifier.Id, "0% / 0pp");
                } else {
                    RestoreToggleText(toggle.ptr(), modifier.Id);
                }
                continue;
            }
            float multiplier = GetMultiplier(modifier.Id);
            SetToggleText(toggle.ptr(), modifier.Id, multiplier != 0 ? FormatToggleText(multiplier) : "");
        }
        refreshMultiplierAndMaxRank();
    }

    void refreshMultiplierAndMaxRank()
    {
        // If we dont have a panel reference we cant do anything
        if (modifiersPanel && !ssActive && HasSongModifiers()) {
            auto effects = GetModifierEffects();
            auto totalMultiplier = effects.totalMultiplier;

            // Correct texts & color of total multiplier & rank with our values
            modifiersPanel->_totalMultiplierValueText->SetText((totalMultiplier > 1 ? "+" : "") + to_string_wprecision(totalMultiplier * 100.0f, 1) + "%", true);
            modifiersPanel->_maxRankValueText->SetText(getRankForMultiplier(totalMultiplier), true);

            auto color = totalMultiplier >= 1 ? modifiersPanel->_positiveColor : modifiersPanel->_negativeColor;
            modifiersPanel->_totalMultiplierValueText->set_color(color);
            modifiersPanel->_maxRankValueText->set_color(color);
        }
    }

    TriangleRating ApplyCurrentModifiers(TriangleRating rating) {
        auto effects = GetModifierEffects();
        if (effects.ratingSelected) {
            rating = *effects.ratingSelected;
        }

        rating.stars *= effects.totalMultiplier;
        rating.techRating *= effects.totalMultiplier;
        rating.accRating *= effects.totalMultiplier;
        rating.passRating *= effects.totalMultiplier;
        return rating;
    }

    void setup() {
        INSTALL_HOOK(BeatLeaderLogger, RefreshMultipliers);
        INSTALL_HOOK(BeatLeaderLogger, ActivateMultiplayer);
        INSTALL_HOOK(BeatLeaderLogger, DeActivateMultiplayer);
    }

    void SetModifiersActive(bool active) {
        ssActive = !active;
        refreshAllModifiers();
        if (modifiersPanel && !active) {
            modifiersPanel->RefreshTotalMultiplierAndRankUI();
        }
    }
}
