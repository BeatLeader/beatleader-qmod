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
#include "include/Utils/ReplaySynchronizer.hpp"
#include "include/Utils/PlaylistSynchronizer.hpp"
#include "include/Utils/WebUtils.hpp"
#include "include/Utils/RecorderUtils.hpp"
#include "include/Utils/FileManager.hpp"

#include "config-utils/shared/config-utils.hpp"
#include "custom-types/shared/register.hpp"

#include "GlobalNamespace/MenuTransitionsHelper.hpp"
#include "GlobalNamespace/AppInit.hpp"

#include "questui/shared/CustomTypes/Components/MainThreadScheduler.hpp"
#include "questui/shared/QuestUI.hpp"

#include "leaderboardcore/shared/LeaderboardCore.hpp"

using namespace GlobalNamespace;
using namespace QuestUI;

ModInfo modInfo; // Stores the ID and version of our mod, and is sent to the modloader upon startup

static std::optional<ReplaySynchronizer> synchronizer;

// Returns a logger, useful for printing debug messages
Logger& getLogger() {
    static Logger* logger = new Logger(modInfo, LoggerOptions(false, true));
    return *logger;
}

// Called at the early stages of game loading
extern "C" void setup(ModInfo& info) {
    info.id = MOD_ID;
    info.version = VERSION;
    modInfo = info;
	
    getModConfig().Init(modInfo); // Load the config file
    getLogger().info("Completed setup!");
}

MAKE_HOOK_MATCH(Restart, &MenuTransitionsHelper::RestartGame, void, MenuTransitionsHelper* self, ::System::Action_1<::Zenject::DiContainer*>* finishCallback) {
    Restart(self, finishCallback);

    LeaderboardUI::reset();
    LevelInfoUI::reset();
    EmojiSupport::Reset();
    Sprites::ResetCache();
}

void replayPostCallback(ReplayUploadStatus status, const string& description, float progress, int code) {
    if (synchronizer != std::nullopt) {
        if (code == 200) {
            synchronizer->updateStatus(ReplayManager::lastReplayFilename, ReplayStatus::uptodate);
        } else if ((code >= 400 && code < 500) || code < 0) {
            synchronizer->updateStatus(ReplayManager::lastReplayFilename, ReplayStatus::shouldnotpost);
        }
    }

    if (!ReplayRecorder::recording) {
        QuestUI::MainThreadScheduler::Schedule([status, description, progress, code] {
            LeaderboardUI::updateStatus(status, description, progress, code > 450 || code < 200);
            if (status == ReplayUploadStatus::finished) {
                QuestUI::MainThreadScheduler::Schedule([]{
                    PlayerController::Refresh(0, [](auto player, auto str){
                        QuestUI::MainThreadScheduler::Schedule([]{
                            LeaderboardUI::updatePlayerRank();
                        });
                    });
                });
            }
        });
    }
}

MAKE_HOOK_MATCH(AppInitStart, &AppInit::Start, void,
    AppInit *self) {
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
        auto cb = self->blockerGO->get_gameObject()->GetComponent<UnityEngine::Canvas*>();
        cb->set_overrideSorting(false);

        auto cm = self->get_gameObject()->GetComponent<UnityEngine::Canvas*>();
        cm->set_overrideSorting(false); 
    }
}

// Called later on in the game loading - a good time to install function hooks
extern "C" void load() {
    il2cpp_functions::Init();
    custom_types::Register::AutoRegister();
    WebUtils::refresh_urls();
    FileManager::EnsureReplaysFolderExists();

    LoggerContextObject logger = getLogger().WithContext("load");

    QuestUI::Init();
    QuestUI::Register::RegisterModSettingsViewController<BeatLeader::PreferencesViewController*>(modInfo, "BeatLeader");
    LeaderboardUI::retryCallback = []() {
        ReplayManager::RetryPosting(replayPostCallback);
    };

    LevelInfoUI::setup();
    ModifiersUI::setup();
    ResultsView::setup();
    RecorderUtils::StartRecorderUtils();

    PlayerController::playerChanged.emplace_back([](optional<Player> const& updated) {
        // if (synchronizer == nullopt) {
        //     synchronizer.emplace();
        // }
    });
    QuestUI::MainThreadScheduler::Schedule([] {
        PlayerController::Refresh();
        LoggerContextObject logger = getLogger().WithContext("load");
        INSTALL_HOOK(logger, ModalView_Show);
    });

    PlaylistSynchronizer::SyncPlaylist();

    ReplayRecorder::StartRecording(
        []() {
            LeaderboardUI::hidePopups();
        },
        [](Replay const& replay, MapStatus status, bool skipUpload) {
        if (status == MapStatus::cleared) {
            ReplayManager::ProcessReplay(replay, skipUpload, replayPostCallback);
        } else {
            ReplayManager::ProcessReplay(replay, skipUpload, [](ReplayUploadStatus finished, string description, float progress, int code) {
                QuestUI::MainThreadScheduler::Schedule([description, progress, finished, code] {
                    LeaderboardUI::updateStatus(finished, description, progress, code > 450 || code < 200);
                });
            });
        }
    });

    getLogger().info("Installing main hooks...");
    
    INSTALL_HOOK(logger, Restart);
    INSTALL_HOOK(logger, AppInitStart);

    getLogger().info("Installed main hooks!");

    LeaderboardCore::Register::RegisterLeaderboard(&LeaderboardUI::leaderboard, modInfo);
}