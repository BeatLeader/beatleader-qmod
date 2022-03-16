#include "main.hpp"

#include "include/Models/Replay.hpp"
#include "include/API/PlayerController.hpp"
#include "include/Assets/Sprites.hpp"

#include "include/Enhancers/MapEnhancer.hpp"
#include "include/Enhancers/UserEnhancer.hpp"

#include "include/UI/PreferencesViewController.hpp"
#include "include/UI/LevelInfoUI.hpp"
#include "include/UI/ModifiersUI.hpp"

#include "include/Utils/ReplayManager.hpp"
#include "include/Utils/WebUtils.hpp"
#include "include/Utils/constants.hpp"
#include "include/Utils/StringUtils.hpp"
#include "include/Utils/ModifiersManager.hpp"

#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "beatsaber-hook/shared/config/rapidjson-utils.hpp"

#include "questui/shared/QuestUI.hpp"
#include "questui/shared/ArrayUtil.hpp"
#include "questui/shared/BeatSaberUI.hpp"
#include "questui/shared/CustomTypes/Components/MainThreadScheduler.hpp"

#include "HMUI/TableView.hpp"
#include "HMUI/TableCell.hpp"
#include "HMUI/IconSegmentedControl.hpp"
#include "HMUI/IconSegmentedControl_DataItem.hpp"

#include "TMPro/TMP_Text.hpp"
#include "TMPro/TextMeshProUGUI.hpp"

#include "UnityEngine/Application.hpp"
#include "UnityEngine/Resources.hpp"
#include "UnityEngine/RectTransform.hpp"
#include "UnityEngine/Time.hpp"
#include "UnityEngine/UI/LayoutElement.hpp"

#include "System/Action_1.hpp"
#include "System/Threading/Tasks/Task.hpp"
#include "System/Threading/Tasks/Task_1.hpp"
#include "System/Collections/Generic/HashSet_1.hpp"

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
#include "GlobalNamespace/PlatformLeaderboardsModel.hpp"
#include "GlobalNamespace/PlayerHeightDetector.hpp"
#include "GlobalNamespace/PlatformLeaderboardViewController.hpp"
#include "GlobalNamespace/LoadingControl.hpp"
#include "GlobalNamespace/LeaderboardTableView.hpp"
#include "GlobalNamespace/LeaderboardTableView_ScoreData.hpp"
#include "GlobalNamespace/PlatformLeaderboardsModel_ScoresScope.hpp"
#include "GlobalNamespace/BeatmapDifficulty.hpp"
#include "GlobalNamespace/IBeatmapLevel.hpp"
#include "GlobalNamespace/IDifficultyBeatmapSet.hpp"
#include "GlobalNamespace/IReadonlyBeatmapData.hpp"
#include "GlobalNamespace/BeatmapCharacteristicSO.hpp"
#include "GlobalNamespace/IPreviewBeatmapLevel.hpp"
#include "GlobalNamespace/LeaderboardTableCell.hpp"
#include "GlobalNamespace/MenuTransitionsHelper.hpp"

#include <map>
#include <chrono>
#include <iostream>
#include <sstream>
#include <regex>

using namespace GlobalNamespace;
using namespace HMUI;
using namespace QuestUI;
using UnityEngine::Resources;

namespace std {
    template <>
    struct hash<Vector3>
    {
        size_t operator()(const Vector3 & v) const
        {
            std::hash<float> h;
            return h(v.z) ^ h(v.y) ^ h(v.z);
        }
    };
}

namespace std {
    template <>
    struct hash<Quaternion>
    {
        size_t operator()(const Quaternion & quat) const
        {
            std::hash<float> h;
            return h(quat.x) ^ h(quat.y) ^ h(quat.z) ^ h(quat.w);
        }
    };
}

