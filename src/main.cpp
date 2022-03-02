#include "main.hpp"

#include "include/Models/Replay.hpp"
#include "include/Utils/ReplayManager.hpp"
#include "include/Enhancers/MapEnhancer.hpp"

#include "GlobalNamespace/NoteController.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"

#include "questui/shared/QuestUI.hpp"
#include "questui/shared/ArrayUtil.hpp"

#include "UnityEngine/Application.hpp"
#include "UnityEngine/Resources.hpp"

#include "System/Action_1.hpp"
#include "System/Threading/Tasks/Task.hpp"
#include "System/Threading/Tasks/Task_1.hpp"

#include "GlobalNamespace/SoloFreePlayFlowCoordinator.hpp"
#include "GlobalNamespace/BeatmapDifficulty.hpp"
#include "GlobalNamespace/BeatmapData.hpp"
#include "GlobalNamespace/GameplayModifiers.hpp"
#include "GlobalNamespace/LevelCompletionResults.hpp"
#include "GlobalNamespace/StandardLevelScenesTransitionSetupDataSO.hpp"
#include "GlobalNamespace/BeatmapEnvironmentHelper.hpp"
#include "GlobalNamespace/OverrideEnvironmentSettings.hpp"
#include "GlobalNamespace/GameplayCoreInstaller.hpp"
#include "GlobalNamespace/AudioTimeSyncController.hpp"
#include "GlobalNamespace/BeatmapObjectSpawnMovementData.hpp"
#include "GlobalNamespace/BeatmapObjectData.hpp"
#include "GlobalNamespace/NoteData.hpp"
#include "GlobalNamespace/BeatmapObjectSpawnMovementData_NoteSpawnData.hpp"
#include "GlobalNamespace/ObstacleData.hpp"
#include "GlobalNamespace/BeatmapObjectSpawnMovementData_ObstacleSpawnData.hpp"
#include "GlobalNamespace/NoteController.hpp"
#include "GlobalNamespace/ObstacleController.hpp"
#include "GlobalNamespace/BeatmapObjectManager.hpp"
#include "GlobalNamespace/ScoreController.hpp"
#include "GlobalNamespace/NoteCutInfo.hpp"
#include "GlobalNamespace/BeatmapObjectSpawnController.hpp"
#include "GlobalNamespace/ISaberSwingRatingCounterDidChangeReceiver.hpp"
#include "GlobalNamespace/ISaberSwingRatingCounterDidFinishReceiver.hpp"
#include "GlobalNamespace/ISaberSwingRatingCounter.hpp"
#include "GlobalNamespace/PlayerHeadAndObstacleInteraction.hpp"
#include "GlobalNamespace/PlayerTransforms.hpp"
#include "GlobalNamespace/PauseMenuManager.hpp"
#include "GlobalNamespace/SaberSwingRatingCounter.hpp"
#include "GlobalNamespace/IPlatformUserModel.hpp"
#include "GlobalNamespace/UserInfo.hpp"
#include "GlobalNamespace/PlatformLeaderboardsModel.hpp"
#include "GlobalNamespace/PlayerHeightDetector.hpp"

#include <map>
#include <chrono>
#include <iostream>
#include <sstream>

using namespace GlobalNamespace;
using UnityEngine::Resources;

ModInfo modInfo; // Stores the ID and version of our mod, and is sent to the modloader upon startup

static Replay* replay;

static MapEnhancer mapEnhancer;

PlatformLeaderboardsModel* pmodel;
AudioTimeSyncController* audioTimeSyncController;
PlayerSpecificSettings* playerSettings;
PlayerHeightDetector* playerHeightDetector;
PlayerHeadAndObstacleInteraction* phoi;

static map<int, NoteCutInfo> _cutInfoCache;
static map<int, NoteEvent *> _noteEventCache;

static map<NoteController *, int> _noteIdCache;
static map<ISaberSwingRatingCounter*, int> _swingIdCache;
static int _noteId;

static map<ObstacleController *, int> _wallCache;
static map<int, WallEvent *> _wallEventCache;
static int _wallId;

static Pause* _currentPause;
static WallEvent* _currentWallEvent;
static chrono::steady_clock::time_point _pauseStartTime;
static System::Action_1<float>* _heightEvent;

// Loads the config from disk using our modInfo, then returns it for use
Configuration& getConfig() {
    static Configuration config(modInfo);
    config.Load();
    return config;
}

// Returns a logger, useful for printing debug messages
Logger& getLogger() {
    static Logger* logger = new Logger(modInfo);
    return *logger;
}

