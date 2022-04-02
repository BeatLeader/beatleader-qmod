#include "main.hpp"

#include "include/UI/LevelInfoUI.hpp"
#include "include/UI/LeaderboardUI.hpp"
#include "include/UI/ModifiersUI.hpp"
#include "include/UI/PreferencesViewController.hpp"

#include "include/API/PlayerController.hpp"
#include "include/Core/ReplayRecorder.hpp"

#include "include/Utils/ModConfig.hpp"
#include "include/Utils/ReplaySynchronizer.hpp"
#include "include/Utils/ModifiersManager.hpp"

#include "config-utils/shared/config-utils.hpp"
#include "custom-types/shared/register.hpp"

#include "GlobalNamespace/MenuTransitionsHelper.hpp"

#include "questui/shared/CustomTypes/Components/MainThreadScheduler.hpp"
#include "questui/shared/QuestUI.hpp"

using namespace GlobalNamespace;
using namespace QuestUI;

ModInfo modInfo; // Stores the ID and version of our mod, and is sent to the modloader upon startup

static ReplaySynchronizer* synchronizer;

DEFINE_CONFIG(ModConfig);

// Returns a logger, useful for printing debug messages
Logger& getLogger() {
    static Logger* logger = new Logger(modInfo, LoggerOptions(false, true));
    return *logger;
}

// Called at the early stages of game loading
extern "C" void setup(ModInfo& info) {
    info.id = ID;
    info.version = VERSION;
    modInfo = info;
	
    getModConfig().Init(modInfo); // Load the config file
    getLogger().info("Completed setup!");
}

MAKE_HOOK_MATCH(Restart, &MenuTransitionsHelper::RestartGame, void, MenuTransitionsHelper* self, ::System::Action_1<::Zenject::DiContainer*>* finishCallback) {
    Restart(self, finishCallback);

    LeaderboardUI::reset();
    LevelInfoUI::reset();
}

void replayPostCallback(ReplayUploadStatus status, string description, float progress, int code) {
    if (status == ReplayUploadStatus::finished) {
        PlayerController::Refresh();
    }
    if (synchronizer != NULL) {
        if (code == 200) {
            synchronizer->updateStatus(ReplayManager::lastReplayFilename, ReplayStatus::uptodate);
        } else if ((code >= 400 && code < 500) || code < 0) {
            synchronizer->updateStatus(ReplayManager::lastReplayFilename, ReplayStatus::shouldnotpost);
        }
    }
    
    QuestUI::MainThreadScheduler::Schedule([status, description, progress] {
        LeaderboardUI::updateStatus(status, description, progress);
    });
}

// Called later on in the game loading - a good time to install function hooks
extern "C" void load() {
    il2cpp_functions::Init();

    LoggerContextObject logger = getLogger().WithContext("load");

    QuestUI::Init();
    QuestUI::Register::RegisterModSettingsViewController<BeatLeader::PreferencesViewController*>(modInfo, "BeatLeader");
    
    LeaderboardUI::setup();
    LeaderboardUI::retryCallback = []() {
        ReplayManager::RetryPosting(replayPostCallback);
    };

    LevelInfoUI::setup();
    ModifiersUI::setup();

    PlayerController::playerChanged.push_back([](Player* updated) {
        synchronizer = new ReplaySynchronizer();
    });
    QuestUI::MainThreadScheduler::Schedule([] {
        PlayerController::Refresh();
    });

    ModifiersManager::Sync();

    ReplayRecorder::StartRecording([](Replay* replay, MapStatus status, bool isOst) {
        if (status == MapStatus::cleared) {
            ReplayManager::ProcessReplay(replay, isOst, replayPostCallback);
        } else {
            ReplayManager::ProcessReplay(replay, isOst, [](ReplayUploadStatus finished, string description, float progress, int code) {
                QuestUI::MainThreadScheduler::Schedule([description, progress, finished] {
                    LeaderboardUI::updateStatus(finished, description, progress);
                });
            });
        }
    });

    getLogger().info("Installing main hooks...");
    
    INSTALL_HOOK(logger, Restart);

    getLogger().info("Installed main hooks!");
}