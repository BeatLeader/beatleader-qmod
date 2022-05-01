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
#include "HMUI/CurvedCanvasSettings.hpp"
#include "HMUI/Screen.hpp"
#include "HMUI/ViewController.hpp"
#include "HMUI/ViewController_AnimationType.hpp"
#include "HMUI/CurvedTextMeshPro.hpp"

#include "TMPro/TMP_Text.hpp"
#include "TMPro/TextMeshProUGUI.hpp"

#include "UnityEngine/Application.hpp"
#include "UnityEngine/Resources.hpp"
#include "UnityEngine/RectTransform.hpp"
#include "UnityEngine/Time.hpp"
#include "UnityEngine/SceneManagement/SceneManager.hpp"
#include "UnityEngine/UI/LayoutElement.hpp"
#include "UnityEngine/Canvas.hpp"
#include "UnityEngine/AdditionalCanvasShaderChannels.hpp"
#include "UnityEngine/UI/CanvasScaler.hpp"

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
#include "GlobalNamespace/IBeatmapLevelData.hpp"
#include "GlobalNamespace/IDifficultyBeatmapSet.hpp"
#include "GlobalNamespace/IReadonlyBeatmapData.hpp"
#include "GlobalNamespace/BeatmapCharacteristicSO.hpp"
#include "GlobalNamespace/IPreviewBeatmapLevel.hpp"
#include "GlobalNamespace/LeaderboardTableCell.hpp"
#include "GlobalNamespace/MenuTransitionsHelper.hpp"

#include "VRUIControls/VRGraphicRaycaster.hpp"

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
HMUI::ImageView* websiteLinkImage = NULL;
UnityEngine::UI::Button* websiteLinkButton = NULL;
PlatformLeaderboardViewController* plvc = NULL;
UnityEngine::UI::Toggle* showBeatLeaderButton = NULL;

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

bool isOst = false;