// Called at the early stages of game loading
extern "C" void setup(ModInfo& info) {
    info.id = ID;
    info.version = VERSION;
    modInfo = info;
	
    getConfig().Load(); // Load the config file
    getLogger().info("Completed setup!");
}

void collectMapData(StandardLevelScenesTransitionSetupDataSO* self, ::StringW gameMode, IDifficultyBeatmap* difficultyBeatmap, IPreviewBeatmapLevel* previewBeatmapLevel, OverrideEnvironmentSettings* overrideEnvironmentSettings, ColorScheme* overrideColorScheme, GameplayModifiers* gameplayModifiers, PlayerSpecificSettings* playerSpecificSettings, PracticeSettings* practiceSettings, ::StringW backButtonText, bool useTestNoteCutSoundEffects) {
    EnvironmentInfoSO* environmentInfoSO = BeatmapEnvironmentHelper::GetEnvironmentInfo(difficultyBeatmap);
    if (overrideEnvironmentSettings->overrideEnvironments)
    {
        environmentInfoSO = overrideEnvironmentSettings->GetOverrideEnvironmentInfoForType(environmentInfoSO->environmentType);
    }
    mapEnhancer.difficultyBeatmap = difficultyBeatmap;
    mapEnhancer.previewBeatmapLevel = previewBeatmapLevel;
    mapEnhancer.gameplayModifiers = gameplayModifiers;
    mapEnhancer.playerSpecificSettings = playerSpecificSettings;
    mapEnhancer.practiceSettings = practiceSettings;
    mapEnhancer.useTestNoteCutSoundEffects = useTestNoteCutSoundEffects;
    mapEnhancer.environmentInfo = environmentInfoSO;
    mapEnhancer.colorScheme = overrideColorScheme;

    playerSettings = playerSpecificSettings;
}

MAKE_HOOK_MATCH(TransitionSetupDataInit, &StandardLevelScenesTransitionSetupDataSO::Init, void, StandardLevelScenesTransitionSetupDataSO* self, ::StringW gameMode, IDifficultyBeatmap* difficultyBeatmap, IPreviewBeatmapLevel* previewBeatmapLevel, OverrideEnvironmentSettings* overrideEnvironmentSettings, ColorScheme* overrideColorScheme, GameplayModifiers* gameplayModifiers, PlayerSpecificSettings* playerSpecificSettings, PracticeSettings* practiceSettings, ::StringW backButtonText, bool useTestNoteCutSoundEffects) {
    TransitionSetupDataInit(self, gameMode, difficultyBeatmap, previewBeatmapLevel, overrideEnvironmentSettings, overrideColorScheme, gameplayModifiers, playerSpecificSettings, practiceSettings, backButtonText, useTestNoteCutSoundEffects);
    collectMapData(self, gameMode, difficultyBeatmap, previewBeatmapLevel, overrideEnvironmentSettings, overrideColorScheme, gameplayModifiers, playerSpecificSettings, practiceSettings, backButtonText, useTestNoteCutSoundEffects);
}

void OnPlayerHeightChange(float height)
{
    AutomaticHeight* automaticHeight = new AutomaticHeight();
    automaticHeight->height = height;
    automaticHeight->time = audioTimeSyncController->get_songTime();

    replay->heights.push_back(automaticHeight);
}

void levelStarted() {
    replay = new Replay();

    replay->info->version = modInfo.version;
    replay->info->gameVersion = (string)UnityEngine::Application::get_version();

    std::stringstream strm;
    strm << std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    replay->info->timestamp = strm.str();

    IPlatformUserModel* userModel = NULL;
    ::ArrayW<PlatformLeaderboardsModel *> pmarray = Resources::FindObjectsOfTypeAll<PlatformLeaderboardsModel*>();
    for (size_t i = 0; i < pmarray.Length(); i++)
    {
        if (pmarray.get(i)->platformUserModel != NULL) {
            userModel = pmarray.get(i)->platformUserModel;
            break;
        }
    }

    if (userModel == NULL) { return; }

    auto userInfoTask = userModel->GetUserInfo();

    auto action = il2cpp_utils::MakeDelegate<System::Action_1<System::Threading::Tasks::Task*>*>(classof(System::Action_1<System::Threading::Tasks::Task*>*), (std::function<void(System::Threading::Tasks::Task_1<GlobalNamespace::UserInfo*>*)>)[&](System::Threading::Tasks::Task_1<GlobalNamespace::UserInfo*>* userInfoTask) {
            UserInfo *ui = userInfoTask->get_Result();
            if (ui != nullptr) {
                replay->info->playerName = (string)ui->userName;
                replay->info->playerID = (string)ui->platformUserId;
                replay->info->platform = "oculus";

                // ¯\_(ツ)_/¯
                replay->info->hmd = "Oculus Quest";
                replay->info->trackingSytem = "Oculus";
                replay->info->controller = "Oculus Touch";
            }
        }
    );

    reinterpret_cast<System::Threading::Tasks::Task*>(userInfoTask)->ContinueWith(action);

    playerHeightDetector = Resources::FindObjectsOfTypeAll<PlayerHeightDetector*>()[0];
    if (playerHeightDetector != NULL && playerSettings->get_automaticPlayerHeight()) {
        _heightEvent = il2cpp_utils::MakeDelegate<System::Action_1<float>*>(
            classof(System::Action_1<float>*), 
            static_cast<Il2CppObject*>(nullptr), OnPlayerHeightChange)
        playerHeightDetector->add_playerHeightDidChangeEvent(_heightEvent);
    }
}

