// Dev harness, built with -DBL_DEV_AUTOPLAY=ON. Reads key=value lines from ConfigPath:
//   level=100Bills characteristic=Standard difficulty=3 custom=DNC,BFS practice=0 godmode=1  plays a level as "Party" (no upload)
//   open=solo toggle=DNC,BFS  flips custom modifier toggles in the Solo menu
//   open=dump dump=<name> depth=7 press=<button text>  logs a UI layout
#ifdef BL_DEV_AUTOPLAY

#include "include/Utils/ArrayUtils.hpp"
#include "beatsaber-hook/shared/hooking.hpp"
#include "bsml/shared/BSML/MainThreadScheduler.hpp"

#include "ModifiersCoreQuest/shared/Core/ModifiersManager.hpp"

#include "GlobalNamespace/BeatmapCharacteristic.hpp"
#include "GlobalNamespace/BeatmapCharacteristicExtensions.hpp"
#include "GlobalNamespace/GameMode.hpp"
#include "GlobalNamespace/GameplayAdditionalInformation.hpp"
#include "GlobalNamespace/GameplayType.hpp"
#include "GlobalNamespace/PlaymodeOptions.hpp"
#include "GlobalNamespace/BeatmapDifficulty.hpp"
#include "GlobalNamespace/BeatmapKey.hpp"
#include "GlobalNamespace/BeatmapLevel.hpp"
#include "GlobalNamespace/BeatmapLevelsModel.hpp"
#include "GlobalNamespace/GameEnergyCounter.hpp"
#include "GlobalNamespace/MainFlowCoordinator.hpp"
#include "GlobalNamespace/MenuTransitionsHelper.hpp"
#include "GlobalNamespace/GameplayModifierToggle.hpp"
#include "GlobalNamespace/GameplayModifiersPanelController.hpp"
#include "GlobalNamespace/GameplaySetupViewController.hpp"
#include "GlobalNamespace/MainMenuViewController.hpp"
#include "HMUI/TextSegmentedControl.hpp"
#include "System/Collections/Generic/List_1.hpp"
#include "TMPro/TextMeshProUGUI.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/UI/Toggle.hpp"
#include "UnityEngine/UI/LayoutElement.hpp"
#include "UnityEngine/UI/LayoutUtility.hpp"
#include "UnityEngine/UI/Button.hpp"
#include "UnityEngine/Events/UnityEvent.hpp"
#include "UnityEngine/Rect.hpp"
#include "UnityEngine/RectTransform.hpp"
#include "include/Utils/ModConfig.hpp"
#include "GlobalNamespace/PauseController.hpp"
#include "GlobalNamespace/PlayerData.hpp"
#include "GlobalNamespace/PlayerDataModel.hpp"
#include "GlobalNamespace/PracticeSettings.hpp"
#include "GlobalNamespace/SoloFreePlayFlowCoordinator.hpp"
#include "System/Nullable_1.hpp"
#include "UnityEngine/Resources.hpp"
#include "UnityEngine/Time.hpp"

#include "include/Core/Dance/DanceModeTracker.hpp"

#include "main.hpp"

#include <fstream>
#include <map>
#include <sstream>

using namespace GlobalNamespace;

static const char* ConfigPath = "/sdcard/ModData/com.beatgames.beatsaber/Mods/BeatLeader/autoplay.txt";
static bool started = false;
static bool godmode = false;

static std::map<std::string, std::string> ReadConfig() {
    std::map<std::string, std::string> config;
    std::ifstream file(ConfigPath);
    std::string line;
    while (std::getline(file, line)) {
        auto eq = line.find('=');
        if (eq == std::string::npos) continue;
        auto value = line.substr(eq + 1);
        while (!value.empty() && (value.back() == '\r' || value.back() == ' ')) value.pop_back();
        config[line.substr(0, eq)] = value;
    }
    return config;
}