namespace std {
    template<>
    struct hash<NoteCutInfo> {
        size_t operator()(const NoteCutInfo & noteCutInfo) const {
            std::hash<int> hashInt;
            std::hash<bool> hashBool;
            std::hash<float> hashFloat;
            std::hash<NoteData*> hashNoteData;
            std::hash<Vector3> hashVector3;
            std::hash<Quaternion> hashQuat;
            std::hash<ISaberMovementData*> hashSaberData;

            return hashBool(noteCutInfo.speedOK) ^ hashBool(noteCutInfo.directionOK) ^ hashBool(noteCutInfo.saberTypeOK) ^ hashBool(noteCutInfo.wasCutTooSoon) ^ hashFloat(noteCutInfo.saberSpeed) ^ hashVector3(noteCutInfo.saberDir) ^ hashInt(noteCutInfo.saberType.value) ^ hashFloat(noteCutInfo.timeDeviation) ^ hashFloat(noteCutInfo.cutDirDeviation) ^ hashVector3(noteCutInfo.cutPoint) ^ hashVector3(noteCutInfo.cutNormal) ^ hashFloat(noteCutInfo.cutAngle) ^ hashFloat(noteCutInfo.cutDistanceToCenter) ^ hashQuat(noteCutInfo.worldRotation) ^ hashQuat(noteCutInfo.inverseWorldRotation) ^ hashQuat(noteCutInfo.noteRotation) ^ hashVector3(noteCutInfo.notePosition) ^ hashSaberData(noteCutInfo.saberMovementData);
        }
    };
}

ModInfo modInfo; // Stores the ID and version of our mod, and is sent to the modloader upon startup

static Replay* replay;
static MapEnhancer mapEnhancer;
static UserEnhancer userEnhancer;

AudioTimeSyncController* audioTimeSyncController;
PlayerSpecificSettings* playerSettings;
PlayerHeightDetector* playerHeightDetector;
PlayerHeadAndObstacleInteraction* phoi;
PlatformLeaderboardViewController* leaderboardViewController;

static map<int, NoteCutInfo> _cutInfoCache;
static map<int, NoteEvent *> _noteEventCache;


static map<NoteData *, int> _noteIdCache;
static map<ISaberSwingRatingCounter*, int> _swingIdCache;
static int _noteId;

static map<ObstacleController *, int> _wallCache;
static map<int, WallEvent *> _wallEventCache;
static int _wallId;

static Pause* _currentPause;
static WallEvent* _currentWallEvent;
static chrono::steady_clock::time_point _pauseStartTime;
static System::Action_1<float>* _heightEvent;

TMPro::TextMeshProUGUI* uploadStatus = NULL;
TMPro::TextMeshProUGUI* playerInfo = NULL;
UnityEngine::UI::Button* retryButton = NULL;
QuestUI::ClickableImage* websiteLink = NULL;
PlatformLeaderboardViewController* plvc = NULL;

// Loads the config from disk using our modInfo, then returns it for use
Configuration& getConfig() {
    static Configuration config(modInfo);
    config.Load();
    return config;
}

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
	
    getConfig().Load(); // Load the config file
    getLogger().info("Completed setup!");
}

