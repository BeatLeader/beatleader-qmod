#pragma once

#include "GlobalNamespace/GameplayModifiers.hpp"

namespace BeatLeader {
    class LeaderboardContextSwitcher {
        public:
            static void HandleModifiersChanged(GlobalNamespace::GameplayModifiers* modifiers);
            static void HandleManualContextChange();
    };
}
