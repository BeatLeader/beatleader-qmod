#pragma once

#include "include/Models/ScoresContext.hpp"
#include "include/Assets/BundleLoader.hpp"
#include "include/Utils/ModConfig.hpp"

namespace BeatLeader {
    class LeaderboardContextsManager {
        public:
            static void UpdateContexts(function<void()> const &completion);
    };
} 