MAKE_HOOK_MATCH(LevelPlay, &SinglePlayerLevelSelectionFlowCoordinator::StartLevel, void, SinglePlayerLevelSelectionFlowCoordinator* self, System::Action* beforeSceneSwitchCallback, bool practice) {
    LevelPlay(self, beforeSceneSwitchCallback, practice);

    levelStarted();
}

void processResults(SinglePlayerLevelSelectionFlowCoordinator* self, LevelCompletionResults* levelCompletionResults, IDifficultyBeatmap* difficultyBeatmap, bool practice) {
    replay->info->score = levelCompletionResults->rawScore;

    mapEnhancer.energy = levelCompletionResults->energy;
    mapEnhancer.Enhance(replay);

    switch (levelCompletionResults->levelEndStateType)
    {
        case LevelCompletionResults::LevelEndStateType::Cleared:
            ReplayManager::ProcessReplay(replay);
            break;
        case LevelCompletionResults::LevelEndStateType::Failed:
            if (levelCompletionResults->levelEndAction != LevelCompletionResults::LevelEndAction::Restart)
            {
                replay->info->failTime = audioTimeSyncController->get_songTime();
                ReplayManager::ProcessReplay(replay);
            }
            break;
    }

    if (playerHeightDetector != NULL && _heightEvent != NULL) {
        playerHeightDetector->remove_playerHeightDidChangeEvent(_heightEvent);
    }
}

MAKE_HOOK_MATCH(ProcessResultsSolo, &SoloFreePlayFlowCoordinator::ProcessLevelCompletionResultsAfterLevelDidFinish, void, SoloFreePlayFlowCoordinator* self, LevelCompletionResults* levelCompletionResults, IDifficultyBeatmap* difficultyBeatmap, GameplayModifiers* gameplayModifiers, bool practice) {
    ProcessResultsSolo(self, levelCompletionResults, difficultyBeatmap, gameplayModifiers, practice);
    processResults(self, levelCompletionResults, difficultyBeatmap, practice);
}

MAKE_HOOK_MATCH(SongStart, &AudioTimeSyncController::Start, void, AudioTimeSyncController* self) {
    SongStart(self);
    audioTimeSyncController = self;
}

void NoteSpawned(NoteController* noteController, NoteData* noteData) {
    _noteId++;
    _noteIdCache[noteController] = _noteId;

    NoteEvent* noteEvent = new NoteEvent();
    noteEvent->noteID = noteData->lineIndex * 1000 + (int)noteData->noteLineLayer * 100 + (int)noteData->colorType * 10 + (int)noteData->cutDirection;
    noteEvent->spawnTime = noteData->time;
    _noteEventCache[_noteId] = noteEvent;
}

MAKE_HOOK_MATCH(SpawnNote, &BeatmapObjectManager::SpawnBasicNote, NoteController*, BeatmapObjectManager* self, NoteData* noteData, BeatmapObjectSpawnMovementData::NoteSpawnData noteSpawnData, float rotation, float cutDirectionAngleOffset) {
    NoteController* noteController = SpawnNote(self, noteData, noteSpawnData, rotation, cutDirectionAngleOffset);

    NoteSpawned(noteController, noteData);

    return noteController;
}

MAKE_HOOK_MATCH(SpawnBombNote, &BeatmapObjectManager::SpawnBombNote, NoteController*, BeatmapObjectManager* self, NoteData* noteData, BeatmapObjectSpawnMovementData::NoteSpawnData noteSpawnData, float rotation) {
    NoteController* noteController = SpawnBombNote(self, noteData, noteSpawnData, rotation);

    NoteSpawned(noteController, noteData);

    return noteController;
}

