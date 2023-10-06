#include "include/UI/Leaderboard.hpp"
#include "include/UI/LeaderboardViewController.hpp"

#include "questui/shared/BeatSaberUI.hpp"

namespace BeatLeader
{
    HMUI::ViewController* Leaderboard::get_leaderboardViewController()
    {
        if (!leaderboardViewController)
        {
            leaderboardViewController = QuestUI::BeatSaberUI::CreateViewController<LeaderboardUI::LeaderboardViewController*>();
        }
        return leaderboardViewController;
    }

    HMUI::ViewController* Leaderboard::get_panelViewController()
    {
        if (!panelViewController)
        {
            panelViewController = QuestUI::BeatSaberUI::CreateViewController();
        }
        return panelViewController;
    }
}