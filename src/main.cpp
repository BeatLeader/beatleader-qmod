#include "main.hpp"

#include "include/UI/LevelInfoUI.hpp"
#include "include/UI/LeaderboardUI.hpp"
#include "include/UI/ModifiersUI.hpp"
#include "include/UI/PreferencesViewController.hpp"
#include "include/UI/EmojiSupport.hpp"
#include "include/UI/ResultsViewController.hpp"

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
#include "BeatSaber/Init/BSAppInit.hpp"

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

MAKE_HOOK_MATCH(Restart, &MenuTransitionsHelper::RestartGame, void, MenuTransitionsHelper* self, ::System::Action_1<::Zenject::DiContainer*>* finishCallback) {
    Restart(self, finishCallback);

    LeaderboardUI::reset();
    LevelInfoUI::reset();
    EmojiSupport::Reset();
    Sprites::ResetCache();
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
    
    INSTALL_HOOK(BeatLeaderLogger, Restart);
    INSTALL_HOOK(BeatLeaderLogger, AppInitStart);

    BeatLeaderLogger.info("Installed main hooks!");
}