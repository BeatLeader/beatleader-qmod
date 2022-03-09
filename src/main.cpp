#include "main.hpp"

#include "include/Models/Replay.hpp"
#include "include/Utils/ReplayManager.hpp"
#include "include/Utils/WebUtils.hpp"
#include "include/Utils/constants.hpp"
#include "include/Enhancers/MapEnhancer.hpp"
#include "include/Enhancers/UserEnhancer.hpp"
#include "include/API/PlayerController.hpp"
#include "include/Assets/Sprites.hpp"

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
#include "UnityEngine/UI/LayoutElement.hpp"

#include "System/Action_1.hpp"
#include "System/Threading/Tasks/Task.hpp"
#include "System/Threading/Tasks/Task_1.hpp"

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
#include "GlobalNamespace/BeatmapCharacteristicSO.hpp"
#include "GlobalNamespace/IPreviewBeatmapLevel.hpp"
#include "GlobalNamespace/LeaderboardTableCell.hpp"

#include <map>
#include <chrono>
#include <iostream>
#include <sstream>
#include <regex>

using namespace GlobalNamespace;
using namespace HMUI;
using namespace QuestUI;
using UnityEngine::Resources;

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
    replay->info->score = levelCompletionResults->rawScore;

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

template <typename T>
string to_string_wprecision(const T a_value, const int n = 6)
{
    std::ostringstream out;
    out.precision(n);
    out << std::fixed << a_value;
    return out.str();
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
    return truncate(nameLabel, 14) + "<pos=50%>" + ppLabel + "   " + accLabel; 
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
    if (PlayerController::currentPlayer == NULL) {
        self->loadingControl->ShowText("Please login in preferences", true);
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
            QuestUI::MainThreadScheduler::Schedule([self] {
                self->loadingControl->Hide();
                self->hasScoresData = false;
                self->loadingControl->ShowText("No scores was found, make one!", true);
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
        move(playerInfo, 0, -26);
        if (PlayerController::currentPlayer != NULL) {
            updatePlayerInfoLabel();
        }

        websiteLink = ::QuestUI::BeatSaberUI::CreateClickableImage(self->leaderboardTableView->get_transform(), Sprites::get_BeatLeaderIcon(), UnityEngine::Vector2(-38, -24), UnityEngine::Vector2(12, 12), []() {
            string url = "https://beatleader.xyz/";
            if (PlayerController::currentPlayer != NULL) {
                url += "u/" + PlayerController::currentPlayer->id;
            }
            UnityEngine::Application::OpenURL(url);
        });

        retryButton = ::QuestUI::BeatSaberUI::CreateUIButton(self->leaderboardTableView->get_transform(), "Retry", UnityEngine::Vector2(20, -23), UnityEngine::Vector2(8, 8), [](){
            retryButton->get_gameObject()->SetActive(false);
            ReplayManager::RetryPosting(replayPostCallback);
        });
        retryButton->get_gameObject()->SetActive(false);

        uploadStatus = ::QuestUI::BeatSaberUI::CreateText(self->leaderboardTableView->get_transform(), "", false);
        move(uploadStatus, 6, -32);
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
        move(result->fullComboText, 0.2, 0);
        move(result->scoreText, 1, 0);
        result->playerNameText->set_fontSize(3);
        result->fullComboText->set_fontSize(3);
        result->scoreText->set_fontSize(3);
    }
    
    return (TableCell *)result;
}

// Called later on in the game loading - a good time to install function hooks
extern "C" void load() {
    il2cpp_functions::Init();

    LoggerContextObject logger = getLogger().WithContext("load");

    QuestUI::Init();
    QuestUI::Register::RegisterModSettingsViewController(modInfo, DidActivate);

    getLogger().info("Installing hooks...");
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
    INSTALL_HOOK(logger, RefreshLeaderboard);
    INSTALL_HOOK(logger, LeaderboardCellSource);
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
    
    // INSTALL_HOOK(logger, RefreshLeaderboard2);
    // Install our hooks (none defined yet)
    getLogger().info("Installed all hooks!");
}