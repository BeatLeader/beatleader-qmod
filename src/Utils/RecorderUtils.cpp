
#include "include/Utils/RecorderUtils.hpp"

#include "GlobalNamespace/SinglePlayerLevelSelectionFlowCoordinator.hpp"
#include "GlobalNamespace/PracticeViewController.hpp"
#include "GlobalNamespace/PauseMenuManager.hpp"
#include "GlobalNamespace/ResultsViewController.hpp"
#include "GlobalNamespace/StandardLevelFailedController.hpp"
#include "GlobalNamespace/StandardLevelFailedController_InitData.hpp"
#include "GlobalNamespace/GameScenesManager.hpp"
#include "GlobalNamespace/GameScenesManager_ScenePresentType.hpp"
#include "GlobalNamespace/GameScenesManager_SceneDismissType.hpp"

#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "beatsaber-hook/shared/utils/logging.hpp"

#include "main.hpp"

using namespace GlobalNamespace;
using namespace std;

bool RecorderUtils::shouldRecord;
bool buffer = false;

static void OnRestartPauseButtonWasPressed() {
    RecorderUtils::shouldRecord = buffer;
    buffer = false;
}

static void OnActionButtonWasPressed() {
    RecorderUtils::shouldRecord = true;
}

static void OnSceneTransitionStarted(bool menuToGame, bool gameToMenu) {
    if (menuToGame || gameToMenu) return; // only transition related to Standard gameplay
    RecorderUtils::shouldRecord = false;
}

/*
* A set of patches that check if the user initiated a transition to the standard game scene through interaction with the default UI elements.
* All transitions beyond these are forbidden and will not trigger the replay recorder.
* Allowed flows for now are:
*  - Press 'Play'    button on a level select screen
*  - Press 'Play'    button on a practice setup screen
*  - Press 'Restart' button on a game pause sceen
*  - Press 'Restart' button on a level result details screen
*  - Failing a map with the enabled 'AutoRestart' option is considered as a restart button action in a game pause screen
*
*  This should prevent activation of the replay recorder for any unknown ways to run GameCore scene.
*/

// Play button from a level selection screen
// MAKE_HOOK_MATCH(ActionButtonWasPressed, &SinglePlayerLevelSelectionFlowCoordinator::ActionButtonWasPressed, void, SinglePlayerLevelSelectionFlowCoordinator* self) {
//     ActionButtonWasPressed(self);
//     OnActionButtonWasPressed();
// }

// // Play button from a practice mode setting screen
// MAKE_HOOK_MATCH(PlayButtonPressed, &PracticeViewController::PlayButtonPressed, void, PracticeViewController* self) {
//     PlayButtonPressed(self);
//     OnActionButtonWasPressed();
// }

// // Restart button from a GameCore paused screen
// MAKE_HOOK_MATCH(RestartButtonPressed, &PauseMenuManager::RestartButtonPressed, void, PauseMenuManager* self) {
//     RestartButtonPressed(self);
//     OnRestartPauseButtonWasPressed();
// }

// // Restart button from a level result screen
// MAKE_HOOK_MATCH(RestartButtonPressed2, &ResultsViewController::RestartButtonPressed, void, ResultsViewController* self) {
//     RestartButtonPressed2(self);
//     OnActionButtonWasPressed();
// }

// MAKE_HOOK_MATCH(LevelFailedCoroutine, &StandardLevelFailedController::LevelFailedCoroutine, System::Collections::IEnumerator*, StandardLevelFailedController* self) {
    
//     if (self->initData->autoRestart) {
//         OnRestartPauseButtonWasPressed();
//     }

//     return LevelFailedCoroutine(self);
// }

// MAKE_HOOK_MATCH(ScenesTransitionCoroutine, &GameScenesManager::ScenesTransitionCoroutine, System::Collections::IEnumerator*, GameScenesManager* self, ::GlobalNamespace::ScenesTransitionSetupDataSO* newScenesTransitionSetupData, ::System::Collections::Generic::List_1<::StringW>* scenesToPresent, ::GlobalNamespace::GameScenesManager::ScenePresentType presentType, ::System::Collections::Generic::List_1<::StringW>* scenesToDismiss, ::GlobalNamespace::GameScenesManager::SceneDismissType dismissType, float minDuration, ::System::Action* afterMinDurationCallback, ::System::Action_1<::Zenject::DiContainer*>* extraBindingsCallback, ::System::Action_1<::Zenject::DiContainer*>* finishCallback) {
    
//     bool menuToGame = scenesToDismiss->Contains("MainMenu") && scenesToPresent->Contains("StandardGameplay");
//     bool gameToMenu = scenesToDismiss->Contains("StandardGameplay") && scenesToPresent->Contains("MainMenu");

//     OnSceneTransitionStarted(menuToGame, gameToMenu);

//     return ScenesTransitionCoroutine(self, newScenesTransitionSetupData, scenesToPresent, presentType, scenesToDismiss, dismissType, minDuration, afterMinDurationCallback, extraBindingsCallback, finishCallback);
// }

void RecorderUtils::StartRecorderUtils() {
    LoggerContextObject logger = getLogger().WithContext("load");

    // INSTALL_HOOK(logger, ActionButtonWasPressed);
    // INSTALL_HOOK(logger, PlayButtonPressed);
    // INSTALL_HOOK(logger, RestartButtonPressed);
    // INSTALL_HOOK(logger, RestartButtonPressed2);
    // INSTALL_HOOK(logger, LevelFailedCoroutine);
    // INSTALL_HOOK(logger, ScenesTransitionCoroutine);
}