static void StartLevel(MainFlowCoordinator* main, std::map<std::string, std::string> config) {
    auto level = main->_beatmapLevelsModel->GetBeatmapLevel(config["level"], false);
    if (!level) {
        BeatLeaderLogger.error("AutoPlay: no level {}", config["level"]);
        return;
    }

    std::optional<BeatmapKey> key;
    for (int i = BeatmapCharacteristic::Standard.value__; i <= BeatmapCharacteristic::Degree90.value__; i++) {
        BeatmapCharacteristic characteristic;
        characteristic.value__ = i;
        if ((std::string)BeatmapCharacteristicExtensions::SerializedName(characteristic) == config["characteristic"]) {
            BeatmapDifficulty difficulty;
            difficulty.value__ = std::stoi(config["difficulty"]);
            key = BeatmapKey(characteristic, difficulty, StringW(config["level"]), GameplayType::BeatSaber);
            break;
        }
    }
    if (!key || !level->GetDifficultyBeatmapData(by_ref<BeatmapKey const>(*key))) {
        BeatLeaderLogger.error("AutoPlay: no {} difficulty {} (0 Easy .. 4 ExpertPlus)", config["characteristic"], config["difficulty"]);
        return;
    }

    for (auto const& modifier : ModifiersCoreQuest::ModifiersManager::get_CustomModifiers()) {
        ModifiersCoreQuest::ModifiersManager::SetModifierState(modifier.Id, config["custom"].find(modifier.Id) != std::string::npos);
    }
    BeatLeaderLogger.info("AutoPlay: custom modifiers {}", config["custom"]);

    godmode = config["godmode"] == "1";
    PracticeSettings* practice = config["practice"] == "1" ? PracticeSettings::New_ctor(0, 1.2f) : nullptr;
    auto playerData = main->_playerDataModel->playerData;

    BeatLeaderLogger.info("AutoPlay: starting {} {} {}", config["level"], config["characteristic"], config["difficulty"]);
    main->_menuTransitionsHelper->StartStandardLevel(
        GameMode::Party, by_ref<BeatmapKey const>(*key), level, playerData->overrideEnvironmentSettings, nullptr, false,
        playerData->gameplayModifiers, playerData->playerSpecificSettings, practice,
        main->_soloFreePlayFlowCoordinator->_environmentsListModel,
        GameplayAdditionalInformation::New_ctor("Menu", false, false, PlaymodeOptions(), nullptr, false, nullptr, false),
        nullptr, nullptr, nullptr, nullptr, nullptr);
}

static void CheckModifiersMenu(MainFlowCoordinator* main, std::map<std::string, std::string> config) {
    main->HandleMainMenuViewControllerDidFinish(nullptr, MainMenuViewController::MenuButton::SoloFreePlay);
    BSML::MainThreadScheduler::ScheduleAfterTime(3, [config] {
        auto setup = BeatLeader::FirstOrDefault(UnityEngine::Resources::FindObjectsOfTypeAll<GameplaySetupViewController*>());
        if (!setup) {
            BeatLeaderLogger.error("AutoPlay: no gameplay setup view");
            return;
        }
        auto panels = setup->_panels;
        for (int i = 0; i < panels->Count; i++) {
            if (panels->get_Item(i)->gameObject->GetComponent<GameplayModifiersPanelController*>()) {
                setup->_selectionSegmentedControl->SelectCellWithNumber(i);
                setup->SetActivePanel(i);
                BeatLeaderLogger.info("AutoPlay: modifiers panel {}", i);
            }
        }
        BSML::MainThreadScheduler::ScheduleAfterTime(2, [config] {
            for (auto& [toggle, modifier] : ModifiersCoreQuest::ModifiersManager::Toggles()) {
                if (!toggle) continue;
                BeatLeaderLogger.info("AutoPlay: toggle {} active {} on {} text '{}' parent {}", modifier.Id, toggle->gameObject->activeInHierarchy,
                    toggle->_toggle->isOn, (std::string)toggle->_multiplierText->text, (std::string)toggle->transform->parent->name);
            }
            auto selector = UnityEngine::GameObject::Find("ModifiersCoreTabSelector");
            BeatLeaderLogger.info("AutoPlay: tab selector {} context {}", selector ? selector->activeInHierarchy : false, getModConfig().Context.GetValue());
            std::stringstream ids(config.contains("toggle") ? config.at("toggle") : "BFS");
            std::string toggleId;
            while (std::getline(ids, toggleId, ',')) {
                for (auto& [toggle, modifier] : ModifiersCoreQuest::ModifiersManager::Toggles()) {
                    if (modifier.Id == toggleId && toggle) {
                        toggle->_toggle->isOn = !toggle->_toggle->isOn;
                        BeatLeaderLogger.info("AutoPlay: toggled {} to {}, context {}, manual {}, text '{}'", toggleId, toggle->_toggle->isOn,
                            getModConfig().Context.GetValue(), getModConfig().ManualContext.GetValue(), (std::string)toggle->_multiplierText->text);
                    }
                }
            }
        });
    });
}