MAKE_HOOK_MATCH(SpawnObstacle, &BeatmapObjectManager::SpawnObstacle, ObstacleController*, BeatmapObjectManager* self, ObstacleData* obstacleData, BeatmapObjectSpawnMovementData::ObstacleSpawnData obstacleSpawnData, float rotation) {
    ObstacleController* obstacleController = SpawnObstacle(self, obstacleData, obstacleSpawnData, rotation);
    int wallId = _wallId++;
    _wallCache[obstacleController] = wallId;

    WallEvent* wallEvent = new WallEvent();
    wallEvent->wallID = obstacleData->lineIndex * 100 + (int)obstacleData->obstacleType * 10 + obstacleData->width;
    wallEvent->spawnTime = audioTimeSyncController->get_songTime();
    _wallEventCache[wallId] = wallEvent;
    return obstacleController;
}

void PopulateNoteCutInfo(ReplayNoteCutInfo* noteCutInfo, ByRef<NoteCutInfo> cutInfo) {
    noteCutInfo->speedOK = cutInfo->speedOK;
    noteCutInfo->directionOK = cutInfo->directionOK;
    noteCutInfo->saberTypeOK = cutInfo->saberTypeOK;
    noteCutInfo->wasCutTooSoon = cutInfo->wasCutTooSoon;
    noteCutInfo->saberSpeed = cutInfo->saberSpeed;
    noteCutInfo->saberDir = new Vector3(cutInfo->saberDir);
    noteCutInfo->saberType = (int)cutInfo->saberType;
    noteCutInfo->timeDeviation = cutInfo->timeDeviation;
    noteCutInfo->cutDirDeviation = cutInfo->cutDirDeviation;
    noteCutInfo->cutPoint = new Vector3(cutInfo->cutPoint);
    noteCutInfo->cutNormal = new Vector3(cutInfo->cutNormal);
    noteCutInfo->cutDistanceToCenter = cutInfo->cutDistanceToCenter;
    noteCutInfo->cutAngle = cutInfo->cutAngle;
} 

MAKE_HOOK_MATCH(NoteCut, &ScoreController::HandleNoteWasCut, void, ScoreController* self, NoteController* noteController, ByRef<NoteCutInfo> noteCutInfo) {
    NoteCut(self, noteController, noteCutInfo);
    
    int noteId = _noteIdCache[noteController];

    NoteEvent* noteEvent = _noteEventCache[noteId];
    noteEvent->eventTime = audioTimeSyncController->get_songTime();

    if (noteController->noteData->colorType == ColorType::None)
    {
        noteEvent->eventType = NoteEventType::bomb;
    }

    replay->notes.push_back(noteEvent);

    _cutInfoCache[noteId] = *noteCutInfo;
    noteEvent->noteCutInfo = new ReplayNoteCutInfo();
    if (noteCutInfo->speedOK && noteCutInfo->directionOK && noteCutInfo->saberTypeOK && !noteCutInfo->wasCutTooSoon) {
        noteEvent->eventType = NoteEventType::good;
    } else {
        noteEvent->eventType = NoteEventType::bad;
        PopulateNoteCutInfo(noteEvent->noteCutInfo, noteCutInfo);
    }

    _swingIdCache[noteCutInfo->swingRatingCounter] = noteId;
}

MAKE_HOOK_MATCH(SwingRatingDidFinish, &SaberSwingRatingCounter::Finish, void, SaberSwingRatingCounter* self) {
    SwingRatingDidFinish(self);

    int noteId = _swingIdCache[(ISaberSwingRatingCounter*)self];
    NoteCutInfo cutInfo = _cutInfoCache[noteId];

    NoteEvent* cutEvent = _noteEventCache[noteId];
    ReplayNoteCutInfo* noteCutInfo = cutEvent->noteCutInfo;
    PopulateNoteCutInfo(noteCutInfo, cutInfo);
    noteCutInfo->beforeCutRating = self->get_beforeCutRating();
    noteCutInfo->afterCutRating = self->get_afterCutRating();
}

MAKE_HOOK_MATCH(NoteMiss, &ScoreController::HandleNoteWasMissed, void, ScoreController* self, NoteController* noteController) {
    NoteMiss(self, noteController);

    int noteId = _noteIdCache[noteController];

    if (noteController->noteData->colorType != ColorType::None)
    {
        NoteEvent* noteEvent = _noteEventCache[noteId];
        noteEvent->eventTime = audioTimeSyncController->get_songTime();
        noteEvent->eventType = NoteEventType::miss;
        replay->notes.push_back(noteEvent);
    }
}

