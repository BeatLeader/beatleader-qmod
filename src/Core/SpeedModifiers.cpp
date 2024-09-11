#include "Core/SpeedModifiers.hpp"

#include "beatsaber-hook/shared/utils/hooking.hpp"

#include "GlobalNamespace/BeatmapObjectSpawnMovementData.hpp"
#include "main.hpp"

MAKE_HOOK_MATCH(ObjectSpawnInit, &GlobalNamespace::BeatmapObjectSpawnMovementData::Init, void, GlobalNamespace::BeatmapObjectSpawnMovementData* self, 
                                                                    int noteLinesCount, 
                                                                    float startNoteJumpMovementSpeed, 
                                                                    float startBpm,
                                                                    GlobalNamespace::__BeatmapObjectSpawnMovementData__NoteJumpValueType noteJumpValueType, 
                                                                    float noteJumpValue,
                                                                    GlobalNamespace::IJumpOffsetYProvider* jumpOffsetYProvider, 
                                                                    UnityEngine::Vector3 rightVec, 
                                                                    UnityEngine::Vector3 forwardVec) {
    auto speed = SpeedModifiers::GetSongSpeedMultiplier();
    if(speed != 1) {                
        // We want to compensate the speed of the AudioTimeSyncController by doing the opposite with the NJS.
        startNoteJumpMovementSpeed = ((startNoteJumpMovementSpeed * speed - startNoteJumpMovementSpeed) / 2 + startNoteJumpMovementSpeed) / speed;
    }
    ObjectSpawnInit(self, noteLinesCount, startNoteJumpMovementSpeed, startBpm, noteJumpValueType, noteJumpValue, jumpOffsetYProvider, rightVec, forwardVec);
}

void SpeedModifiers::setup() {
    INSTALL_HOOK(BeatLeaderLogger, ObjectSpawnInit);
}