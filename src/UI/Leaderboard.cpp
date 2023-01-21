#include "include/UI/Leaderboard.hpp"

#include "GlobalNamespace/PlatformLeaderboardViewController.hpp"
#include "questui/shared/BeatSaberUI.hpp"
#include "include/UI/PreferencesViewController.hpp"

namespace BeatLeader
{
    HMUI::ViewController* Leaderboard::get_leaderboardViewController()
    {
        if (!leaderboardViewController)
        {
            // test
            panelViewController = QuestUI::BeatSaberUI::CreateViewController<BeatLeader::PreferencesViewController*>();
            // panelViewController = QuestUI::BeatSaberUI::CreateViewController<GlobalNamespace::PlatformLeaderboardViewController*>();
        }
        return leaderboardViewController;
    }

    HMUI::ViewController* Leaderboard::get_panelViewController()
    {
        if (!panelViewController)
        {
            panelViewController = QuestUI::BeatSaberUI::CreateViewController<BeatLeader::PreferencesViewController*>();
        }
        return panelViewController;
    }
}