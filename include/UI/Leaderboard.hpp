#pragma once

#include "leaderboardcore/shared/Models/CustomLeaderboard.hpp"

namespace BeatLeader
{
    class Leaderboard : public LeaderboardCore::Models::CustomLeaderboard
    {
        public:
            HMUI::ViewController* get_panelViewController() final override;
            HMUI::ViewController* get_leaderboardViewController() final override;
        private:
            HMUI::ViewController* panelViewController;
            HMUI::ViewController* leaderboardViewController;
    };
}