
#include "include/Core/ReplayRecorder.hpp"

#include "System/Action_1.hpp"
#include "System/Threading/Tasks/Task.hpp"
#include "System/Threading/Tasks/Task_1.hpp"
#include "System/Collections/Generic/HashSet_1.hpp"

#include "include/Models/Replay.hpp"

#include "include/Enhancers/MapEnhancer.hpp"
#include "include/Enhancers/UserEnhancer.hpp"

#include "include/Utils/ReplayManager.hpp"
#include "include/Utils/RecorderUtils.hpp"

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
#include "GlobalNamespace/GameplayCoreSceneSetupData.hpp"

#include "main.hpp"

#include <map>
#include <chrono>
#include <iostream>
#include <sstream>
#include <regex>

using namespace GlobalNamespace;
using UnityEngine::Resources;

namespace ReplayRecorder {

    optional<Replay> replay;
    std::function<void(Replay const&, MapStatus, bool)> replayCallback;

    MapEnhancer mapEnhancer;
    UserEnhancer userEnhancer;

    AudioTimeSyncController* audioTimeSyncController;
    bool automaticPlayerHeight = false;
    PlayerHeadAndObstacleInteraction* phoi;

    map<int, NoteCutInfo> _cutInfoCache;
    map<int, NoteEvent> _noteEventCache;
    map<SaberMovementData *, float> _preSwingContainer;
    map<SaberSwingRatingCounter *, float> _postSwingContainer;

    map<NoteController *, int> _noteIdCache;
    map<ISaberSwingRatingCounter*, int> _swingIdCache;
    int _noteId;

    map<ObstacleController *, int> _wallCache;
    map<int, WallEvent> _wallEventCache;
    int _wallId;

    optional<Pause> _currentPause;
    optional<WallEvent> _currentWallEvent;
    chrono::steady_clock::time_point _pauseStartTime;
    System::Action_1<float>* _heightEvent;

    bool isOst = false;

    void collectMapData(StandardLevelScenesTransitionSetupDataSO* self) {
        GameplayCoreSceneSetupData* gameplayCoreSceneSetupData = reinterpret_cast<GameplayCoreSceneSetupData*>(self->sceneSetupDataArray.get(2));

        isOst = !gameplayCoreSceneSetupData->previewBeatmapLevel->get_levelID().starts_with("custom_level");

        mapEnhancer.difficultyBeatmap = self->difficultyBeatmap;
        mapEnhancer.previewBeatmapLevel = gameplayCoreSceneSetupData->previewBeatmapLevel;
        mapEnhancer.gameplayModifiers = gameplayCoreSceneSetupData->gameplayModifiers;
        mapEnhancer.playerSpecificSettings = gameplayCoreSceneSetupData->playerSpecificSettings;
        mapEnhancer.practiceSettings = self->practiceSettings;
        mapEnhancer.environmentInfo = self->environmentInfo;
        mapEnhancer.colorScheme = self->colorScheme;
    }

    void OnPlayerHeightChange(float height)
    {
        if (audioTimeSyncController && automaticPlayerHeight && replay != nullopt) {
            replay->heights.emplace_back(height, audioTimeSyncController->songTime);
        }
    }

    MAKE_HOOK_MATCH(LevelPlay, &SinglePlayerLevelSelectionFlowCoordinator::StartLevel, void, SinglePlayerLevelSelectionFlowCoordinator* self, System::Action* beforeSceneSwitchCallback, bool practice) {
        LevelPlay(self, beforeSceneSwitchCallback, practice);

        std::string timeStamp(std::to_string(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count()));

        recording = true;
        _currentPause = nullopt;
        replay.emplace(ReplayInfo(modInfo.version, UnityEngine::Application::get_version(), timeStamp));

        userEnhancer.Enhance(replay.value());
        automaticPlayerHeight = self->get_playerSettings()->automaticPlayerHeight;
    }

