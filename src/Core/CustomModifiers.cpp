#include "Core/CustomModifiers.hpp"
#include "Core/SpeedModifiers.hpp"

#include "Assets/BundleLoader.hpp"
#include "Core/ReplayPlayer.hpp"
#include "Utils/FileManager.hpp"
#include "ModifiersCoreQuest/shared/Core/ModifierCategories.hpp"
#include "ModifiersCoreQuest/shared/Core/ModifiersManager.hpp"

#include "GlobalNamespace/GameplayModifierToggle.hpp"
#include "UnityEngine/UI/Toggle.hpp"

#include "main.hpp"

#include <algorithm>
#include <exception>
#include <optional>
#include <sstream>
#include <vector>

using namespace ModifiersCoreQuest;

static const std::vector<std::string> AlwaysNegative = { "NF", "NO", "NB", "NA", "SS", "SC" };

static bool Contains(std::vector<std::string> const& ids, std::string const& id) {
    return std::find(ids.begin(), ids.end(), id) != ids.end();
}

static std::vector<std::string> With(std::vector<std::string> ids, std::string const& id) {
    ids.push_back(id);
    return ids;
}

static CustomModifier SpeedModifier(std::string const& id, std::string const& name, std::string const& description, UnityEngine::Sprite* icon, float multiplier) {
    std::vector<std::string> categories = { ModifierCategories::Speed };
    return {{
        id,
        name,
        description,
        icon,
        multiplier,
        categories,
        categories,
        std::nullopt,
        std::nullopt,
        std::nullopt
    }, UnityEngine::Color::get_blue(), UnityEngine::Color::get_yellow()};
}

static std::vector<CustomModifier> Definitions() {
    auto bundle = BundleLoader::bundle;
    std::vector<std::string> energy = { ModifierCategories::Energy };
    return {
        SpeedModifier(SpeedModifiers::BFSId, "Better Faster Song", "Increases song speed by 20%, note speed by 10%", bundle->BFSIcon, 0.1f),
        SpeedModifier(SpeedModifiers::BSFId, "Better Super Fast Song", "Increases song speed by 50%, note speed by 25%", bundle->BSFIcon, 0.2f),
        {{
            DanceModifier::Id,
            "Dance Mode",
            "Score is how much you dance. Energy also drains when you stop dancing for a while",
            // Placeholder, Dance Mode has no icon yet
            bundle->profileIcon,
            0,
            energy,
            energy,
            With(AlwaysNegative, GolfModifier::Id),
            std::nullopt,
            std::nullopt
        }, UnityEngine::Color(0.6f, 0.2f, 0.9f, 1.0f), UnityEngine::Color::get_yellow()},
        {{
            GolfModifier::Id,
            "Golf",
            "Score goes only to the Golf leaderboard, where lower accuracy is better. Needs accuracy of at most 50%",
            bundle->golfIcon,
            0,
            std::nullopt,
            std::nullopt,
            With(AlwaysNegative, DanceModifier::Id),
            std::nullopt,
            std::nullopt
        }, UnityEngine::Color(0.2f, 0.75f, 0.3f, 1.0f), UnityEngine::Color::get_yellow()}
    };
}

namespace CustomModifiers {
    static std::optional<std::vector<std::string>> replayModifiers;
    static std::vector<std::string> activeModifiers;

    void Register() {
        for (auto& modifier : Definitions()) {
            if (ModifiersManager::HasModifierWithId(modifier.Id)) {
                continue;
            }
            if (!modifier.Icon) {
                BeatLeaderLogger.warn("No {} icon in the bundle, using a placeholder", modifier.Id);
                modifier.Icon = BundleLoader::bundle->unknownIcon;
            }
            ModifiersManager::AddModifier(modifier);
        }
    }

    bool IsEnabled(std::string const& id) {
        try {
            return ModifiersManager::HasModifierWithId(id) && ModifiersManager::GetModifierState(id);
        } catch (std::exception const&) {
            return false;
        }
    }

    void LoadReplayModifiers(std::string const& replayPath) {
        auto info = FileManager::ReadInfo(replayPath);
        std::stringstream stream(info ? info->modifiers : "");
        replayModifiers.emplace();
        for (std::string code; std::getline(stream, code, ',');) {
            replayModifiers->push_back(code);
        }
    }

    void BeginLevel() {
        activeModifiers.clear();
        bool inReplay = ReplayInstalled() && IsInReplay();
        for (auto const& modifier : ModifiersManager::get_CustomModifiers()) {
            if (inReplay ? replayModifiers && Contains(*replayModifiers, modifier.Id) : IsEnabled(modifier.Id)) {
                activeModifiers.push_back(modifier.Id);
            }
        }
        if (inReplay) {
            replayModifiers = std::nullopt;
        }
    }

    bool IsActive(std::string const& id) {
        return Contains(activeModifiers, id);
    }
}

namespace NegativeModifiers {
    static std::vector<std::string> negativeModifiers = AlwaysNegative;

    static const std::vector<std::pair<std::string, std::string>> restrictedModifiers = {
        { DanceModifier::Id, GolfModifier::Id },
        { GolfModifier::Id, DanceModifier::Id }
    };

    static void DisableModifier(std::string const& id) {
        for (auto& [toggle, modifier] : ModifiersManager::Toggles()) {
            if (modifier.Id == id && toggle) {
                toggle->_toggle->isOn = false;
                return;
            }
        }
        ModifiersManager::SetModifierState(id, false);
    }

    void UpdateExclusiveModifiers(std::unordered_map<std::string, float> const& modifierValues) {
        negativeModifiers = AlwaysNegative;
        for (auto const& [id, value] : modifierValues) {
            if (value < 0 && ModifiersManager::HasModifierWithId(id) && !Contains(negativeModifiers, id)) {
                negativeModifiers.push_back(id);
            }
        }

        for (auto const& [id, other] : restrictedModifiers) {
            if (!ModifiersManager::HasModifierWithId(id)) {
                continue;
            }
            ModifiersManager::SetMutuallyExclusiveModifiers(id, With(negativeModifiers, other));
            if (HasNegativeModifiers(id)) {
                DisableModifier(id);
            }
        }
    }

    bool HasNegativeModifiers(std::string const& id) {
        return CustomModifiers::IsEnabled(id) && std::any_of(negativeModifiers.begin(), negativeModifiers.end(), CustomModifiers::IsEnabled);
    }
}
