#pragma once
#include "config-utils/shared/config-utils.hpp"
#include "bs-utils/shared/utils.hpp"
#include "metacore/shared/game.hpp"
#include "include/UI/LeaderboardUI.hpp"

DECLARE_CONFIG(ModConfig) {
    CONFIG_VALUE(ServerType, std::string, "ServerType", "Main", "");
    CONFIG_VALUE(DomainType, int, "Domain Type", 0);
    CONFIG_VALUE(SaveLocalReplays, bool, "Keep local replays", true, "Save replays on this device");
    CONFIG_VALUE(AvatarsActive, bool, "Show Avatars", false);
    CONFIG_VALUE(ClansActive, bool, "Show Clans", true);
    CONFIG_VALUE(ScoresActive, bool, "Show Scores", true);
    CONFIG_VALUE(TimesetActive, bool, "Show Timeset", true);
    CONFIG_VALUE(IncognitoList, std::string, "Hidden players", "{}");
    CONFIG_VALUE(ShowReplaySettings, bool, "Show replay settings", true);
    CONFIG_VALUE(ShowBeatleader, bool, "Show BeatLeader", true, "Priority for BeatLeader or SS");
    CONFIG_VALUE(StarValueToShow, int, "Display Stars", 0);
    CONFIG_VALUE(Context, int, "Selected Context", static_cast<int>(LeaderboardUI::Context::Standard));
    CONFIG_VALUE(CaptureActive, bool, "Show Leaderboard clan capture status", true);
    CONFIG_VALUE(NoticeboardEnabled, bool, "Show Noticeboard in main menu", true);
};

inline bool UploadEnabled() {
    return bs_utils::Submission::getEnabled() && !MetaCore::Game::IsScoreSubmissionDisabled();
}

inline bool UploadDisabledByReplay() {
    for (auto kv : bs_utils::Submission::getDisablingMods()) {
        if (kv.id == "Replay") {
            return true;
        }
    }

    for (auto kv : MetaCore::Game::GetScoreSubmissionDisablers()) {
        if (kv == "Replay") {
            return true;
        }
    }
    return false;
}

inline std::string UploadDisablers() {
    std::vector<std::string> disablers;
    
    // Add bs-utils disablers
    auto map = bs_utils::Submission::getDisablingMods();
    for (auto kv : map) {
        disablers.push_back(kv.id);
    }

    // Add MetaCore disablers
    auto metacoreDisablers = MetaCore::Game::GetScoreSubmissionDisablers();
    for (auto disabler : metacoreDisablers) {
        // Only add if not already present
        if (std::find(disablers.begin(), disablers.end(), disabler) == disablers.end()) {
            disablers.push_back(disabler);
        }
    }

    std::string result = "Score submission disabled by ";
    for (int i = 0; i < disablers.size(); i++) {
        result += disablers[i];
        if (i < disablers.size() - 1) {
            result += ", ";
        }
    }
    return result;
}