#include "main.hpp"

#include "include/UI/LevelInfoUI.hpp"
#include "include/UI/LeaderboardUI.hpp"
#include "include/UI/ModifiersUI.hpp"
#include "include/UI/PreferencesViewController.hpp"
#include "include/UI/EmojiSupport.hpp"
#include "include/UI/ResultsViewController.hpp"
#include "include/UI/QuestUI.hpp"

#include "include/API/PlayerController.hpp"
#include "include/Core/ReplayRecorder.hpp"

#include "include/Assets/BundleLoader.hpp"
#include "include/Assets/Sprites.hpp"

#include "include/Utils/ModConfig.hpp"
#include "include/Utils/PlaylistSynchronizer.hpp"
#include "include/Utils/WebUtils.hpp"
#include "include/Utils/FileManager.hpp"

#include "config-utils/shared/config-utils.hpp"
#include "custom-types/shared/register.hpp"

#include "GlobalNamespace/MenuTransitionsHelper.hpp"
#include "GlobalNamespace/AppInit.hpp"
#include "GlobalNamespace/RichPresenceManager.hpp"
#include "GlobalNamespace/MainFlowCoordinator.hpp"
#include "GlobalNamespace/SettingsFlowCoordinator.hpp"

#include "BeatSaber/Init/BSAppInit.hpp"
#include "UnityEngine/SceneManagement/SceneManager.hpp"
#include "UnityEngine/SceneManagement/Scene.hpp"

#include "bsml/shared/BSML/MainThreadScheduler.hpp"
#include "bsml/shared/BSML.hpp"

using namespace GlobalNamespace;
using namespace BSML;

// Called at the early stages of game loading
MOD_EXPORT void setup(CModInfo *info) noexcept {
    *info = modInfo.to_c();

    getModConfig().Init(modInfo);

    Paper::Logger::RegisterFileContextId(BeatLeaderLogger.tag);

    BeatLeaderLogger.info("Completed setup!");
}

void resetUI() {
    LeaderboardUI::reset();
    LevelInfoUI::reset();
    EmojiSupport::Reset();
    Sprites::ResetCache();
}

MAKE_HOOK_MATCH(HandleSettingsFlowCoordinatorDidFinish, &MainFlowCoordinator::HandleSettingsFlowCoordinatorDidFinish, void, MainFlowCoordinator* self, ::GlobalNamespace::SettingsFlowCoordinator* settingsFlowCoordinator, ::GlobalNamespace::__SettingsFlowCoordinator__FinishAction finishAction) {
    HandleSettingsFlowCoordinatorDidFinish(self, settingsFlowCoordinator, finishAction);

    if (finishAction != ::GlobalNamespace::__SettingsFlowCoordinator__FinishAction::Cancel) {
        resetUI();
    }
}

MAKE_HOOK_MATCH(MenuTransitionsHelperRestartGame, &MenuTransitionsHelper::RestartGame, void, MenuTransitionsHelper* self, ::System::Action_1<::Zenject::DiContainer*>* finishCallback) {
    MenuTransitionsHelperRestartGame(self, finishCallback);

    resetUI();
}

void replayPostCallback(ReplayUploadStatus status, const string& description, float progress, int code) {
    if (!ReplayRecorder::recording) {
        BSML::MainThreadScheduler::Schedule([status, description, progress, code] {
            LeaderboardUI::updateStatus(status, description, progress, code > 450 || code < 200);
            if (status == ReplayUploadStatus::finished) {
                std::thread t ([] {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                    PlayerController::Refresh(0, [](auto player, auto str){
                        BSML::MainThreadScheduler::Schedule([]{
                            LeaderboardUI::updatePlayerRank();
                        });
                    });
                    }
                );
                t.detach();
            }
        });
    }
}

MAKE_HOOK_MATCH(AppInitStart, &BeatSaber::Init::BSAppInit::InstallBindings, void,
    BeatSaber::Init::BSAppInit *self) {
    self->::UnityEngine::MonoBehaviour::StartCoroutine(custom_types::Helpers::CoroutineHelper::New(BundleLoader::LoadBundle(self->get_gameObject())));
    AppInitStart(self);
    LeaderboardUI::setup();
}

static bool hasInited = false;
static bool shouldClear = false;