static void DumpLayout(UnityEngine::Transform* transform, int depth, int maxDepth) {
    auto rect = i2c::try_cast<UnityEngine::RectTransform*>(transform);
    if (rect) {
        auto r = rect->rect;
        auto element = transform->GetComponent<UnityEngine::UI::LayoutElement*>();
        BeatLeaderLogger.info("AutoPlay: {}{} active {} size {:.1f}x{:.1f} pos {:.1f},{:.1f} world x {:.2f} pref {:.1f} min {:.1f} flex {:.1f} layout pref {:.1f} min {:.1f} flex {:.1f}",
            std::string(depth * 2, ' '), (std::string)transform->name, transform->gameObject->activeInHierarchy, r.width, r.height,
            rect->anchoredPosition.x, rect->anchoredPosition.y, transform->position.x,
            element ? element->preferredWidth : -1, element ? element->minWidth : -1, element ? element->flexibleWidth : -1,
            UnityEngine::UI::LayoutUtility::GetPreferredWidth(rect), UnityEngine::UI::LayoutUtility::GetMinWidth(rect),
            UnityEngine::UI::LayoutUtility::GetFlexibleWidth(rect));
    }
    if (depth >= maxDepth) return;
    for (int i = 0; i < transform->childCount; i++) {
        DumpLayout(transform->GetChild(i), depth + 1, maxDepth);
    }
}

MAKE_HOOK_MATCH(AutoPlayMainFlowDidActivate, &MainFlowCoordinator::DidActivate, void, MainFlowCoordinator* self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
    AutoPlayMainFlowDidActivate(self, firstActivation, addedToHierarchy, screenSystemEnabling);
    if (started) return;
    auto config = ReadConfig();
    if (config.empty()) return;
    started = true;
    if (config["open"] == "solo") {
        BSML::MainThreadScheduler::ScheduleAfterTime(5, [self, config] { CheckModifiersMenu(self, config); });
        return;
    }
    if (config["open"] == "dump") {
        BSML::MainThreadScheduler::ScheduleAfterTime(8, [config] {
            std::string name = config.contains("dump") ? config.at("dump") : "BeatLeaderNewsViewController";
            auto go = UnityEngine::GameObject::Find(name);
            if (!go) {
                for (auto transform : UnityEngine::Resources::FindObjectsOfTypeAll<UnityEngine::RectTransform*>()) {
                    if ((std::string)transform->name == name) { go = transform->gameObject; break; }
                }
            }
            if (!go) {
                BeatLeaderLogger.error("AutoPlay: nothing named {}", name);
                return;
            }
            int depth = config.contains("depth") ? std::stoi(config.at("depth")) : 7;
            DumpLayout(go->transform, 0, depth);
            if (!config.contains("press")) return;
            for (auto button : go->GetComponentsInChildren<UnityEngine::UI::Button*>()) {
                auto text = button->GetComponentInChildren<TMPro::TextMeshProUGUI*>();
                if (text && (std::string)text->text == config.at("press")) {
                    BeatLeaderLogger.info("AutoPlay: pressing {}", config.at("press"));
                    button->onClick->Invoke();
                    break;
                }
            }
            safe_ptr<UnityEngine::GameObject*> root = go.ptr();
            BSML::MainThreadScheduler::ScheduleAfterTime(1, [root, depth] {
                if (root) DumpLayout(root->transform, 0, depth);
            });
        });
        return;
    }
    BSML::MainThreadScheduler::ScheduleAfterTime(5, [self, config] { StartLevel(self, config); });
}

MAKE_HOOK_MATCH(AutoPlayProcessEnergyChange, &GameEnergyCounter::ProcessEnergyChange, void, GameEnergyCounter* self, float energyChange) {
    static int calls = 0;
    if (godmode && self->energy + energyChange < 0.05f) {
        energyChange = 0.05f - self->energy;
    }
    AutoPlayProcessEnergyChange(self, energyChange);
    if (godmode && ++calls % 90 == 0) {
        auto live = DanceModeTracker::Live();
        BeatLeaderLogger.info("AutoPlay: energy {} real time {} live ready {} level {} counted {} drain {} song time {}",
            self->energy, UnityEngine::Time::get_timeSinceLevelLoad(),
            live ? live->Ready : false, live ? live->Level : -1, live ? live->CountedFraction : -1, live ? live->ShouldDrain : false, live ? live->SongTime : -1);
    }
}

// The headset pauses the game when nobody wears it
MAKE_HOOK_MATCH(AutoPlayPause, &PauseController::Pause, void, PauseController* self) {
    if (started) {
        BeatLeaderLogger.info("AutoPlay: skipped a pause");
        return;
    }
    AutoPlayPause(self);
}

void AutoPlaySetup() {
    INSTALL_HOOK(BeatLeaderLogger, AutoPlayMainFlowDidActivate);
    INSTALL_HOOK(BeatLeaderLogger, AutoPlayProcessEnergyChange);
    INSTALL_HOOK(BeatLeaderLogger, AutoPlayPause);
}

#endif