void collectMapData(StandardLevelScenesTransitionSetupDataSO* self, ::Il2CppString* gameMode, IDifficultyBeatmap* difficultyBeatmap, IPreviewBeatmapLevel* previewBeatmapLevel, OverrideEnvironmentSettings* overrideEnvironmentSettings, ColorScheme* overrideColorScheme, GameplayModifiers* gameplayModifiers, PlayerSpecificSettings* playerSpecificSettings, PracticeSettings* practiceSettings, ::Il2CppString* backButtonText, bool useTestNoteCutSoundEffects) {
    EnvironmentInfoSO* environmentInfoSO = BeatmapEnvironmentHelper::GetEnvironmentInfo(difficultyBeatmap);
    if (overrideEnvironmentSettings != NULL && environmentInfoSO != NULL && overrideEnvironmentSettings->overrideEnvironments)
    {
        environmentInfoSO = overrideEnvironmentSettings->GetOverrideEnvironmentInfoForType(environmentInfoSO->environmentType);
    }
    isOst = !to_utf8(csstrtostr(previewBeatmapLevel->get_levelID())).starts_with("custom_level");

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

MAKE_HOOK_MATCH(TransitionSetupDataInit, &StandardLevelScenesTransitionSetupDataSO::Init, void, StandardLevelScenesTransitionSetupDataSO* self, ::Il2CppString* gameMode, IDifficultyBeatmap* difficultyBeatmap, IPreviewBeatmapLevel* previewBeatmapLevel, OverrideEnvironmentSettings* overrideEnvironmentSettings, ColorScheme* overrideColorScheme, GameplayModifiers* gameplayModifiers, PlayerSpecificSettings* playerSpecificSettings, PracticeSettings* practiceSettings, ::Il2CppString* backButtonText, bool useTestNoteCutSoundEffects) {
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

void levelStarted(SinglePlayerLevelSelectionFlowCoordinator* self) {
    _currentPause = NULL;

    replay = new Replay();
    replay->info->version = modInfo.version;
    replay->info->gameVersion = to_utf8(csstrtostr(UnityEngine::Application::get_version()));

    std::stringstream strm;
    strm << std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    replay->info->timestamp = strm.str();
    userEnhancer.Enhance(replay);

    if (playerSettings->get_automaticPlayerHeight()) {
        auto detectors = Resources::FindObjectsOfTypeAll<PlayerHeightDetector*>();
        if (detectors->Length() == 0 || detectors->get(0) == 0) { return; }
        playerHeightDetector = detectors->get(0);
        _heightEvent = il2cpp_utils::MakeDelegate<System::Action_1<float>*>(
            classof(System::Action_1<float>*), 
            self, OnPlayerHeightChange);
        playerHeightDetector->add_playerHeightDidChangeEvent(_heightEvent);
    }
}

MAKE_HOOK_MATCH(LevelPlay, &SinglePlayerLevelSelectionFlowCoordinator::StartLevel, void, SinglePlayerLevelSelectionFlowCoordinator* self, System::Action* beforeSceneSwitchCallback, bool practice) {
    LevelPlay(self, beforeSceneSwitchCallback, practice);

    levelStarted(self);
}

void replayPostCallback(ReplayUploadStatus status, string description, float progress) {
    if (status == ReplayUploadStatus::finished) {
        PlayerController::Refresh();
    }
    QuestUI::MainThreadScheduler::Schedule([status, description, progress] {
        uploadStatus->SetText(il2cpp_utils::createcsstr(description));
        switch (status)
        {
            case ReplayUploadStatus::finished:
                leaderboardViewController->Refresh(true, true);
                break;
            case ReplayUploadStatus::error:
                retryButton->get_gameObject()->SetActive(true);
                break;
            case ReplayUploadStatus::inProgress:
                if(progress >= 100)
                    uploadStatus->SetText(il2cpp_utils::createcsstr("<color=#b103fcff>Posting replay: Finishing up..."));
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
            ReplayManager::ProcessReplay(replay, replayPostCallback, isOst);
            break;
        case LevelCompletionResults::LevelEndStateType::Failed:
            if (levelCompletionResults->levelEndAction != LevelCompletionResults::LevelEndAction::Restart)
            {
                replay->info->failTime = audioTimeSyncController->get_songTime();
                ReplayManager::ProcessReplay(replay, [](bool finished, string description, float progress) {
                    QuestUI::MainThreadScheduler::Schedule([description] {
                        uploadStatus->SetText(il2cpp_utils::createcsstr(description));
                    });
                }, isOst);
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

void PopulateNoteCutInfo(ReplayNoteCutInfo* noteCutInfo, NoteCutInfo cutInfo) {
    noteCutInfo->speedOK = cutInfo.speedOK;
    noteCutInfo->directionOK = cutInfo.directionOK;
    noteCutInfo->saberTypeOK = cutInfo.saberTypeOK;
    noteCutInfo->wasCutTooSoon = cutInfo.wasCutTooSoon;
    noteCutInfo->saberSpeed = cutInfo.saberSpeed;
    noteCutInfo->saberDir = new Vector3(cutInfo.saberDir);
    noteCutInfo->saberType = (int)cutInfo.saberType;
    noteCutInfo->timeDeviation = cutInfo.timeDeviation;
    noteCutInfo->cutDirDeviation = cutInfo.cutDirDeviation;
    noteCutInfo->cutPoint = new Vector3(cutInfo.cutPoint);
    noteCutInfo->cutNormal = new Vector3(cutInfo.cutNormal);
    noteCutInfo->cutDistanceToCenter = cutInfo.cutDistanceToCenter;
    noteCutInfo->cutAngle = cutInfo.cutAngle;
} 

MAKE_HOOK_MATCH(NoteCut, &ScoreController::HandleNoteWasCut, void, ScoreController* self, NoteController* noteController, ByRef<NoteCutInfo> noteCutInfo) {
    NoteCut(self, noteController, noteCutInfo);
    
    int noteId = _noteIdCache[noteController];

    NoteCutInfo derefCutInfo = noteCutInfo.heldRef;

    NoteEvent* noteEvent = _noteEventCache[noteId];
    noteEvent->eventTime = audioTimeSyncController->get_songTime();

    if (noteController->noteData->colorType == ColorType::None)
    {
        noteEvent->eventType = NoteEventType::bomb;
    }

    replay->notes.push_back(noteEvent);

    _cutInfoCache[noteId] = derefCutInfo;
    noteEvent->noteCutInfo = new ReplayNoteCutInfo();
    if (derefCutInfo.speedOK && derefCutInfo.directionOK && derefCutInfo.saberTypeOK && !derefCutInfo.wasCutTooSoon) {
        noteEvent->eventType = NoteEventType::good;
    } else {
        noteEvent->eventType = NoteEventType::bad;
        PopulateNoteCutInfo(noteEvent->noteCutInfo, derefCutInfo);
    }

    _swingIdCache[derefCutInfo.swingRatingCounter] = noteId;
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
            playerInfo->SetText(il2cpp_utils::createcsstr("#" + to_string(player->rank) + "       " + player->name + "         " + to_string_wprecision(player->pp, 2) + "pp"));
            
            if (plvc != NULL) {
                auto countryControl = plvc->scopeSegmentedControl->dataItems->get(2);
                countryControl->set_hintText(il2cpp_utils::createcsstr("Country"));
                countryControl->set_icon(GetCountryIcon(player->country));
                plvc->scopeSegmentedControl->SetData(plvc->scopeSegmentedControl->dataItems);
            }

        } else {
            playerInfo->SetText(il2cpp_utils::createcsstr(player->name + ", you are ready!"));
        }
    } else {
        playerInfo->SetText(il2cpp_utils::createcsstr(""));
    }
}

MAKE_HOOK_MATCH(LeaderboardActivate, &PlatformLeaderboardViewController::DidActivate, void, PlatformLeaderboardViewController* self, bool firstActivation, bool addedToHeirarchy, bool screenSystemEnabling) {
    LeaderboardActivate(self, firstActivation, addedToHeirarchy, screenSystemEnabling);
    if(firstActivation) {
        plvc = self;
        HMUI::ImageView* imageView = self->get_gameObject()->get_transform()->Find(il2cpp_utils::createcsstr("HeaderPanel"))->get_gameObject()->GetComponentInChildren<HMUI::ImageView*>();
        imageView->set_color(UnityEngine::Color(0.64,0.64,0.64,1));
        imageView->set_color0(UnityEngine::Color(0.93,0,0.55,1));
        imageView->set_color1(UnityEngine::Color(0.25,0.52,0.9,1));
    }
}

//Work in progress to get stuff above leaderboard
UnityEngine::GameObject* CreateCustomScreen(HMUI::ViewController* rootView, UnityEngine::Vector2 screenSize, UnityEngine::Vector3 position, float curvatureRadius) {
    auto gameObject = QuestUI::BeatSaberUI::CreateCanvas();
    auto screen = gameObject->AddComponent<HMUI::Screen*>();
    screen->rootViewController = rootView;
    auto curvedCanvasSettings = gameObject->AddComponent<HMUI::CurvedCanvasSettings*>();
    curvedCanvasSettings->SetRadius(curvatureRadius);

    auto transform = gameObject->get_transform();
    UnityEngine::GameObject* screenSystem = UnityEngine::GameObject::Find(il2cpp_utils::createcsstr("ScreenContainer"));
    if(screenSystem) {
        transform->set_position(screenSystem->get_transform()->get_position());
        screen->get_gameObject()->GetComponent<UnityEngine::RectTransform*>()->set_sizeDelta(screenSize);
    }
    return gameObject;
}

bool ssInstalled = true;
bool showBeatLeader = false;

MAKE_HOOK_MATCH(ScoreSaberDetector, &PlatformLeaderboardViewController::Refresh, void, PlatformLeaderboardViewController* self, bool firstActivation, bool addedToHierarchy) {
    ssInstalled = false;
    showBeatLeader = true;
}

MAKE_HOOK_MATCH(RefreshLeaderboard, &PlatformLeaderboardViewController::Refresh, void, PlatformLeaderboardViewController* self, bool firstActivation, bool addedToHierarchy) {
    if (showBeatLeader) {
    plvc->scores->Clear();
    plvc->leaderboardTableView->tableView->SetDataSource((HMUI::TableView::IDataSource *)plvc->leaderboardTableView, true);
    if (PlayerController::currentPlayer == NULL) {
        plvc->loadingControl->ShowText(il2cpp_utils::createcsstr("Please sign up or log in mod settings!"), true);
        return;
    }

    IPreviewBeatmapLevel* levelData = reinterpret_cast<IPreviewBeatmapLevel*>(plvc->difficultyBeatmap->get_level());
    if(!to_utf8(csstrtostr(levelData->get_levelID())).starts_with("custom_level")) {
        QuestUI::MainThreadScheduler::Schedule([] {
            plvc->loadingControl->Hide();
            plvc->hasScoresData = false;
            plvc->loadingControl->ShowText(il2cpp_utils::createcsstr("Leaderboards for this map are not supported!"), false);
            plvc->leaderboardTableView->tableView->SetDataSource((HMUI::TableView::IDataSource *)plvc->leaderboardTableView, true);
        });
        if (uploadStatus == NULL) {
            playerInfo = ::QuestUI::BeatSaberUI::CreateText(plvc->leaderboardTableView->get_transform(), "", false);
            move(playerInfo, 5, -26);
            if (PlayerController::currentPlayer != NULL) {
                updatePlayerInfoLabel();
            }

            websiteLinkImage = ::QuestUI::BeatSaberUI::CreateImage(plvc->leaderboardTableView->get_transform(), Sprites::get_BeatLeaderIcon(), UnityEngine::Vector2(-33, -24), UnityEngine::Vector2(12, 12));
            websiteLinkButton = ::QuestUI::BeatSaberUI::CreateUIButton(plvc->leaderboardTableView->get_transform(), "", UnityEngine::Vector2(-33, -24), UnityEngine::Vector2(12, 12), [](){
                string url = "https://beatleader.xyz/";
                if (PlayerController::currentPlayer != NULL) {
                    url += "u/" + PlayerController::currentPlayer->id;
                }
                UnityEngine::Application::OpenURL(il2cpp_utils::createcsstr(url));
            });
            retryButton = ::QuestUI::BeatSaberUI::CreateUIButton(plvc->leaderboardTableView->get_transform(), "Retry", UnityEngine::Vector2(30, -24), UnityEngine::Vector2(15, 8), [](){
                retryButton->get_gameObject()->SetActive(false);
                ReplayManager::RetryPosting(replayPostCallback);
            });
            retryButton->get_gameObject()->SetActive(false);
            retryButton->GetComponentInChildren<CurvedTextMeshPro*>()->set_alignment(TMPro::TextAlignmentOptions::Left);
            uploadStatus = ::QuestUI::BeatSaberUI::CreateText(plvc->leaderboardTableView->get_transform(), "", false);
            move(uploadStatus, 11, -32);
            resize(uploadStatus, 10, 0);
            uploadStatus->set_fontSize(3);
            uploadStatus->set_richText(true);
        }
        return;
    }
    string hash = regex_replace(to_utf8(csstrtostr(levelData->get_levelID())), basic_regex("custom_level_"), "");
    string difficulty = MapEnhancer::DiffName(plvc->difficultyBeatmap->get_difficulty().value);
    string mode = to_utf8(csstrtostr(plvc->difficultyBeatmap->get_parentDifficultyBeatmapSet()->get_beatmapCharacteristic()->serializedName));
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

    WebUtils::GetJSONAsync(url, [](long status, bool error, rapidjson::Document& result){
        auto scores = result.GetArray();
        plvc->scores->Clear();

        if ((int)scores.Size() == 0) {
            QuestUI::MainThreadScheduler::Schedule([status] {
                plvc->loadingControl->Hide();
                plvc->hasScoresData = false;
                plvc->loadingControl->ShowText(il2cpp_utils::createcsstr("No scores were found!"), true);
                
                plvc->leaderboardTableView->tableView->SetDataSource((HMUI::TableView::IDataSource *)plvc->leaderboardTableView, true);
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
                    il2cpp_utils::createcsstr(generateLabel(nameLabel, ppLabel, accLabel)), 
                    score["rank"].GetInt(), 
                    score["fullCombo"].GetBool());
                plvc->scores->Add(scoreData);
            }
        }
            
        plvc->leaderboardTableView->scores = plvc->scores;
        plvc->leaderboardTableView->specialScorePos = selectedScore;
        QuestUI::MainThreadScheduler::Schedule([] {
            plvc->loadingControl->Hide();
            plvc->hasScoresData = true;
            plvc->leaderboardTableView->tableView->SetDataSource((HMUI::TableView::IDataSource *)plvc->leaderboardTableView, true);
        });
    });

    plvc->loadingControl->ShowText(il2cpp_utils::createcsstr("Loading"), true);
    
    if (uploadStatus == NULL) {

        //UnityEngine::GameObject* parentScreen = CreateCustomScreen(self, UnityEngine::Vector2(100,50), self->screen->get_transform()->get_position(), 140);
        playerInfo = ::QuestUI::BeatSaberUI::CreateText(plvc->leaderboardTableView->get_transform(), "", false);
        move(playerInfo, 5, -26);
        if (PlayerController::currentPlayer != NULL) {
            updatePlayerInfoLabel();
        }

        websiteLinkImage = ::QuestUI::BeatSaberUI::CreateImage(plvc->leaderboardTableView->get_transform(), Sprites::get_BeatLeaderIcon(), UnityEngine::Vector2(-33, -24), UnityEngine::Vector2(12, 12));
        websiteLinkButton = ::QuestUI::BeatSaberUI::CreateUIButton(plvc->leaderboardTableView->get_transform(), "", UnityEngine::Vector2(-33, -24), UnityEngine::Vector2(12, 12), [](){
            string url = "https://beatleader.xyz/";
            if (PlayerController::currentPlayer != NULL) {
                url += "u/" + PlayerController::currentPlayer->id;
            }
            UnityEngine::Application::OpenURL(il2cpp_utils::createcsstr(url));
        });

        retryButton = ::QuestUI::BeatSaberUI::CreateUIButton(plvc->leaderboardTableView->get_transform(), "Retry", UnityEngine::Vector2(30, -24), UnityEngine::Vector2(15, 8), [](){
            retryButton->get_gameObject()->SetActive(false);
            ReplayManager::RetryPosting(replayPostCallback);
        });
        retryButton->get_gameObject()->SetActive(false);
        retryButton->GetComponentInChildren<CurvedTextMeshPro*>()->set_alignment(TMPro::TextAlignmentOptions::Left);
        uploadStatus = ::QuestUI::BeatSaberUI::CreateText(plvc->leaderboardTableView->get_transform(), "", false);
        move(uploadStatus, 11, -32);
        resize(uploadStatus, 10, 0);
        uploadStatus->set_fontSize(3);
        uploadStatus->set_richText(true);
    }
    } else {
        RefreshLeaderboard(self, firstActivation, addedToHierarchy);
    }

    if (ssInstalled && showBeatLeaderButton == NULL) {
        showBeatLeaderButton = ::QuestUI::BeatSaberUI::CreateToggle(self->get_gameObject()->get_transform()->Find(il2cpp_utils::createcsstr("HeaderPanel"))->get_transform(), "Show BL", UnityEngine::Vector2(0, 0), [](bool changed){
            showBeatLeader = !showBeatLeader;
            plvc->Refresh(true, true);
            if (uploadStatus != NULL) {
                websiteLinkButton->get_gameObject()->SetActive(showBeatLeader);
                websiteLinkImage->get_gameObject()->SetActive(showBeatLeader);
                playerInfo->get_gameObject()->SetActive(showBeatLeader);
                uploadStatus->get_gameObject()->SetActive(false);
                retryButton->get_gameObject()->SetActive(false);

                if (showBeatLeader) {
                    HMUI::ImageView* imageView = plvc->get_gameObject()->get_transform()->Find(il2cpp_utils::createcsstr("HeaderPanel"))->get_gameObject()->GetComponentInChildren<HMUI::ImageView*>();
                    imageView->set_color(UnityEngine::Color(0.64,0.64,0.64,1));
                    imageView->set_color0(UnityEngine::Color(0.93,0,0.55,1));
                    imageView->set_color1(UnityEngine::Color(0.25,0.52,0.9,1));
                }
            }
        });
        resize(showBeatLeaderButton, 170, 0);

        HMUI::ImageView* imageView = self->get_gameObject()->get_transform()->Find(il2cpp_utils::createcsstr("HeaderPanel"))->get_gameObject()->GetComponentInChildren<HMUI::ImageView*>();
        imageView->set_color(UnityEngine::Color(0.5,0.5,0.5,1));
        imageView->set_color0(UnityEngine::Color(0.5,0.5,0.5,1));
        imageView->set_color1(UnityEngine::Color(0.5,0.5,0.5,1));
    }
}

MAKE_HOOK_MATCH(LeaderboardCellSource, &LeaderboardTableView::CellForIdx, HMUI::TableCell*, LeaderboardTableView* self, HMUI::TableView* tableView, int row) {
    LeaderboardTableCell* result = (LeaderboardTableCell *)LeaderboardCellSource(self, tableView, row);

    if (showBeatLeader) {
        if (result->scoreText->get_fontSize() > 3) {
            result->playerNameText->set_enableAutoSizing(false);
            result->playerNameText->set_richText(true);
            resize(result->playerNameText, 10, 0);
            move(result->playerNameText, -2, 0);
            move(result->scoreText, 1, 0);
            move(result->fullComboText, 0.2, 0);
            result->playerNameText->set_fontSize(3);
            result->fullComboText->set_fontSize(3);
            result->scoreText->set_fontSize(3);
        }
    } else {
        if (result->scoreText->get_fontSize() == 3) {
            result->playerNameText->set_enableAutoSizing(true);
            resize(result->playerNameText, -10, 0);
            move(result->playerNameText, 2, 0);
            move(result->fullComboText, -0.2, 0);
            move(result->scoreText, -1, 0);
            result->playerNameText->set_fontSize(4);
            result->fullComboText->set_fontSize(4);
            result->scoreText->set_fontSize(4);
        }
    }
    
    return (TableCell *)result;
}

MAKE_HOOK_MATCH(Restart, &MenuTransitionsHelper::RestartGame, void, MenuTransitionsHelper* self, ::System::Action_1<::Zenject::DiContainer*>* finishCallback) {
    Restart(self, finishCallback);

    uploadStatus = NULL;
    plvc = NULL;
    leaderboardViewController = NULL;
    ResetLevelInfoUI();
}

// Called later on in the game loading - a good time to install function hooks
extern "C" void load() {
    il2cpp_functions::Init();

    LoggerContextObject logger = getLogger().WithContext("load");

    QuestUI::Init();
    QuestUI::Register::RegisterModSettingsViewController<BeatLeader::PreferencesViewController*>(modInfo, "BeatLeader");

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
    INSTALL_HOOK(logger, LeaderboardActivate);
    INSTALL_HOOK(logger, ScoreSaberDetector);
    
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

        LoggerContextObject logger = getLogger().WithContext("load");
        INSTALL_HOOK(logger, RefreshLeaderboard);
        INSTALL_HOOK(logger, LeaderboardCellSource);
    });

    ModifiersManager::Sync();
}