// do things with the scene transition stuff
MAKE_HOOK_MATCH(RichPresenceManager_HandleGameScenesManagerTransitionDidFinish, &GlobalNamespace::RichPresenceManager::HandleGameScenesManagerTransitionDidFinish, void, GlobalNamespace::RichPresenceManager* self, GlobalNamespace::ScenesTransitionSetupDataSO* setupData, Zenject::DiContainer* container) {
    RichPresenceManager_HandleGameScenesManagerTransitionDidFinish(self, setupData, container);

    if (shouldClear) {
        shouldClear = false;
        QuestUI::ClearCache();
        if (hasInited) {
            hasInited = false;
            QuestUI::SetupPersistentObjects();
        }
    }
}

// Here we just check if we should be doing things after all the scene transitions are done:
MAKE_HOOK_MATCH(SceneManager_Internal_ActiveSceneChanged, &UnityEngine::SceneManagement::SceneManager::Internal_ActiveSceneChanged, void, UnityEngine::SceneManagement::Scene prevScene, UnityEngine::SceneManagement::Scene nextScene) {
    SceneManager_Internal_ActiveSceneChanged(prevScene, nextScene);
    bool prevValid = prevScene.IsValid(), nextValid = nextScene.IsValid();

    if (prevValid && nextValid) {
        std::string prevSceneName(prevScene.get_name());
        std::string nextSceneName(nextScene.get_name());

        if (prevSceneName == "QuestInit") hasInited = true;

        // if we just inited, and aren't already going to clear, check the next scene name for the menu
        if (hasInited && !shouldClear && nextSceneName.find("Menu") != std::u16string::npos) {
            shouldClear = true;
        }
    }
}

#include "HMUI/ModalView.hpp"
#include "HMUI/Screen.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/Canvas.hpp"
#include "UnityEngine/GameObject.hpp"

MAKE_HOOK_MATCH(ModalView_Show, &HMUI::ModalView::Show, void, HMUI::ModalView* self, bool animated, bool moveToCenter, System::Action* finishedCallback)
{
	ModalView_Show(self, animated, moveToCenter, finishedCallback); 

    if (((string)self->get_name()).find("BeatLeader") != string::npos) {
        auto cb = self->_blockerGO->get_gameObject()->GetComponent<UnityEngine::Canvas*>();
        cb->set_overrideSorting(false);

        auto cm = self->get_gameObject()->GetComponent<UnityEngine::Canvas*>();
        cm->set_overrideSorting(false); 
    }
}

// Called later on in the game loading - a good time to install function hooks
MOD_EXPORT "C" void late_load() {
    il2cpp_functions::Init();
    custom_types::Register::AutoRegister();
    WebUtils::refresh_urls();
    FileManager::EnsureReplaysFolderExists();

    BSML::Init();
    BSML::Register::RegisterSettingsMenu<BeatLeader::PreferencesViewController*>("BeatLeader");
    
    LeaderboardUI::retryCallback = []() {
        ReplayManager::RetryPosting(replayPostCallback);
    };

    LevelInfoUI::setup();
    ModifiersUI::setup();
    ResultsView::setup();

    PlayerController::playerChanged.emplace_back([](optional<Player> const& updated) {
        // if (synchronizer == nullopt) {
        //     synchronizer.emplace();
        // }
    });
    BSML::MainThreadScheduler::Schedule([] {
        PlayerController::Refresh();
        INSTALL_HOOK(BeatLeaderLogger, ModalView_Show);
    });

    PlaylistSynchronizer::SyncPlaylist();

    ReplayRecorder::StartRecording(
        []() {
            LeaderboardUI::hidePopups();
        },
        [](Replay const& replay, PlayEndData status, bool skipUpload) {
            ReplayManager::ProcessReplay(replay, status, skipUpload, replayPostCallback); 
        });

    BeatLeaderLogger.info("Installing main hooks...");
    
    INSTALL_HOOK(BeatLeaderLogger, HandleSettingsFlowCoordinatorDidFinish);
    INSTALL_HOOK(BeatLeaderLogger, MenuTransitionsHelperRestartGame);
    INSTALL_HOOK(BeatLeaderLogger, AppInitStart);
    INSTALL_HOOK(BeatLeaderLogger, SceneManager_Internal_ActiveSceneChanged);
    INSTALL_HOOK(BeatLeaderLogger, RichPresenceManager_HandleGameScenesManagerTransitionDidFinish);

    BeatLeaderLogger.info("Installed main hooks!");
}