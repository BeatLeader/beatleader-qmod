#include "Core/SpeedModifiers.hpp"
#include "Core/CustomModifiers.hpp"

#include "beatsaber-hook/shared/hooking.hpp"

#include "GlobalNamespace/AudioManager.hpp"
#include "GlobalNamespace/AudioTimeSyncController.hpp"
#include "GlobalNamespace/GameplayCoreInstaller.hpp"
#include "GlobalNamespace/GameplayCoreSceneSetupData.hpp"
#include "GlobalNamespace/NoteJumpSpeedEventData.hpp"
#include "GlobalNamespace/VariableMovementDataProvider.hpp"
#include "Zenject/DiContainer.hpp"

#include "metacore-bs/shared/game.hpp"

#include "main.hpp"

using namespace GlobalNamespace;

static float noteJumpSpeedMultiplier = 1;

static float SongSpeed() {
    if (CustomModifiers::IsActive(SpeedModifiers::BFSId)) {
        return 1.2f;
    }
    if (CustomModifiers::IsActive(SpeedModifiers::BSFId)) {
        return 1.5f;
    }
    return 1;
}

// get_songSpeedMul may be inlined, so the speed is set on what the installer bound instead
MAKE_HOOK_MATCH(SpeedGameplayCoreInstallerInstall, &GameplayCoreInstaller::InstallBindings, void, GameplayCoreInstaller* self) {
    CustomModifiers::BeginLevel();

    auto setupData = self->_sceneSetupData;
    float speed = setupData && !setupData->practiceSettings ? SongSpeed() : 1;
    float noteSpeed = 1 + (speed - 1) / 2;
    noteJumpSpeedMultiplier = noteSpeed / speed;

    SpeedGameplayCoreInstallerInstall(self);

    if (speed == 1) {
        return;
    }

    // Keeps BFS and BSF scores off other leaderboards, UploadEnabled ignores this disabler
    MetaCore::Game::DisableScoreSubmissionOnce(MOD_ID);

    auto initData = reinterpret_cast<AudioTimeSyncController_InitData*>(self->get_Container()->TryResolve(i2c::cs_type_of<AudioTimeSyncController_InitData*>()));
    if (!initData) {
        BeatLeaderLogger.error("Song speed modifier: no AudioTimeSyncController.InitData to apply {} to", speed);
        return;
    }
    initData->timeScale = speed;
    self->_audioManager->musicPitch = 1.0f / speed;
    BeatLeaderLogger.info("Song speed modifier: time scale {}, note jump speed x{}", speed, noteJumpSpeedMultiplier);
}

MAKE_HOOK_MATCH(VariableMovementDataProviderInit, &VariableMovementDataProvider::Init, void, VariableMovementDataProvider* self,
    float_t startHalfJumpDurationInBeats, float_t maxHalfJumpDistance, float_t noteJumpMovementSpeed, float_t minRelativeNoteJumpSpeed, float_t bpm,
    BeatmapObjectSpawnMovementData_NoteJumpValueType noteJumpValueType, float_t noteJumpValue, UnityEngine::Vector3 centerPosition,
    UnityEngine::Vector3 forwardVector) {
    if (noteJumpSpeedMultiplier != 1) {
        noteJumpMovementSpeed *= noteJumpSpeedMultiplier;
        minRelativeNoteJumpSpeed *= noteJumpSpeedMultiplier;
        BeatLeaderLogger.info("Song speed modifier: NJS {}, min relative NJS {}", noteJumpMovementSpeed, minRelativeNoteJumpSpeed);
    }
    VariableMovementDataProviderInit(self, startHalfJumpDurationInBeats, maxHalfJumpDistance, noteJumpMovementSpeed, minRelativeNoteJumpSpeed, bpm, noteJumpValueType, noteJumpValue, centerPosition, forwardVector);
}

MAKE_HOOK_MATCH(VariableMovementDataProviderHandleNJSEvent, &VariableMovementDataProvider::HandleNoteJumpMovementSpeedEvent, void, VariableMovementDataProvider* self, NoteJumpSpeedEventData* currentEventData) {
    VariableMovementDataProviderHandleNJSEvent(self, currentEventData);

    if (noteJumpSpeedMultiplier == 1) {
        return;
    }
    auto& interpolation = self->_relativeNoteJumpSpeedInterpolation;
    interpolation._fromValue *= noteJumpSpeedMultiplier;
    interpolation._toValue *= noteJumpSpeedMultiplier;
}

void SpeedModifiers::setup() {
    INSTALL_HOOK(BeatLeaderLogger, SpeedGameplayCoreInstallerInstall);
    INSTALL_HOOK(BeatLeaderLogger, VariableMovementDataProviderInit);
    INSTALL_HOOK(BeatLeaderLogger, VariableMovementDataProviderHandleNJSEvent);
}
