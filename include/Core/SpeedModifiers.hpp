#include "Assets/BundleLoader.hpp"
#include "ModifiersCoreQuest/shared/Core/CustomModifier.hpp"
#include "ModifiersCoreQuest/shared/Core/ModifierCategories.hpp"
#include "ModifiersCoreQuest/shared/Core/ModifiersManager.hpp"
#include "GlobalNamespace/GameplayModifiers.hpp"
#include <vector>

namespace SpeedModifiers {
    inline static const ModifiersCoreQuest::CustomModifier get_BFS(){
        return {
            "BFS",
            "Better Faster Song",
            "Increases song speed by 20%, note speed by 10%",
            BundleLoader::bundle->BFSIcon,
            0.1,
            (std::vector<std::string>){ ModifiersCoreQuest::ModifierCategories::Speed },
            (std::vector<std::string>){ ModifiersCoreQuest::ModifierCategories::Speed },
            std::optional<std::vector<std::string>>(),
            std::optional<std::vector<std::string>>(),
            std::optional<std::vector<std::string>>(),
            UnityEngine::Color::get_blue(),
            UnityEngine::Color::get_yellow(),
        };
    }
    inline static const ModifiersCoreQuest::CustomModifier get_BSF(){
        return {
            "BSF",
            "Better Super Fast Song",
            "Increases song speed by 50%, note speed by 25%",
            BundleLoader::bundle->BSFIcon,
            0.2,
            (std::vector<std::string>){ ModifiersCoreQuest::ModifierCategories::Speed },
            (std::vector<std::string>){ ModifiersCoreQuest::ModifierCategories::Speed },
            std::optional<std::vector<std::string>>(),
            std::optional<std::vector<std::string>>(),
            std::optional<std::vector<std::string>>(),
            UnityEngine::Color::get_blue(),
            UnityEngine::Color::get_yellow(),
        };
    }

    inline static GlobalNamespace::GameplayModifiers::SongSpeed GetSongSpeed() {
        if (ModifiersCoreQuest::ModifiersManager::GetModifierState(get_BFS().Id)) {
            return GlobalNamespace::GameplayModifiers::SongSpeed::Faster;
        }
        if (ModifiersCoreQuest::ModifiersManager::GetModifierState(get_BSF().Id)) {
            return GlobalNamespace::GameplayModifiers::SongSpeed::SuperFast;
        }
        return GlobalNamespace::GameplayModifiers::SongSpeed::Normal;
    }

    inline static float GetSongSpeedMultiplier() {
        if(ModifiersCoreQuest::ModifiersManager::GetModifierState(get_BFS().Id)) {
            return 1.2f;
        }
        if (ModifiersCoreQuest::ModifiersManager::GetModifierState(get_BSF().Id)) {
            return 1.5f;
        }
        return 1;
    }

    void setup();
}