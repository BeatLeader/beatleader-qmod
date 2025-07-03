#pragma once
#include "config-utils/shared/config-utils.hpp"
#include "bs-utils/shared/utils.hpp"
#include "include/UI/LeaderboardUI.hpp"

DECLARE_CONFIG(ModConfig,
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
    CONFIG_VALUE(ExperienceBarEnabled, bool, "Show experience bar", true);
)

inline bool UploadEnabled() {
    return bs_utils::Submission::getEnabled();
}

inline bool UploadDisabledByReplay() {
    for (auto kv : bs_utils::Submission::getDisablingMods()) {
        if (kv.id == "Replay") {
            return true;
        }
    } 
    return false;
}

inline std::string UploadDisablers() {
    auto map = bs_utils::Submission::getDisablingMods();
    std::string result = "Score submission disabled by ";
    int counter = 0;
    int size = map.size();

    for (auto kv : map) {
        counter++;
        result += kv.id + (counter != size ? ", " : "");
    } 
    return result;
}