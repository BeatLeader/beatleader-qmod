
#include "include/Core/ReplayRecorder.hpp"

#include "System/Action_1.hpp"
#include "System/Threading/Tasks/Task.hpp"
#include "System/Threading/Tasks/Task_1.hpp"
#include "System/Collections/Generic/HashSet_1.hpp"

#include "include/Models/Replay.hpp"

#include "include/Enhancers/MapEnhancer.hpp"
#include "include/Enhancers/UserEnhancer.hpp"

#include "include/Utils/ReplayManager.hpp"
#include "include/Utils/ModConfig.hpp"
#include "include/API/PlayerController.hpp"

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
#include "GlobalNamespace/StandardLevelScenesTransitionSetupDataSO.hpp"
#include "GlobalNamespace/OverrideEnvironmentSettings.hpp"
#include "GlobalNamespace/GameplayCoreInstaller.hpp"
#include "GlobalNamespace/AudioTimeSyncController.hpp"
#include "GlobalNamespace/BeatmapObjectSpawnMovementData.hpp"
#include "GlobalNamespace/BeatmapObjectData.hpp"
#include "GlobalNamespace/NoteData.hpp"
#include "GlobalNamespace/CutScoreBuffer.hpp"
#include "GlobalNamespace/ObstacleData.hpp"
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
#include "GlobalNamespace/IBeatmapLevelData.hpp"
#include "GlobalNamespace/IReadonlyBeatmapData.hpp"
#include "GlobalNamespace/BeatmapCharacteristicSO.hpp"
#include "GlobalNamespace/BeatmapLevel.hpp"
#include "GlobalNamespace/SaberMovementData.hpp"
#include "GlobalNamespace/SaberSwingRating.hpp"
#include "GlobalNamespace/GameplayCoreSceneSetupData.hpp"
#include "GlobalNamespace/SaberManager.hpp"
#include "GlobalNamespace/Saber.hpp"

#include "GlobalNamespace/ScoringElement.hpp"
#include "GlobalNamespace/BadCutScoringElement.hpp"
#include "GlobalNamespace/GoodCutScoringElement.hpp"
#include "GlobalNamespace/MissScoringElement.hpp"
#include "GlobalNamespace/MultiplayerController.hpp"
#include "GlobalNamespace/MultiplayerLevelScenesTransitionSetupDataSO.hpp"
#include "GlobalNamespace/MultiplayerResultsData.hpp"
#include "GlobalNamespace/MultiplayerPlayerResultsData.hpp"
#include "GlobalNamespace/MultiplayerLevelCompletionResults.hpp"
#include "GlobalNamespace/PlayersSpecificSettingsAtGameStartModel.hpp"
#include "GlobalNamespace/PlayerSpecificSettingsNetSerializable.hpp"

#include "Zenject/DiContainer.hpp"

