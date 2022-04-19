
#include "include/Core/ReplayRecorder.hpp"

#include "System/Action_1.hpp"
#include "System/Threading/Tasks/Task.hpp"
#include "System/Threading/Tasks/Task_1.hpp"
#include "System/Collections/Generic/HashSet_1.hpp"

#include "include/Models/Replay.hpp"

#include "include/Enhancers/MapEnhancer.hpp"
#include "include/Enhancers/UserEnhancer.hpp"

#include "include/Utils/ReplayManager.hpp"

#include "beatsaber-hook/shared/utils/hooking.hpp"

#include "config-utils/shared/config-utils.hpp"
#include "custom-types/shared/register.hpp"

#include "UnityEngine/Application.hpp"
#include "UnityEngine/Resources.hpp"
#include "UnityEngine/Time.hpp"

#include "GlobalNamespace/NoteController.hpp"
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
#include "GlobalNamespace/CutScoreBuffer.hpp"
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
#include "GlobalNamespace/PlayerHeightDetector.hpp"
#include "GlobalNamespace/BeatmapDifficulty.hpp"
#include "GlobalNamespace/IBeatmapLevel.hpp"
#include "GlobalNamespace/IBeatmapLevelData.hpp"
#include "GlobalNamespace/IDifficultyBeatmapSet.hpp"
#include "GlobalNamespace/IReadonlyBeatmapData.hpp"
#include "GlobalNamespace/BeatmapCharacteristicSO.hpp"
#include "GlobalNamespace/IPreviewBeatmapLevel.hpp"
#include "GlobalNamespace/SaberMovementData.hpp"
#include "GlobalNamespace/SaberSwingRating.hpp"

#include "main.hpp"

#include <map>
#include <chrono>
#include <iostream>
#include <sstream>
#include <regex>

using namespace GlobalNamespace;
using UnityEngine::Resources;

namespace ReplayRecorder {

    Replay* replay;
    std::function<void(Replay*, MapStatus, bool)> replayCallback;

    MapEnhancer mapEnhancer;
    UserEnhancer userEnhancer;

    AudioTimeSyncController* audioTimeSyncController;
    PlayerSpecificSettings* playerSettings;
    PlayerHeightDetector* playerHeightDetector;
    PlayerHeadAndObstacleInteraction* phoi;

    map<int, NoteCutInfo> _cutInfoCache;
    map<int, NoteEvent *> _noteEventCache;
    map<SaberMovementData *, float> _preSwingContainer;
    map<SaberSwingRatingCounter *, float> _postSwingContainer;

    map<NoteController *, int> _noteIdCache;
    map<ISaberSwingRatingCounter*, int> _swingIdCache;
    int _noteId;

    map<ObstacleController *, int> _wallCache;
    map<int, WallEvent *> _wallEventCache;
    int _wallId;

    Pause* _currentPause;
    WallEvent* _currentWallEvent;
    chrono::steady_clock::time_point _pauseStartTime;
    System::Action_1<float>* _heightEvent;