    MAKE_HOOK_MATCH(PlayerHeightDetectorStart, &PlayerHeightDetector::Start, void, PlayerHeightDetector* self) {
        PlayerHeightDetectorStart(self);

        _heightEvent = il2cpp_utils::MakeDelegate<System::Action_1<float> *>(
                        classof(System::Action_1<float>*),
                        static_cast<Il2CppObject *>(nullptr), OnPlayerHeightChange);
        self->add_playerHeightDidChangeEvent(_heightEvent);
    }

    void processResults(LevelCompletionResults* levelCompletionResults) {
        replay->info.score = levelCompletionResults->rawScore;

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

    MAKE_HOOK_MATCH(ProcessResultsSolo, &StandardLevelScenesTransitionSetupDataSO::Finish, void, StandardLevelScenesTransitionSetupDataSO* self, LevelCompletionResults* levelCompletionResults) {
        ProcessResultsSolo(self, levelCompletionResults);
        recording = false;
        if (self->gameMode != "Party" && replay != nullopt) {
            collectMapData(self);
            processResults(levelCompletionResults);
        }
    }

    MAKE_HOOK_MATCH(SongStart, &AudioTimeSyncController::Start, void, AudioTimeSyncController* self) {
        SongStart(self);
        audioTimeSyncController = self;
    }

    void NoteSpawned(NoteController* noteController, NoteData* noteData) {
        _noteId++;
        _noteIdCache[noteController] = _noteId;

        int colorType = (int)noteData->colorType;
        if (colorType < 0) {
            colorType = 3;
        }
        auto noteID = noteData->lineIndex * 1000 + (int)noteData->noteLineLayer * 100 + colorType * 10 + (int)noteData->cutDirection;
        auto spawnTime = noteData->time;
        _noteEventCache.emplace(_noteId, NoteEvent(noteID, spawnTime));
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

        auto wallID = obstacleController->obstacleData->lineIndex * 100 + (int)obstacleController->obstacleData->obstacleType * 10 + obstacleController->obstacleData->width;
        auto spawnTime = audioTimeSyncController->songTime;

        _wallEventCache.emplace(wallId, WallEvent(wallID, spawnTime));
        return obstacleController;
    }

    void PopulateNoteCutInfo(ReplayNoteCutInfo& noteCutInfo, NoteCutInfo const& cutInfo) {
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
        
        int noteId = _noteIdCache[noteController];

        NoteEvent& noteEvent = _noteEventCache.at(noteId);
        noteEvent.eventTime = audioTimeSyncController->songTime;

        noteEvent.noteCutInfo = ReplayNoteCutInfo();
        if (noteCutInfo->speedOK && noteCutInfo->directionOK && noteCutInfo->saberTypeOK && !noteCutInfo->wasCutTooSoon) {
            noteEvent.eventType = NoteEventType::GOOD;
        } else if (replay != nullopt) {
            noteEvent.eventType = noteController->noteData->colorType == ColorType::None ? noteEvent.eventType = NoteEventType::BOMB : NoteEventType::BAD;
            PopulateNoteCutInfo(noteEvent.noteCutInfo, noteCutInfo.heldRef);
        }

        replay->notes.emplace_back(noteEvent);

        _swingIdCache[noteCutInfo->swingRatingCounter] = replay->notes.size() - 1;
        _cutInfoCache[replay->notes.size() - 1] = *noteCutInfo;
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

    static float ChooseSwingRating(float real, float unclamped) {
        return real < 1 ? real : max(real, unclamped);
    }

    MAKE_HOOK_MATCH(SwingRatingDidFinish, &SaberSwingRatingCounter::Finish, void, SaberSwingRatingCounter* self) {
        SwingRatingDidFinish(self);

        int noteId = _swingIdCache[(ISaberSwingRatingCounter*)self];
        
        if (replay != nullopt) {
            NoteEvent& cutEvent = replay->notes.at(noteId);
            NoteCutInfo const& cutInfo = _cutInfoCache[noteId];
            ReplayNoteCutInfo& noteCutInfo = cutEvent.noteCutInfo;
            PopulateNoteCutInfo(noteCutInfo, cutInfo);

            noteCutInfo.beforeCutRating = _preSwingContainer[(SaberMovementData *)self->saberMovementData];
            noteCutInfo.afterCutRating = _postSwingContainer[self];

            noteCutInfo.beforeCutRating = ChooseSwingRating(self->beforeCutRating, _preSwingContainer[(SaberMovementData *)self->saberMovementData]);
            noteCutInfo.afterCutRating = ChooseSwingRating(self->afterCutRating, _postSwingContainer[self]);

            _preSwingContainer[(SaberMovementData *)self->saberMovementData] = 0;
            _postSwingContainer[self] = 0;
        }
    }

    MAKE_HOOK_MATCH(NoteMiss, &ScoreController::HandleNoteWasMissed, void, ScoreController* self, NoteController* noteController) {
        NoteMiss(self, noteController);

        int noteId = _noteIdCache[noteController];

        if (noteController->noteData->colorType != ColorType::None && replay != nullopt)
        {
            NoteEvent& noteEvent = _noteEventCache.at(noteId);
            noteEvent.eventTime = audioTimeSyncController->songTime;
            noteEvent.eventType = NoteEventType::MISS;
            replay->notes.emplace_back(noteEvent);
        }
    }

    MAKE_HOOK_MATCH(ComboMultiplierChanged, &ScoreController::NotifyForChange, void,  ScoreController* self, bool comboChanged, bool multiplierChanged) {
        ComboMultiplierChanged(self, comboChanged, multiplierChanged);

        if (comboChanged && self->playerHeadAndObstacleInteraction->get_intersectingObstacles()->get_Count() > 0 && replay != nullopt) {
            WallEvent& wallEvent = _wallEventCache.at(_wallCache[self->playerHeadAndObstacleInteraction->get_intersectingObstacles()->get_Item(0)]);
            wallEvent.time = audioTimeSyncController->get_songTime();
            replay->walls.emplace_back(wallEvent);
            _currentWallEvent = wallEvent;
            phoi = self->playerHeadAndObstacleInteraction;
        }
    }

    MAKE_HOOK_MATCH(BeatMapStart, &BeatmapObjectSpawnController::Start, void, BeatmapObjectSpawnController* self) {
        BeatMapStart(self);

        if(replay != nullopt) {
            replay->info.jumpDistance = self->get_jumpDistance();
            _currentPause = nullopt;
            _currentWallEvent = nullopt;
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

        if (replay != nullopt) {
            replay->pauses.emplace_back(_currentPause.value());
            _currentPause = nullopt;
        }
    }

    MAKE_HOOK_MATCH(Tick, &PlayerTransforms::Update, void, PlayerTransforms* trans) {
        Tick(trans);
        if (audioTimeSyncController != nullptr && _currentPause == nullopt && replay != nullopt) {
            
            auto time = audioTimeSyncController->songTime;
            auto fps = 1.0f / UnityEngine::Time::get_deltaTime();
            
            auto head = Transform(trans->get_headPseudoLocalPos(), trans->get_headPseudoLocalRot());
            auto leftHand = Transform(trans->get_leftHandPseudoLocalPos(), trans->get_leftHandPseudoLocalRot());
            auto rightHand = Transform(trans->get_rightHandPseudoLocalPos(), trans->get_rightHandPseudoLocalRot());
            
            replay->frames.emplace_back(time, fps, head, leftHand, rightHand);
        }

        if (_currentWallEvent != nullopt) {
            if (phoi->get_intersectingObstacles()->get_Count() == 0)
            {
                _currentWallEvent->energy = audioTimeSyncController->songTime;
                _currentWallEvent = nullopt;
            }
        }
    }

    void StartRecording(function<void(Replay const &, MapStatus, bool)> const &callback) {
        LoggerContextObject logger = getLogger().WithContext("load");

        getLogger().info("Installing ReplayRecorder hooks...");

        INSTALL_HOOK(logger, ProcessResultsSolo);
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
        INSTALL_HOOK(logger, PlayerHeightDetectorStart);

        getLogger().info("Installed all ReplayRecorder hooks!");

        replayCallback = callback;
    }
}