#include "sombrero/shared/FastQuaternion.hpp"
#include "sombrero/shared/Vector3Utils.hpp"
#include "sombrero/shared/QuaternionUtils.hpp"

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
    std::function<void(Replay const&, PlayEndData, bool)> replayCallback;
    std::function<void(void)> startedCallback;

    MapEnhancer mapEnhancer;
    UserEnhancer userEnhancer;

    AudioTimeSyncController* audioTimeSyncController;
    PlayerTransforms* playerTransforms;
    SaberManager* saberManager;
    bool automaticPlayerHeight = false;
    PlayerHeadAndObstacleInteraction* phoi;

    static map<NoteData *, int> _noteIdCache;
    map<int, NoteEvent> _noteEventCache;
    map<int, NoteController *> _noteControllerCache;
    map<int, NoteCutInfo> _noteCutInfoCache;

    map<SaberMovementData *, float> _preSwingContainer;
    map<SaberSwingRatingCounter *, float> _postSwingContainer;

    static map<NoteData *, int> _replayNoteIdCache;
    map<ISaberSwingRatingCounter*, int> _swingIdCache;
    int _noteId;

    map<ObstacleController *, int> _wallCache;
    map<int, WallEvent> _wallEventCache;
    int _wallId;

    optional<Pause> _currentPause;
    optional<WallEvent> _currentWallEvent;
    chrono::steady_clock::time_point _pauseStartTime;
    System::Action_1<float>* _heightEvent;
    System::Action_1<ScoringElement*>* _scoreEvent;
    System::Action_1<UnityW<ObstacleController>>* _wallEvent;
    
    void collectMapData(StandardLevelScenesTransitionSetupDataSO* self) {

        mapEnhancer.difficultyBeatmap = self->beatmapKey;
        mapEnhancer.beatmapLevel = self->beatmapLevel;
        mapEnhancer.gameplayModifiers = self->gameplayModifiers;
        mapEnhancer.playerSpecificSettings = self->gameplayCoreSceneSetupData->playerSpecificSettings;
        mapEnhancer.practiceSettings = self->practiceSettings;
        mapEnhancer.environmentInfo = self->environmentInfo;
        mapEnhancer.colorScheme = self->colorScheme;

        automaticPlayerHeight = self->gameplayCoreSceneSetupData->playerSpecificSettings->automaticPlayerHeight;
    }

    void collectMultiplayerMapData(MultiplayerLevelScenesTransitionSetupDataSO* self) {
        GameplayCoreSceneSetupData* gameplayCoreSceneSetupData = reinterpret_cast<GameplayCoreSceneSetupData*>(self->gameplayCoreSceneSetupData);

        mapEnhancer.difficultyBeatmap = self->beatmapKey;
        mapEnhancer.beatmapLevel = gameplayCoreSceneSetupData->beatmapLevel;
        mapEnhancer.gameplayModifiers = gameplayCoreSceneSetupData->gameplayModifiers;
        mapEnhancer.playerSpecificSettings = gameplayCoreSceneSetupData->playerSpecificSettings;
        mapEnhancer.practiceSettings = NULL;
        mapEnhancer.environmentInfo = self->_loadedMultiplayerEnvironmentInfo;
        mapEnhancer.colorScheme = self->colorScheme;

        automaticPlayerHeight = gameplayCoreSceneSetupData->playerSpecificSettings->automaticPlayerHeight;
    }

    void startReplay() {
        std::string timeStamp(std::to_string(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count()));

        recording = true;
        _currentPause = nullopt;
        replay.emplace(ReplayInfo(modInfo.version, UnityEngine::Application::get_version(), timeStamp));

        userEnhancer.Enhance(replay.value());
        audioTimeSyncController = nullptr;
    }

    MAKE_HOOK_MATCH(SinglePlayerInstallBindings, &GameplayCoreInstaller::InstallBindings, void, GameplayCoreInstaller* self) {
        SinglePlayerInstallBindings(self);

        startedCallback();
        if (UploadDisabledByReplay()) {
            replay = nullopt;
            return;
        }

        startReplay();
    }

    MAKE_HOOK_MATCH(PlayerHeightDetectorLateUpdate, &PlayerHeightDetector::LateUpdate, void, PlayerHeightDetector* self) {
        PlayerHeightDetectorLateUpdate(self);

        if (audioTimeSyncController && automaticPlayerHeight && replay != nullopt) {
            replay->heights.emplace_back(self->_lastReportedHeight, audioTimeSyncController->songTime);
        }
    }

    void processResults(LevelCompletionResults* levelCompletionResults, bool skipUpload) {
        replay->info.score = levelCompletionResults->multipliedScore;

        mapEnhancer.energy = levelCompletionResults->energy;
        mapEnhancer.Enhance(replay.value());

        auto playEndData = PlayEndData(levelCompletionResults, replay->info.speed);

        if (playEndData.GetEndType() == LevelEndType::Fail) {
            replay->info.failTime = audioTimeSyncController->songTime;
        }
        
        replayCallback(*replay, playEndData, skipUpload);
    }

    MAKE_HOOK_MATCH(ProcessResultsSolo, &StandardLevelScenesTransitionSetupDataSO::Finish, void, StandardLevelScenesTransitionSetupDataSO* self, LevelCompletionResults* levelCompletionResults) {
        ProcessResultsSolo(self, levelCompletionResults);
        recording = false;
        if (replay != nullopt) {
            collectMapData(self);
            processResults(levelCompletionResults, self->gameMode == "Party");
        }
    }

    void processMultiplayerResults(MultiplayerResultsData* levelCompletionResults) {
        auto results = levelCompletionResults->localPlayerResultData->multiplayerLevelCompletionResults;

        if (results->get_hasAnyResults()) {
            switch (results->playerLevelEndReason)
            {
                case MultiplayerLevelCompletionResults::MultiplayerPlayerLevelEndReason::Cleared:
                {
                    auto results = levelCompletionResults->localPlayerResultData->multiplayerLevelCompletionResults->levelCompletionResults; 
                    auto playEndData = PlayEndData(results, replay->info.speed);

                    replay->info.score = results->multipliedScore;

                    mapEnhancer.energy = results->energy;
                    mapEnhancer.Enhance(replay.value());
                    replayCallback(*replay, playEndData, false);
                    break;
                }
                default:
                    break;
            }
        }
    }

    MAKE_HOOK_MATCH(ProcessResultsMultiplayer, &MultiplayerLevelScenesTransitionSetupDataSO::Finish, void, MultiplayerLevelScenesTransitionSetupDataSO* self, MultiplayerResultsData* levelCompletionResults) {
        ProcessResultsMultiplayer(self, levelCompletionResults);
        recording = false;
        if (replay != nullopt && levelCompletionResults != NULL) {
            collectMultiplayerMapData(self);
            processMultiplayerResults(levelCompletionResults);
        }
    }

    void NoteSpawned(NoteController* noteController, NoteData* noteData) {
        if (replay == nullopt) return;
        _noteId++;
        _noteIdCache[noteData] = _noteId;
        _noteControllerCache[_noteId] = noteController;

        int colorType = (int)noteData->colorType;
        if (colorType < 0) {
            colorType = 3;
        }
        auto noteID = ((int)noteData->scoringType + 2) * 10000 + noteData->lineIndex * 1000 + (int)noteData->noteLineLayer * 100 + colorType * 10 + (int)noteData->cutDirection;
        auto spawnTime = noteData->time;
        _noteEventCache.emplace(_noteId, NoteEvent(noteID, spawnTime));
    }

    MAKE_HOOK_MATCH(SpawnNote, &BeatmapObjectManager::AddSpawnedNoteController, void, BeatmapObjectManager* self, NoteController* noteController, BeatmapObjectSpawnMovementData::NoteSpawnData noteSpawnData, float rotation) {
        SpawnNote(self, noteController, noteSpawnData, rotation);
        NoteSpawned(noteController, noteController->noteData);
    }

    MAKE_HOOK_MATCH(SpawnObstacle, &BeatmapObjectManager::AddSpawnedObstacleController, void, BeatmapObjectManager* self, ObstacleController* obstacleController, BeatmapObjectSpawnMovementData::ObstacleSpawnData obstacleSpawnData, float rotation) {
        SpawnObstacle(self, obstacleController, obstacleSpawnData, rotation);

        if (replay != nullopt && audioTimeSyncController != NULL && obstacleController->obstacleData != NULL) {
            int wallId = _wallId++;
            _wallCache[obstacleController] = wallId;

            auto wallID = obstacleController->obstacleData->lineIndex * 100 + (int)obstacleController->obstacleData->type * 10 + obstacleController->obstacleData->width;
            auto spawnTime = audioTimeSyncController->songTime;

            _wallEventCache.emplace(wallId, WallEvent(wallID, spawnTime));
        }
    }

    void PopulateNoteCutInfo(ReplayNoteCutInfo& noteCutInfo, NoteCutInfo const& cutInfo, NoteController* noteController) {
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
        if (Sombrero::FastVector3::Dot(noteCutInfo.cutNormal, noteCutInfo.cutPoint - noteController->noteTransform->get_position()) <= 0) {
            noteCutInfo.cutDistanceToCenterPositive = true;
        }
    }

    MAKE_HOOK_MATCH(HandleNoteControllerNoteWasCut, &BeatmapObjectManager::HandleNoteControllerNoteWasCut, void, BeatmapObjectManager* self, NoteController* noteController, ByRef<::GlobalNamespace::NoteCutInfo> noteCutInfo) {
        HandleNoteControllerNoteWasCut(self, noteController, noteCutInfo);

        if (replay == nullopt) return;

        auto noteData = noteController->noteData;

        if (!_noteIdCache.count(noteData)) return;

        int noteId = _noteIdCache[noteData];
        _noteCutInfoCache[noteId] = noteCutInfo.heldRef;
    }

    static float ChooseSwingRating(float real, float unclamped) {
        return real < 1 ? real : max(real, unclamped);
    }

    void onObstacle(UnityW<ObstacleController> obstacle) {
        if (_currentWallEvent == nullopt) {
            WallEvent& wallEvent = _wallEventCache.at(_wallCache[obstacle.ptr()]);
            wallEvent.time = audioTimeSyncController->songTime;
            replay->walls.emplace_back(wallEvent);
            _currentWallEvent.emplace(wallEvent);
        }
    }

    void scoringElementFinished(ScoringElement* scoringElement) {
        if (replay == nullopt) return;

        auto noteData = scoringElement->noteData;

        if (!_replayNoteIdCache.count(noteData)) return;
        int noteId = _noteIdCache[noteData];
        int replayNoteId = _replayNoteIdCache[noteData];
        if (replay->notes.size() <= replayNoteId) return;
        NoteEvent& noteEvent = replay->notes.at(replayNoteId);
        bool isBomb = noteData->colorType == ColorType::None;
        
        if (il2cpp_utils::try_cast<MissScoringElement>(scoringElement) != nullopt) {
            if (isBomb) return;

            noteEvent.eventType = NoteEventType::MISS;
        } else if (il2cpp_utils::try_cast<BadCutScoringElement>(scoringElement) != nullopt) {
            noteEvent.eventType = isBomb ? NoteEventType::BOMB : NoteEventType::BAD;

            if (!_noteCutInfoCache.count(noteId)) return;

            ReplayNoteCutInfo& noteCutInfo = noteEvent.noteCutInfo;
            PopulateNoteCutInfo(noteCutInfo, _noteCutInfoCache[noteId], _noteControllerCache[noteId]);
                
        } else if (il2cpp_utils::try_cast<GoodCutScoringElement>(scoringElement) != nullopt) {
            GoodCutScoringElement* goodCut = il2cpp_utils::try_cast<GoodCutScoringElement>(scoringElement).value();
            CutScoreBuffer* cutScoreBuffer = goodCut->_cutScoreBuffer;
            SaberSwingRatingCounter* saberSwingRatingCounter = cutScoreBuffer->_saberSwingRatingCounter;

            ReplayNoteCutInfo& noteCutInfo = noteEvent.noteCutInfo;
            PopulateNoteCutInfo(noteCutInfo, cutScoreBuffer->noteCutInfo, _noteControllerCache[noteId]);
            
            noteCutInfo.beforeCutRating = ChooseSwingRating(saberSwingRatingCounter->beforeCutRating, _preSwingContainer[(SaberMovementData *)saberSwingRatingCounter->_saberMovementData]);
            noteCutInfo.afterCutRating = ChooseSwingRating(saberSwingRatingCounter->afterCutRating, _postSwingContainer[saberSwingRatingCounter]);

            _preSwingContainer[(SaberMovementData *)saberSwingRatingCounter->_saberMovementData] = 0;
            _postSwingContainer[saberSwingRatingCounter] = 0;
        }
    }

    MAKE_HOOK_MATCH(ScoreControllerLateUpdate, &ScoreController::LateUpdate, void, ScoreController* self) {
        auto sortedScoringElementsWithoutMultiplier = self->_sortedScoringElementsWithoutMultiplier;
        auto sortedNoteTimesWithoutScoringElements = self->_sortedNoteTimesWithoutScoringElements;

        if (replay
            && sortedScoringElementsWithoutMultiplier 
            && sortedNoteTimesWithoutScoringElements
            && self->_audioTimeSyncController) {
            auto songTime = self->_audioTimeSyncController->songTime;

            float nearestNotCutNoteTime = sortedNoteTimesWithoutScoringElements->get_Count() > 0 ? sortedNoteTimesWithoutScoringElements->get_Item(0) : 10000000;
            float skipAfter = songTime + 0.15f;

            for (int i = 0; i < sortedScoringElementsWithoutMultiplier->get_Count(); i++) {
                auto scoringElement = sortedScoringElementsWithoutMultiplier->get_Item(i);
                if (scoringElement->get_time() >= skipAfter && scoringElement->get_time() <= nearestNotCutNoteTime) break;
                
                auto noteData = scoringElement->noteData;
                if (il2cpp_utils::try_cast<MissScoringElement>(scoringElement) != nullopt && (noteData->colorType == ColorType::None || noteData->scoringType == NoteData::ScoringType::NoScore)) continue;

                int noteId = _noteIdCache[noteData];
                NoteEvent& noteEvent = _noteEventCache.at(noteId);
                noteEvent.eventTime = songTime;

                if (replay != nullopt) {
                    replay->notes.push_back(noteEvent);
                    _replayNoteIdCache[noteData] = replay->notes.size() - 1;
                }
            }
        }

        ScoreControllerLateUpdate(self);
    }

    MAKE_HOOK_MATCH(ScoreControllerStart, &ScoreController::Start, void, ScoreController* self) {
        ScoreControllerStart(self);
        if (replay == nullopt) return;

        _scoreEvent = custom_types::MakeDelegate<System::Action_1<ScoringElement*> *>((std::function<void(ScoringElement*)>)scoringElementFinished);
        self->add_scoringForNoteFinishedEvent(_scoreEvent);

        _wallEvent = custom_types::MakeDelegate<System::Action_1<UnityW<ObstacleController>> *>((std::function<void(UnityW<ObstacleController>)>)onObstacle);
        self->_playerHeadAndObstacleInteraction->add_headDidEnterObstacleEvent(_wallEvent);

        phoi = self->_playerHeadAndObstacleInteraction;

        audioTimeSyncController = self->_audioTimeSyncController;
    }

    MAKE_HOOK_MATCH(ComputeSwingRating, static_cast<float (SaberMovementData::*)(bool, float)>(&SaberMovementData::ComputeSwingRating), float, SaberMovementData* self, bool overrideSegmenAngle, float overrideValue) {
        float result = ComputeSwingRating(self, overrideSegmenAngle, overrideValue);
        if (replay == nullopt) return result;
        auto _data = self->_data;
        int _nextAddIndex = self->_nextAddIndex;
        int _validCount = self->_validCount;

        int length = _data.size();

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
        bool alreadyCut = self->_notePlaneWasCut;
        ProcessNewSwingData(self, newData, prevData, prevDataAreValid);
        if (replay == nullopt) return;

        float postSwing = _postSwingContainer[self];
        if (!alreadyCut && !self->_notePlane.SameSide(newData.topPos, prevData.topPos))
        {
            float angleDiff = UnityEngine::Vector3::Angle(UnityEngine::Vector3::op_Subtraction(self->_cutTopPos, self->_cutBottomPos), UnityEngine::Vector3::op_Subtraction(self->_afterCutTopPos, self->_afterCutBottomPos));

            if (self->_rateAfterCut)
            {
                postSwing = SaberSwingRating::AfterCutStepRating(angleDiff, 0.0f);
            }
        }
        else
        {
            float normalDiff = UnityEngine::Vector3::Angle(newData.segmentNormal, self->_cutPlaneNormal);
            if (self->_rateAfterCut)
            {
                postSwing += SaberSwingRating::AfterCutStepRating(newData.segmentAngle, normalDiff);
            }
        }

        _postSwingContainer[self] = postSwing;
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
        if (replay == nullopt) return;

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

    MAKE_HOOK_MATCH(GameplayCoreInstallerInstall, &GameplayCoreInstaller::InstallBindings, void, GameplayCoreInstaller* installer) {
        GameplayCoreInstallerInstall(installer);

        auto container = installer->get_Container();
        
        playerTransforms = reinterpret_cast<PlayerTransforms*>(container->Resolve(csTypeOf(PlayerTransforms*)));
        saberManager = reinterpret_cast<SaberManager*>(container->Resolve(csTypeOf(SaberManager*)));
    }

    Sombrero::FastQuaternion Inverse(Sombrero::FastQuaternion rotation) {
        float lengthSq = rotation.x * rotation.x + rotation.y * rotation.y + rotation.z * rotation.z + rotation.w * rotation.w;
        if (lengthSq != 0.0)
        {
            float i = 1.0f / lengthSq;
            return Sombrero::FastQuaternion(rotation.x * -i, rotation.y * -i, rotation.z * -i, rotation.w * i);
        }
        return rotation;
    }

    Sombrero::FastQuaternion InverseTransformRotation(Sombrero::FastQuaternion rotation, Sombrero::FastQuaternion worldRotation) {
        return Inverse(rotation) * worldRotation;
    }

    MAKE_HOOK_MATCH(Tick, &PlayerTransforms::Update, void, PlayerTransforms* trans) {
        Tick(trans);
        if (replay == nullopt || !saberManager || trans != playerTransforms) return;

        if (audioTimeSyncController != nullptr && _currentPause == nullopt) {
            
            auto time = audioTimeSyncController->songTime;
            auto fps = 1.0f / UnityEngine::Time::get_deltaTime();

            auto origin = trans->_originParentTransform;
            auto headTransform = trans->_headTransform;
            auto leftSaber = saberManager->leftSaber->get_transform();
            auto rightSaber = saberManager->rightSaber->get_transform();
            
            auto head = ReplayTransform(origin->InverseTransformPoint(headTransform->position), InverseTransformRotation(origin->rotation, headTransform->rotation));
            auto leftHand = ReplayTransform(origin->InverseTransformPoint(leftSaber->position), InverseTransformRotation(origin->rotation, leftSaber->rotation));
            auto rightHand = ReplayTransform(origin->InverseTransformPoint(rightSaber->position), InverseTransformRotation(origin->rotation, rightSaber->rotation));
            
            replay->frames.emplace_back(time, fps, head, leftHand, rightHand);
        }

        if (_currentWallEvent != nullopt) {
            if (phoi->_intersectingObstacles->get_Count() == 0)
            {
                WallEvent& wallEvent = replay->walls[replay->walls.size() - 1];
                wallEvent.energy = audioTimeSyncController->songTime;
                _currentWallEvent = nullopt;
            }
        }
    }

    void StartRecording(
        function<void(void)> const &started,
        function<void(Replay const &, PlayEndData, bool)> const &callback) {

        BeatLeaderLogger.info("Installing ReplayRecorder hooks...");

        INSTALL_HOOK(BeatLeaderLogger, ProcessResultsSolo);
        INSTALL_HOOK(BeatLeaderLogger, SinglePlayerInstallBindings);
        INSTALL_HOOK(BeatLeaderLogger, SpawnNote);
        INSTALL_HOOK(BeatLeaderLogger, SpawnObstacle);
        INSTALL_HOOK(BeatLeaderLogger, BeatMapStart);
        INSTALL_HOOK(BeatLeaderLogger, LevelPause);
        INSTALL_HOOK(BeatLeaderLogger, LevelUnpause);
        INSTALL_HOOK(BeatLeaderLogger, GameplayCoreInstallerInstall);
        INSTALL_HOOK(BeatLeaderLogger, Tick);
        INSTALL_HOOK(BeatLeaderLogger, ComputeSwingRating);
        INSTALL_HOOK(BeatLeaderLogger, ProcessNewSwingData);
        INSTALL_HOOK(BeatLeaderLogger, PlayerHeightDetectorLateUpdate);
        INSTALL_HOOK(BeatLeaderLogger, ScoreControllerStart);
        INSTALL_HOOK(BeatLeaderLogger, ScoreControllerLateUpdate);
        INSTALL_HOOK(BeatLeaderLogger, ProcessResultsMultiplayer);
        INSTALL_HOOK(BeatLeaderLogger, HandleNoteControllerNoteWasCut);

        BeatLeaderLogger.info("Installed all ReplayRecorder hooks!");

        startedCallback = started;
        replayCallback = callback;
    }
}