MAKE_HOOK_MATCH(ComboMultiplierChanged, &ScoreController::NotifyForChange, void,  ScoreController* self, bool comboChanged, bool multiplierChanged) {
    ComboMultiplierChanged(self, comboChanged, multiplierChanged);

    if (comboChanged && self->playerHeadAndObstacleInteraction->get_intersectingObstacles()->get_Count() > 0) {
        WallEvent* wallEvent = _wallEventCache[_wallCache[self->playerHeadAndObstacleInteraction->get_intersectingObstacles()->get_Item(0)]];
        wallEvent->time = audioTimeSyncController->get_songTime();
        replay->walls.push_back(wallEvent);
        _currentWallEvent = wallEvent;
        phoi = self->playerHeadAndObstacleInteraction;
    }
}

MAKE_HOOK_MATCH(BeatMapStart, &BeatmapObjectSpawnController::Start, void, BeatmapObjectSpawnController* self) {
    BeatMapStart(self);

    replay->info->jumpDistance = self->get_jumpDistance();
}

MAKE_HOOK_MATCH(LevelPause, &PauseMenuManager::ShowMenu, void, PauseMenuManager* self) {
    LevelPause(self);

    _currentPause = new Pause();
    _currentPause->time = audioTimeSyncController->get_songTime();
    _pauseStartTime = chrono::steady_clock::now();
}

MAKE_HOOK_MATCH(LevelUnpause, &PauseMenuManager::HandleResumeFromPauseAnimationDidFinish, void, PauseMenuManager* self) {
    LevelUnpause(self);

    _currentPause->duration = (long)chrono::duration_cast<std::chrono::seconds>(chrono::steady_clock::now() - _pauseStartTime).count();
    replay->pauses.push_back(_currentPause);
    _currentPause = NULL;
}

MAKE_HOOK_MATCH(Tick, &PlayerTransforms::Update, void, PlayerTransforms* trans) {
    Tick(trans);

    if (audioTimeSyncController != NULL && _currentPause == NULL) {
        Frame* frame = new Frame();
        frame->time = audioTimeSyncController->get_songTime();
        frame->fps = 72;

        frame->head = new Transform();
        frame->head->rotation = new Quaternion(trans->get_headPseudoLocalRot());
        frame->head->position = new Vector3(trans->get_headPseudoLocalPos());

        frame->leftHand = new Transform();
        frame->leftHand->rotation = new Quaternion(trans->get_leftHandPseudoLocalRot());
        frame->leftHand->position = new Vector3(trans->get_leftHandPseudoLocalPos());
        
        frame->rightHand = new Transform();
        frame->rightHand->rotation = new Quaternion(trans->get_rightHandPseudoLocalRot());
        frame->rightHand->position = new Vector3(trans->get_rightHandPseudoLocalPos());
        
        replay->frames.push_back(frame);
    }

    if (_currentWallEvent != NULL) {
        if (phoi->get_intersectingObstacles()->get_Count() == 0)
        {
            _currentWallEvent->energy = audioTimeSyncController->get_songTime();
            _currentWallEvent = NULL;
        }
    }
}

// Called later on in the game loading - a good time to install function hooks
extern "C" void load() {
    il2cpp_functions::Init();

    LoggerContextObject logger = getLogger().WithContext("load");

    QuestUI::Init();
    QuestUI::Register::RegisterModSettingsViewController(modInfo, "BeatLeader", DidActivate);

    getLogger().info("Installing hooks...");
    INSTALL_HOOK(logger, ProcessResultsSolo);
    INSTALL_HOOK(logger, TransitionSetupDataInit);
    INSTALL_HOOK(logger, LevelPlay);
    INSTALL_HOOK(logger, SongStart);
    INSTALL_HOOK(logger, SpawnNote);
    INSTALL_HOOK(logger, NoteCut);
    INSTALL_HOOK(logger, NoteMiss);
    INSTALL_HOOK(logger, ComboMultiplierChanged);
    INSTALL_HOOK(logger, BeatMapStart);
    INSTALL_HOOK(logger, LevelPause);
    INSTALL_HOOK(logger, LevelUnpause);
    INSTALL_HOOK(logger, Tick);
    INSTALL_HOOK(logger, SwingRatingDidFinish);
    // Install our hooks (none defined yet)
    getLogger().info("Installed all hooks!");
}