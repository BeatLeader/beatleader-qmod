#pragma once

#include "include/Core/Dance/DanceScore.hpp"

#include <optional>

namespace Zenject {
    class DiContainer;
}

namespace GlobalNamespace {
    class GameplayCoreSceneSetupData;
}

namespace DanceDread {
    inline constexpr double DreadLine = 0.85;

    struct Summary {
        bool Died = false;
        float HauntedSeconds = 0;
        float PlayedSeconds = 0;
        int Escapes = 0;
        float DeathTime = 0;
        float LongestStillSeconds = 0;
    };

    bool IsHaunting();
    bool IsWarning();
    std::optional<Summary> const& LastSummary();

    void Prepare(Zenject::DiContainer* container, GlobalNamespace::GameplayCoreSceneSetupData* setupData);
    void Update(Dance::DanceLive const& live);
    void Stop();

    void setup();
}
