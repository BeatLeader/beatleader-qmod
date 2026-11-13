#pragma once

#include "include/Core/Dance/DanceScore.hpp"
#include "shared/Models/Replay.hpp"

#include "Zenject/DiContainer.hpp"
#include "GlobalNamespace/GameplayCoreSceneSetupData.hpp"

#include <optional>

namespace DanceModeTracker {
    void Start(Zenject::DiContainer* container, GlobalNamespace::GameplayCoreSceneSetupData* setupData);
    void Stop();

    void AddFrame(Frame const& frame);
    void AddPause(float songTime);
    void Tick();
    void Finish();

    bool RanThisLevel();
    std::optional<Dance::DanceLive> Live();
    std::optional<Dance::DanceResult> const& LastResult();

    void setup();
}
