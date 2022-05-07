
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

    std::optional<Replay> replay;
    std::function<void(Replay const&, MapStatus, bool)> replayCallback;

    MapEnhancer mapEnhancer;
    UserEnhancer userEnhancer;

    AudioTimeSyncController* audioTimeSyncController;
    PlayerSpecificSettings* playerSettings;
    PlayerHeightDetector* playerHeightDetector;
    PlayerHeadAndObstacleInteraction* phoi;

    map<int, NoteCutInfo> _cutInfoCache;
    map<int, NoteEvent> _noteEventCache;
    map<SaberMovementData *, float> _preSwingContainer;
    map<SaberSwingRatingCounter *, float> _postSwingContainer;

    static map<NoteData *, int> _noteIdCache;
    map<ISaberSwingRatingCounter*, int> _swingIdCache;
    int _noteId;

    map<ObstacleController *, int> _wallCache;
    map<int, WallEvent> _wallEventCache;
    int _wallId;

    std::optional<Pause> _currentPause;
    std::optional<WallEvent> _currentWallEvent;
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

    MAKE_HOOK_MATCH(TransitionSetupDataInit, &StandardLevelScenesTransitionSetupDataSO::Init, void, StandardLevelScenesTransitionSetupDataSO* self, ::StringW gameMode, IDifficultyBeatmap* difficultyBeatmap, IPreviewBeatmapLevel* previewBeatmapLevel, OverrideEnvironmentSettings* overrideEnvironmentSettings, ColorScheme* overrideColorScheme, GameplayModifiers* gameplayModifiers, PlayerSpecificSettings* playerSpecificSettings, PracticeSettings* practiceSettings, ::StringW backButtonText, bool useTestNoteCutSoundEffects, bool startPaused) {
        TransitionSetupDataInit(self, gameMode, difficultyBeatmap, previewBeatmapLevel, overrideEnvironmentSettings, overrideColorScheme, gameplayModifiers, playerSpecificSettings, practiceSettings, backButtonText, useTestNoteCutSoundEffects, startPaused);
        collectMapData(self, gameMode, difficultyBeatmap, previewBeatmapLevel, overrideEnvironmentSettings, overrideColorScheme, gameplayModifiers, playerSpecificSettings, practiceSettings, backButtonText, useTestNoteCutSoundEffects);
    }

    void OnPlayerHeightChange(float height)
    {
        replay->heights.emplace_back(height, audioTimeSyncController->songTime);
    }

    MAKE_HOOK_MATCH(LevelPlay, &SinglePlayerLevelSelectionFlowCoordinator::StartLevel, void, SinglePlayerLevelSelectionFlowCoordinator* self, System::Action* beforeSceneSwitchCallback, bool practice) {
        LevelPlay(self, beforeSceneSwitchCallback, practice);



        std::string timeStamp(std::to_string(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count()));

        _currentPause = std::nullopt;
        replay.emplace(Replay(ReplayInfo(modInfo.version,(string)UnityEngine::Application::get_version(), timeStamp)));

        userEnhancer.Enhance(replay.value());

        playerHeightDetector = Resources::FindObjectsOfTypeAll<PlayerHeightDetector*>()[0];
        if (playerHeightDetector != NULL && playerSettings->get_automaticPlayerHeight()) {
            _heightEvent = il2cpp_utils::MakeDelegate<System::Action_1<float>*>(
                classof(System::Action_1<float>*), 
                static_cast<Il2CppObject*>(nullptr), OnPlayerHeightChange);
            playerHeightDetector->add_playerHeightDidChangeEvent(_heightEvent);
        }
    }

    void processResults(SinglePlayerLevelSelectionFlowCoordinator* self, LevelCompletionResults* levelCompletionResults, IDifficultyBeatmap* difficultyBeatmap, bool practice) {
        if (self->get_gameMode() == "Party") return;

        replay->info.score = levelCompletionResults->multipliedScore;

        mapEnhancer.energy = levelCompletionResults->energy;
        mapEnhancer.Enhance(replay.value());
        
        switch (levelCompletionResults->levelEndStateType)
        {
            case LevelCompletionResults::LevelEndStateType::Cleared:
                replayCallback(*replay, MapStatus::cleared, isOst);
                break;
            case LevelCompletionResults::LevelEndStateType::Failed:
                if (levelCompletionResults->levelEndAction != LevelCompletionResults::LevelEndAction::Restart)
                {
                    replay->info.failTime = audioTimeSyncController->songTime;
                    replayCallback(*replay, MapStatus::failed, isOst);
                }
                break;
        }
    }

    MAKE_HOOK_MATCH(ProcessResultsSolo, &SoloFreePlayFlowCoordinator::ProcessLevelCompletionResultsAfterLevelDidFinish, void, SoloFreePlayFlowCoordinator* self, LevelCompletionResults* levelCompletionResults, IReadonlyBeatmapData* transformedBeatmapData, IDifficultyBeatmap* difficultyBeatmap, GameplayModifiers* gameplayModifiers, bool practice) {
        ProcessResultsSolo(self, levelCompletionResults, transformedBeatmapData, difficultyBeatmap, gameplayModifiers, practice);
        processResults(self, levelCompletionResults, difficultyBeatmap, practice);
    }

    MAKE_HOOK_MATCH(SongStart, &AudioTimeSyncController::Start, void, AudioTimeSyncController* self) {
        SongStart(self);
        audioTimeSyncController = self;
    }

    void NoteSpawned(NoteController* noteController, NoteData* noteData) {
        _noteId++;
        _noteIdCache[noteData] = _noteId;


        auto noteID = noteData->lineIndex * 1000 + (int)noteData->noteLineLayer * 100 + (int)noteData->colorType * 10 + (int)noteData->cutDirection;
        auto spawnTime = noteData->time;
        _noteEventCache.emplace(_noteId, NoteEvent(noteID, spawnTime));
    }

    MAKE_HOOK_MATCH(SpawnNote, &BeatmapObjectManager::AddSpawnedNoteController, void, BeatmapObjectManager* self, NoteController* noteController, BeatmapObjectSpawnMovementData::NoteSpawnData noteSpawnData, float rotation) {
        SpawnNote(self, noteController, noteSpawnData, rotation);
        NoteSpawned(noteController, noteController->noteData);
    }

    MAKE_HOOK_MATCH(SpawnObstacle, &BeatmapObjectManager::AddSpawnedObstacleController, void, BeatmapObjectManager* self, ObstacleController* obstacleController, BeatmapObjectSpawnMovementData::ObstacleSpawnData obstacleSpawnData, float rotation) {
        SpawnObstacle(self, obstacleController, obstacleSpawnData, rotation);
        int wallId = _wallId++;
        _wallCache[obstacleController] = wallId;

        auto wallID = obstacleController->obstacleData->lineIndex * 100 + (int)obstacleController->obstacleData->type * 10 + obstacleController->obstacleData->width;
        auto spawnTime = audioTimeSyncController->songTime;

        _wallEventCache.emplace(wallId, WallEvent(wallID, spawnTime));
    }

    constexpr void PopulateNoteCutInfo(ReplayNoteCutInfo& noteCutInfo, NoteCutInfo const& cutInfo) {
        noteCutInfo.speedOK = cutInfo.speedOK;
        noteCutInfo.directionOK = cutInfo.directionOK;
        noteCutInfo.saberTypeOK = cutInfo.saberTypeOK;
        noteCutInfo.wasCutTooSoon = cutInfo.wasCutTooSoon;
        noteCutInfo.saberSpeed = cutInfo.saberSpeed;
        noteCutInfo.saberDir = cutInfo.saberDir;
        noteCutInfo.saberType = (int)cutInfo.saberType;
        noteCutInfo.timeDeviation = cutInfo.timeDeviation;
        noteCutInfo.cutDirDeviation = cutInfo.cutDirDeviation;
        noteCutInfo.cutPoint = cutInfo.cutPoint;
        noteCutInfo.cutNormal = cutInfo.cutNormal;
        noteCutInfo.cutDistanceToCenter = cutInfo.cutDistanceToCenter;
        noteCutInfo.cutAngle = cutInfo.cutAngle;
    }

    MAKE_HOOK_MATCH(NoteCut, &ScoreController::HandleNoteWasCut, void, ScoreController* self, NoteController* noteController, ByRef<NoteCutInfo> noteCutInfo) {
        NoteCut(self, noteController, noteCutInfo);
        int noteId = _noteIdCache[noteCutInfo->noteData];

        NoteEvent& noteEvent = _noteEventCache.at(noteId);
        noteEvent.eventTime = audioTimeSyncController->songTime;

        if (noteController->noteData->colorType == ColorType::None)
        {
            noteEvent.eventType = NoteEventType::BOMB;
        }

        replay->notes.emplace_back(noteEvent);

        _cutInfoCache[noteId] = *noteCutInfo;
        noteEvent.noteCutInfo = ReplayNoteCutInfo();
        if (noteCutInfo->speedOK && noteCutInfo->directionOK && noteCutInfo->saberTypeOK && !noteCutInfo->wasCutTooSoon) {
            noteEvent.eventType = NoteEventType::GOOD;
        } else {
            noteEvent.eventType = NoteEventType::BAD;
            PopulateNoteCutInfo(noteEvent.noteCutInfo, noteCutInfo.heldRef);
        }
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

    MAKE_HOOK_MATCH(SwingRatingDidFinish, &CutScoreBuffer::HandleSaberSwingRatingCounterDidFinish, void, CutScoreBuffer* self, ISaberSwingRatingCounter* swingRatingCounter) {
        SwingRatingDidFinish(self, swingRatingCounter);
        int noteId = _noteIdCache[self->noteCutInfo.noteData];
        NoteCutInfo const& cutInfo = self->noteCutInfo;

        NoteEvent& cutEvent = _noteEventCache.at(noteId);
        ReplayNoteCutInfo& noteCutInfo = cutEvent.noteCutInfo;
        PopulateNoteCutInfo(noteCutInfo, cutInfo);
        noteCutInfo.beforeCutRating = _preSwingContainer[(SaberMovementData *)self->saberSwingRatingCounter->saberMovementData];
        noteCutInfo.afterCutRating = _postSwingContainer[self->saberSwingRatingCounter];
    }

    MAKE_HOOK_MATCH(NoteMiss, &ScoreController::HandleNoteWasMissed, void, ScoreController* self, NoteController* noteController) {
        NoteMiss(self, noteController);
        int noteId = _noteIdCache[noteController->noteData];

        if (noteController->noteData->colorType != ColorType::None)
        {
            NoteEvent& noteEvent = _noteEventCache.at(noteId);
            noteEvent.eventTime = audioTimeSyncController->songTime;
            noteEvent.eventType = NoteEventType::MISS;
            replay->notes.emplace_back(noteEvent);
        }
    }

    MAKE_HOOK_MATCH(ComboMultiplierChanged, &ScoreController::HandlePlayerHeadDidEnterObstacles, void,  ScoreController* self) {
        ComboMultiplierChanged(self);
        if (self->scoreMultiplierCounter->ProcessMultiplierEvent(ScoreMultiplierCounter::MultiplierEventType::Negative) && self->playerHeadAndObstacleInteraction->intersectingObstacles->get_Count() > 0) {
            auto obstacleEnumerator = self->playerHeadAndObstacleInteraction->intersectingObstacles->GetEnumerator();
            if(obstacleEnumerator.MoveNext()) {
                WallEvent& wallEvent = _wallEventCache.at(_wallCache[reinterpret_cast<ObstacleController*>(obstacleEnumerator.current)]);
                wallEvent.time = audioTimeSyncController->songTime;
                replay->walls.emplace_back(wallEvent);
                _currentWallEvent = wallEvent;
                phoi = self->playerHeadAndObstacleInteraction;
            }
        }
    }

    MAKE_HOOK_MATCH(BeatMapStart, &BeatmapObjectSpawnController::Start, void, BeatmapObjectSpawnController* self) {
        BeatMapStart(self);

        if(replay) {
            replay->info.jumpDistance = self->get_jumpDistance();
            _currentPause = std::nullopt;
            _currentWallEvent = std::nullopt;
        }
    }

    MAKE_HOOK_MATCH(LevelPause, &PauseMenuManager::ShowMenu, void, PauseMenuManager* self) {
        LevelPause(self);

        _currentPause = Pause();
        _currentPause->time = audioTimeSyncController->songTime;
        _pauseStartTime = chrono::steady_clock::now();
    }

    MAKE_HOOK_MATCH(LevelUnpause, &PauseMenuManager::HandleResumeFromPauseAnimationDidFinish, void, PauseMenuManager* self) {
        LevelUnpause(self);

        _currentPause->duration = (long)chrono::duration_cast<std::chrono::seconds>(chrono::steady_clock::now() - _pauseStartTime).count();
        replay->pauses.emplace_back(_currentPause.value());
        _currentPause = std::nullopt;
        getLogger().info("current pause is now null");
    }

    MAKE_HOOK_MATCH(Tick, &PlayerTransforms::Update, void, PlayerTransforms* trans) {
        Tick(trans);
        if (audioTimeSyncController != nullptr && _currentPause == std::nullopt && replay) {
            
            auto time = audioTimeSyncController->songTime;
            auto fps = 1.0f / UnityEngine::Time::get_deltaTime();
            
            auto head = Transform(trans->get_headPseudoLocalPos(), trans->get_headPseudoLocalRot());
            auto leftHand = Transform(trans->get_leftHandPseudoLocalPos(), trans->get_leftHandPseudoLocalRot());
            auto rightHand = Transform(trans->get_rightHandPseudoLocalPos(), trans->get_rightHandPseudoLocalRot());
            
            replay->frames.emplace_back(time, fps, head, leftHand, rightHand);
        }

        if (_currentWallEvent != std::nullopt) {
            if (phoi->intersectingObstacles->get_Count() == 0)
            {
                _currentWallEvent->energy = audioTimeSyncController->songTime;
                _currentWallEvent = std::nullopt;
            }
        }
    }

    void StartRecording(function<void(Replay const &, MapStatus, bool)> const &callback) {
        LoggerContextObject logger = getLogger().WithContext("load");

        getLogger().info("Installing ReplayRecorder hooks...");

        INSTALL_HOOK(logger, ProcessResultsSolo);
        INSTALL_HOOK(logger, TransitionSetupDataInit);
        INSTALL_HOOK(logger, LevelPlay);
        INSTALL_HOOK(logger, SongStart);
        INSTALL_HOOK(logger, SpawnNote);
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