void collectMapData(StandardLevelScenesTransitionSetupDataSO* self, ::StringW gameMode, IDifficultyBeatmap* difficultyBeatmap, IPreviewBeatmapLevel* previewBeatmapLevel, OverrideEnvironmentSettings* overrideEnvironmentSettings, ColorScheme* overrideColorScheme, GameplayModifiers* gameplayModifiers, PlayerSpecificSettings* playerSpecificSettings, PracticeSettings* practiceSettings, ::StringW backButtonText, bool useTestNoteCutSoundEffects) {
    EnvironmentInfoSO* environmentInfoSO = BeatmapEnvironmentHelper::GetEnvironmentInfo(difficultyBeatmap);
    if (overrideEnvironmentSettings != NULL && environmentInfoSO != NULL && overrideEnvironmentSettings->overrideEnvironments)
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

MAKE_HOOK_MATCH(TransitionSetupDataInit, &StandardLevelScenesTransitionSetupDataSO::Init, void, StandardLevelScenesTransitionSetupDataSO* self, ::StringW gameMode, IDifficultyBeatmap* difficultyBeatmap, IPreviewBeatmapLevel* previewBeatmapLevel, OverrideEnvironmentSettings* overrideEnvironmentSettings, ColorScheme* overrideColorScheme, GameplayModifiers* gameplayModifiers, PlayerSpecificSettings* playerSpecificSettings, PracticeSettings* practiceSettings, ::StringW backButtonText, bool useTestNoteCutSoundEffects, bool startPaused) {
    TransitionSetupDataInit(self, gameMode, difficultyBeatmap, previewBeatmapLevel, overrideEnvironmentSettings, overrideColorScheme, gameplayModifiers, playerSpecificSettings, practiceSettings, backButtonText, useTestNoteCutSoundEffects, startPaused);
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

MAKE_HOOK_MATCH(LevelPlay, &SinglePlayerLevelSelectionFlowCoordinator::StartLevel, void, SinglePlayerLevelSelectionFlowCoordinator* self, System::Action* beforeSceneSwitchCallback, bool practice) {
    LevelPlay(self, beforeSceneSwitchCallback, practice);

    levelStarted();
}

void replayPostCallback(ReplayUploadStatus status, string description) {
    if (status == ReplayUploadStatus::finished) {
        PlayerController::Refresh();
    }
    QuestUI::MainThreadScheduler::Schedule([status, description] {
        uploadStatus->SetText(description);
        switch (status)
        {
        case ReplayUploadStatus::finished:
            leaderboardViewController->Refresh(true, true);
            break;
        case ReplayUploadStatus::error:
            retryButton->get_gameObject()->SetActive(true);
            break;
        case ReplayUploadStatus::inProgress:
            break;
        }
    });
}

void processResults(SinglePlayerLevelSelectionFlowCoordinator* self, LevelCompletionResults* levelCompletionResults, IDifficultyBeatmap* difficultyBeatmap, bool practice) {
    replay->info->score = levelCompletionResults->modifiedScore;

    mapEnhancer.energy = levelCompletionResults->energy;
    mapEnhancer.Enhance(replay);
    
    switch (levelCompletionResults->levelEndStateType)
    {
        case LevelCompletionResults::LevelEndStateType::Cleared:
            ReplayManager::ProcessReplay(replay, replayPostCallback);
            break;
        case LevelCompletionResults::LevelEndStateType::Failed:
            if (levelCompletionResults->levelEndAction != LevelCompletionResults::LevelEndAction::Restart)
            {
                replay->info->failTime = audioTimeSyncController->get_songTime();
                ReplayManager::ProcessReplay(replay, [](bool finished, string description) {
                    QuestUI::MainThreadScheduler::Schedule([description] {
                        uploadStatus->SetText(description);
                    });
                });
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

    NoteEvent* noteEvent = new NoteEvent();
    noteEvent->noteID = noteData->lineIndex * 1000 + (int)noteData->noteLineLayer * 100 + (int)noteData->colorType * 10 + (int)noteData->cutDirection;
    noteEvent->spawnTime = noteData->time;
    _noteEventCache[_noteId] = noteEvent;
}

MAKE_HOOK_MATCH(SpawnNote, &BeatmapObjectManager::AddSpawnedNoteController, void, BeatmapObjectManager* self, NoteController* noteController, BeatmapObjectSpawnMovementData::NoteSpawnData noteSpawnData, float rotation) {

    SpawnNote(self, noteController, noteSpawnData, rotation);
    NoteSpawned(noteController, noteController->noteData);
}

MAKE_HOOK_MATCH(SpawnObstacle, &BeatmapObjectManager::AddSpawnedObstacleController, void, BeatmapObjectManager* self, ObstacleController* obstacleController, BeatmapObjectSpawnMovementData::ObstacleSpawnData obstacleSpawnData, float rotation) {
    SpawnObstacle(self, obstacleController, obstacleSpawnData, rotation);
    int wallId = _wallId++;
    _wallCache[obstacleController] = wallId;

    WallEvent* wallEvent = new WallEvent();
    wallEvent->wallID = obstacleController->obstacleData->lineIndex * 100 + (int)obstacleController->obstacleData->type * 10 + obstacleController->obstacleData->width;
    wallEvent->spawnTime = audioTimeSyncController->get_songTime();
    _wallEventCache[wallId] = wallEvent;
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
    if(replay) {
        int noteId = _noteIdCache[noteCutInfo->noteData];

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
    }
}

MAKE_HOOK_MATCH(SwingRatingDidFinish, &CutScoreBuffer::HandleSaberSwingRatingCounterDidFinish, void, CutScoreBuffer* self, ISaberSwingRatingCounter* swingRatingCounter) {
    SwingRatingDidFinish(self, swingRatingCounter);
    if(replay) {
        int noteId = _noteIdCache[self->noteCutInfo.noteData];
        NoteCutInfo cutInfo = self->noteCutInfo;

        NoteEvent* cutEvent = _noteEventCache[noteId];
        ReplayNoteCutInfo* noteCutInfo = cutEvent->noteCutInfo;
        PopulateNoteCutInfo(noteCutInfo, cutInfo);
        noteCutInfo->beforeCutRating = self->get_beforeCutSwingRating();
        noteCutInfo->afterCutRating = self->get_afterCutSwingRating();
    }
}

MAKE_HOOK_MATCH(NoteMiss, &ScoreController::HandleNoteWasMissed, void, ScoreController* self, NoteController* noteController) {
    NoteMiss(self, noteController);
    if(replay) {
        int noteId = _noteIdCache[noteController->noteData];

        if (noteController->noteData->colorType != ColorType::None)
        {
            NoteEvent* noteEvent = _noteEventCache[noteId];
            noteEvent->eventTime = audioTimeSyncController->get_songTime();
            noteEvent->eventType = NoteEventType::miss;
            replay->notes.push_back(noteEvent);
        }
    }
}

MAKE_HOOK_MATCH(ComboMultiplierChanged, &ScoreController::HandlePlayerHeadDidEnterObstacles, void,  ScoreController* self) {
    ComboMultiplierChanged(self);
    if(replay) {
        if (self->scoreMultiplierCounter->ProcessMultiplierEvent(ScoreMultiplierCounter::MultiplierEventType::Negative) && self->playerHeadAndObstacleInteraction->intersectingObstacles->get_Count() > 0) {
            auto obstacleEnumerator = self->playerHeadAndObstacleInteraction->intersectingObstacles->GetEnumerator();
            if(obstacleEnumerator.MoveNext()) {
                WallEvent* wallEvent = _wallEventCache[_wallCache[reinterpret_cast<ObstacleController*>(obstacleEnumerator.current)]];
                wallEvent->time = audioTimeSyncController->get_songTime();
                replay->walls.push_back(wallEvent);
                _currentWallEvent = wallEvent;
                phoi = self->playerHeadAndObstacleInteraction;
            }
        }
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
        //getLogger().info("Check Has passed. audiotimesync is %s, currentpause is %s", audioTimeSyncController != NULL ? "not null" : "null", _currentPause != NULL ? "not null" : "null");
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
    else
        getLogger().info("check failed. audiotimesync is %s, currentpause is %s", audioTimeSyncController != NULL ? "not null" : "null", _currentPause != NULL ? "not null" : "null");

    if (_currentWallEvent != NULL) {
        if (phoi->intersectingObstacles->get_Count() == 0)
        {
            _currentWallEvent->energy = audioTimeSyncController->get_songTime();
            _currentWallEvent = NULL;
        }
    }
}

string truncate(string str, size_t width, bool show_ellipsis=true)
{
    if (str.length() > width) {
        if (show_ellipsis) {
            return str.substr(0, width) + "...";
        } else {
            return str.substr(0, width);
        }
    }
            
    return str;
}

string generateLabel(string nameLabel, string ppLabel, string accLabel) {
    return truncate(nameLabel, 20) + "<pos=50%>" + ppLabel + "   " + accLabel; 
}

void move(UnityEngine::Component* label, float x, float y) {
    UnityEngine::RectTransform* transform = label->GetComponent<UnityEngine::RectTransform *>();
    UnityEngine::Vector2 position = transform->get_anchoredPosition();
    position.x += x;
    position.y += y;
    transform->set_anchoredPosition(position);
}

void resize(UnityEngine::Component* label, float x, float y) {
    UnityEngine::RectTransform* transform = label->GetComponent<UnityEngine::RectTransform *>();
    UnityEngine::Vector2 sizeDelta = transform->get_sizeDelta();
    sizeDelta.x += x;
    sizeDelta.y += y;
    transform->set_sizeDelta(sizeDelta);
}

char asciitolower(char in) {
    if (in <= 'Z' && in >= 'A')
        return in - ('Z' - 'z');
    return in;
}

UnityEngine::Sprite* GetCountryIcon(string country) {
    std::string data;
    std::string lowerCountry;
    lowerCountry.resize(country.size());
    std::transform(country.begin(), country.end(), lowerCountry.begin(), asciitolower);
    WebUtils::Get("https://cdn.beatleader.xyz/flags/" + lowerCountry + ".png", 64, data);
    std::vector<uint8_t> bytes(data.begin(), data.end());
    Array<uint8_t>* spriteArray = il2cpp_utils::vectorToArray(bytes);
    return BeatSaberUI::ArrayToSprite(spriteArray);
}

void updatePlayerInfoLabel() {
    Player* player = PlayerController::currentPlayer;
    if (player != NULL) {
        if (player->rank > 0) {
            playerInfo->SetText("#" + to_string(player->rank) + "       " + player->name + "         " + to_string_wprecision(player->pp, 2) + "pp");

            if (plvc != NULL) {
                auto countryControl = plvc->scopeSegmentedControl->dataItems.get(2);
                countryControl->set_hintText("Country");
                countryControl->set_icon(GetCountryIcon(player->country));
                plvc->scopeSegmentedControl->SetData(plvc->scopeSegmentedControl->dataItems);
            }

        } else {
            playerInfo->SetText(player->name + ", you are ready!");
        }
    } else {
        playerInfo->SetText("");
    }
}

MAKE_HOOK_MATCH(RefreshLeaderboard, &PlatformLeaderboardViewController::Refresh, void, PlatformLeaderboardViewController* self, bool firstActivation, bool addedToHierarchy) {
    leaderboardViewController = self;
    self->scores->Clear();
    self->leaderboardTableView->tableView->SetDataSource((HMUI::TableView::IDataSource *)self->leaderboardTableView, true);

    if (PlayerController::currentPlayer == NULL) {
        self->loadingControl->ShowText("Please sign up or log in mod settings!", true);
        return;
    }
    IPreviewBeatmapLevel* levelData = reinterpret_cast<IPreviewBeatmapLevel*>(self->difficultyBeatmap->get_level());
    string hash = regex_replace((string)levelData->get_levelID(), basic_regex("custom_level_"), "");
    string difficulty = MapEnhancer::DiffName(self->difficultyBeatmap->get_difficulty().value);
    string mode = (string)self->difficultyBeatmap->get_parentDifficultyBeatmapSet()->get_beatmapCharacteristic()->serializedName;
    string url = API_URL + "scores/" + hash + "/" + difficulty + "/" + mode;

    switch (PlatformLeaderboardViewController::_get__scoresScope())
    {
    case PlatformLeaderboardsModel::ScoresScope::AroundPlayer:
        url += "?player=" + PlayerController::currentPlayer->id;
        break;
    case PlatformLeaderboardsModel::ScoresScope::Friends:
        url += "?country=" + PlayerController::currentPlayer->country;
        break;
    
    default:
        break;
    } 

    WebUtils::GetJSONAsync(url, [self](long status, bool error, rapidjson::Document& result){
        auto scores = result.GetArray();
        self->scores->Clear();
        if ((int)scores.Size() == 0) {
            QuestUI::MainThreadScheduler::Schedule([self, status] {
                self->loadingControl->Hide();
                self->hasScoresData = false;
                if (status == 404) {
                    self->loadingControl->ShowText("Leaderboards for this map are not supported!", true);
                } else {
                    self->loadingControl->ShowText("No scores were found!", true);
                }
                
                self->leaderboardTableView->tableView->SetDataSource((HMUI::TableView::IDataSource *)self->leaderboardTableView, true);
            });
            return;
        }

        int selectedScore = 10;
        for (int index = 0; index < 10; ++index)
        {
            if (index < (int)scores.Size())
            {
                auto score = scores[index].GetObject();

                string ppLabel = score["pp"].GetDouble() > 0 ? to_string_wprecision(score["pp"].GetDouble(), 2) + "pp" : "";
                string accLabel = to_string_wprecision(score["accuracy"].GetDouble() * 100, 2) + "%";
                string nameLabel = score["player"].GetObject()["name"].GetString();

                if (nameLabel.compare(PlayerController::currentPlayer->name) == 0) {
                    selectedScore = index;
                }

                LeaderboardTableView::ScoreData* scoreData = LeaderboardTableView::ScoreData::New_ctor(
                    score["modifiedScore"].GetInt(), 
                    generateLabel(nameLabel, ppLabel, accLabel), 
                    score["rank"].GetInt(), 
                    score["fullCombo"].GetBool());
                self->scores->Add(scoreData);
            }
        }
            
        self->leaderboardTableView->scores = self->scores;
        self->leaderboardTableView->specialScorePos = selectedScore;
        QuestUI::MainThreadScheduler::Schedule([self] {
            self->loadingControl->Hide();
            self->hasScoresData = true;
            self->leaderboardTableView->tableView->SetDataSource((HMUI::TableView::IDataSource *)self->leaderboardTableView, true);
        });
    });

    self->loadingControl->ShowText("Loading", true);
    
    if (uploadStatus == NULL) {
        plvc = self;
        playerInfo = ::QuestUI::BeatSaberUI::CreateText(self->leaderboardTableView->get_transform(), "", false);
        move(playerInfo, 5, -26);
        if (PlayerController::currentPlayer != NULL) {
            updatePlayerInfoLabel();
        }

        websiteLink = ::QuestUI::BeatSaberUI::CreateClickableImage(self->leaderboardTableView->get_transform(), Sprites::get_BeatLeaderIcon(), UnityEngine::Vector2(-33, -24), UnityEngine::Vector2(12, 12), []() {
            string url = "https://beatleader.xyz/";
            if (PlayerController::currentPlayer != NULL) {
                url += "u/" + PlayerController::currentPlayer->id;
            }
            UnityEngine::Application::OpenURL(url);
        });

        retryButton = ::QuestUI::BeatSaberUI::CreateUIButton(self->leaderboardTableView->get_transform(), "Retry", UnityEngine::Vector2(25, -23), [](){
            retryButton->get_gameObject()->SetActive(false);
            ReplayManager::RetryPosting(replayPostCallback);
        });
        retryButton->get_gameObject()->SetActive(false);

        uploadStatus = ::QuestUI::BeatSaberUI::CreateText(self->leaderboardTableView->get_transform(), "", false);
        move(uploadStatus, 11, -32);
        resize(uploadStatus, 10, 0);
        uploadStatus->set_fontSize(3);
        uploadStatus->set_richText(true);
    }
}

MAKE_HOOK_MATCH(LeaderboardCellSource, &LeaderboardTableView::CellForIdx, TableCell*, LeaderboardTableView* self, TableView* tableView, int row) {
    LeaderboardTableCell* result = (LeaderboardTableCell *)LeaderboardCellSource(self, tableView, row);

    if (result->playerNameText->get_fontSize() > 3) {
        result->playerNameText->set_enableAutoSizing(false);
        result->playerNameText->set_richText(true);
        resize(result->playerNameText, 10, 0);
        move(result->playerNameText, -2, 0);
        move(result->fullComboText, 0.2, 0);
        move(result->scoreText, 1, 0);
        result->playerNameText->set_fontSize(3);
        result->fullComboText->set_fontSize(3);
        result->scoreText->set_fontSize(3);
    }
    
    return (TableCell *)result;
}

MAKE_HOOK_MATCH(Restart, &MenuTransitionsHelper::RestartGame, void, MenuTransitionsHelper* self, ::System::Action_1<::Zenject::DiContainer*>* finishCallback) {
    Restart(self, finishCallback);

    uploadStatus = NULL;
    plvc = NULL;
    ResetLevelInfoUI();
}

// Called later on in the game loading - a good time to install function hooks
extern "C" void load() {
    il2cpp_functions::Init();

    LoggerContextObject logger = getLogger().WithContext("load");

    QuestUI::Init();
    QuestUI::Register::RegisterModSettingsViewController(modInfo, "BeatLeader", PreferencesDidActivate);

    getLogger().info("Installing hooks...");
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
    INSTALL_HOOK(logger, RefreshLeaderboard);
    INSTALL_HOOK(logger, LeaderboardCellSource);
    INSTALL_HOOK(logger, Restart);

    getLogger().info("Installed all hooks!");

    SetupLevelInfoUI();
    SetupModifiersUI();

    PlayerController::playerChanged = [](Player* updated) {
        QuestUI::MainThreadScheduler::Schedule([] {
            if (playerInfo != NULL) {
                updatePlayerInfoLabel();
            }
        });
    };
    QuestUI::MainThreadScheduler::Schedule([] {
        PlayerController::Refresh();
    });

    ModifiersManager::Sync();
}