    bool isOst = false;
    void collectMapData(StandardLevelScenesTransitionSetupDataSO* self, ::StringW gameMode, IDifficultyBeatmap* difficultyBeatmap, IPreviewBeatmapLevel* previewBeatmapLevel, OverrideEnvironmentSettings* overrideEnvironmentSettings, ColorScheme* overrideColorScheme, GameplayModifiers* gameplayModifiers, PlayerSpecificSettings* playerSpecificSettings, PracticeSettings* practiceSettings, ::StringW backButtonText, bool useTestNoteCutSoundEffects) {
        EnvironmentInfoSO* environmentInfoSO = BeatmapEnvironmentHelper::GetEnvironmentInfo(difficultyBeatmap);
        if (overrideEnvironmentSettings != NULL && environmentInfoSO != NULL && overrideEnvironmentSettings->overrideEnvironments)
        {
            environmentInfoSO = overrideEnvironmentSettings->GetOverrideEnvironmentInfoForType(environmentInfoSO->environmentType);
        }
        isOst = !previewBeatmapLevel->get_levelID().starts_with("custom_level");

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

    MAKE_HOOK_MATCH(LevelPlay, &SinglePlayerLevelSelectionFlowCoordinator::StartLevel, void, SinglePlayerLevelSelectionFlowCoordinator* self, System::Action* beforeSceneSwitchCallback, bool practice) {
        LevelPlay(self, beforeSceneSwitchCallback, practice);

        _currentPause = NULL;
        replay = new Replay();
        replay->info->version = modInfo.version;
        replay->info->gameVersion = (string)UnityEngine::Application::get_version();

        std::stringstream strm;
        strm << std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        replay->info->timestamp = strm.str();
        userEnhancer.Enhance(replay);

        playerHeightDetector = Resources::FindObjectsOfTypeAll<PlayerHeightDetector*>()[0];
        if (playerHeightDetector != NULL && playerSettings->get_automaticPlayerHeight()) {
            _heightEvent = il2cpp_utils::MakeDelegate<System::Action_1<float>*>(
                classof(System::Action_1<float>*), 
                static_cast<Il2CppObject*>(nullptr), OnPlayerHeightChange);
            playerHeightDetector->add_playerHeightDidChangeEvent(_heightEvent);
        }
    }

    void processResults(SinglePlayerLevelSelectionFlowCoordinator* self, LevelCompletionResults* levelCompletionResults, IDifficultyBeatmap* difficultyBeatmap, bool practice) {
        replay->info->score = levelCompletionResults->rawScore;

        mapEnhancer.energy = levelCompletionResults->energy;
        mapEnhancer.Enhance(replay);
        
        switch (levelCompletionResults->levelEndStateType)
        {
            case LevelCompletionResults::LevelEndStateType::Cleared:
                replayCallback(replay, MapStatus::cleared, isOst);
                
                break;
            case LevelCompletionResults::LevelEndStateType::Failed:
                if (levelCompletionResults->levelEndAction != LevelCompletionResults::LevelEndAction::Restart)
                {
                    replay->info->failTime = audioTimeSyncController->get_songTime();
                    replayCallback(replay, MapStatus::failed, isOst);
                }
                break;
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

    MAKE_HOOK_MATCH(ComputeSwingRating, static_cast<float (SaberMovementData::*)(bool, float)>(&SaberMovementData::ComputeSwingRating), float, SaberMovementData* self, bool overrideSegmenAngle, float overrideValue) {
        float result = ComputeSwingRating(self, overrideSegmenAngle, overrideValue);
        auto _data = self->data;
        int _nextAddIndex = self->nextAddIndex;
        int _validCount = self->validCount;

        int length = _data.Length();

        int index = _nextAddIndex - 1;
        if (index < 0) index += length;

        float startTime = _data[index].time;
        float time = startTime;

        UnityEngine::Vector3 segmentNormal1 = _data[index].segmentNormal;
        float angleDiff = overrideSegmenAngle ? overrideValue : _data[index].segmentAngle;
        float swingRating = SaberSwingRating::BeforeCutStepRating(angleDiff, 0.0f);
        for (int i = 2; (double)startTime - (double)time < 0.4 && i < _validCount; ++i)
        {
            --index;
            if (index < 0) index += length;

            UnityEngine::Vector3 segmentNormal2 = _data[index].segmentNormal;
            float segmentAngle = _data[index].segmentAngle;

            float normalDiff = UnityEngine::Vector3::Angle(segmentNormal2, segmentNormal1);
            if ((double)normalDiff <= 90.0)
            {
                swingRating += SaberSwingRating::BeforeCutStepRating(segmentAngle, normalDiff);
                time = _data[index].time;
            }
            else {
                break;
            }
        }

        _preSwingContainer[self] = swingRating;
        return result;
    }

    MAKE_HOOK_MATCH(ProcessNewSwingData, &SaberSwingRatingCounter::ProcessNewData, void, SaberSwingRatingCounter* self, BladeMovementDataElement newData, BladeMovementDataElement prevData, bool prevDataAreValid) {
        bool alreadyCut = self->notePlaneWasCut;
        ProcessNewSwingData(self, newData, prevData, prevDataAreValid);

        float postSwing = _postSwingContainer[self];
        if (!alreadyCut && !self->notePlane.SameSide(newData.topPos, prevData.topPos))
        {
            float angleDiff = UnityEngine::Vector3::Angle(self->cutTopPos - self->cutBottomPos, self->afterCutTopPos - self->afterCutBottomPos);

            if (self->rateAfterCut)
            {
                postSwing = SaberSwingRating::AfterCutStepRating(angleDiff, 0.0f);
            }
        }
        else
        {
            float normalDiff = UnityEngine::Vector3::Angle(newData.segmentNormal, self->cutPlaneNormal);
            if (self->rateAfterCut)
            {
                postSwing += SaberSwingRating::AfterCutStepRating(newData.segmentAngle, normalDiff);
            }
        }

        _postSwingContainer[self] = postSwing;
    }

    MAKE_HOOK_MATCH(SwingRatingDidFinish, &SaberSwingRatingCounter::Finish, void, SaberSwingRatingCounter* self) {
        SwingRatingDidFinish(self);

        int noteId = _swingIdCache[(ISaberSwingRatingCounter*)self];
        NoteCutInfo cutInfo = _cutInfoCache[noteId];

        NoteEvent* cutEvent = _noteEventCache[noteId];
        ReplayNoteCutInfo* noteCutInfo = cutEvent->noteCutInfo;
        PopulateNoteCutInfo(noteCutInfo, cutInfo);

        noteCutInfo->beforeCutRating = _preSwingContainer[(SaberMovementData *)self->saberMovementData];
        noteCutInfo->afterCutRating = _postSwingContainer[self];
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

        if(replay) {
            replay->info->jumpDistance = self->get_jumpDistance();
            _currentPause = NULL;
            _currentWallEvent = NULL;
        }
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
        getLogger().info("current pause is now null");
    }

    MAKE_HOOK_MATCH(Tick, &PlayerTransforms::Update, void, PlayerTransforms* trans) {
        Tick(trans);
        if (audioTimeSyncController != NULL && _currentPause == NULL && replay) {

            Frame* frame = new Frame();
            frame->time = audioTimeSyncController->get_songTime();
            frame->fps = 1.0f / UnityEngine::Time::get_deltaTime();

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

    void StartRecording(std::function<void(Replay*, MapStatus, bool)> callback) {
        LoggerContextObject logger = getLogger().WithContext("load");

        getLogger().info("Installing ReplayRecorder hooks...");

        INSTALL_HOOK(logger, ProcessResultsSolo);
        INSTALL_HOOK(logger, TransitionSetupDataInit);
        INSTALL_HOOK(logger, LevelPlay);
        INSTALL_HOOK(logger, SongStart);
        INSTALL_HOOK(logger, SpawnNote);
        INSTALL_HOOK(logger, SpawnBombNote);
        INSTALL_HOOK(logger, SpawnObstacle);
        INSTALL_HOOK(logger, NoteCut);
        INSTALL_HOOK(logger, NoteMiss);
        INSTALL_HOOK(logger, ComboMultiplierChanged);
        INSTALL_HOOK(logger, BeatMapStart);
        INSTALL_HOOK(logger, LevelPause);
        INSTALL_HOOK(logger, LevelUnpause);
        INSTALL_HOOK(logger, Tick);
        INSTALL_HOOK(logger, SwingRatingDidFinish);
        INSTALL_HOOK(logger, ComputeSwingRating);
        INSTALL_HOOK(logger, ProcessNewSwingData);

        getLogger().info("Installed all ReplayRecorder hooks!");

        replayCallback = callback;
    }
}