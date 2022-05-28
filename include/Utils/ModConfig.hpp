#pragma once

#include "UI/QuestUIExtensions.hpp"
#include "config-utils/shared/config-utils.hpp"

DECLARE_CONFIG(ModConfig,

    CONFIG_VALUE(ServerType, std::string, "ServerType", "Main", "");
    CONFIG_VALUE(Save, bool, "Keep local replays", true, "Save replays on this device");
    CONFIG_VALUE(ShowBeatleader, bool, "Show BeatLeader", false, "Priority for BeatLeader or SS");
    CONFIG_VALUE(Modifiers, bool, "Positive modifiers", true, "Show leaderboards with positive modifiers");

    CONFIG_INIT_FUNCTION(
        CONFIG_INIT_VALUE(ServerType);
        CONFIG_INIT_VALUE(Save);
        CONFIG_INIT_VALUE(ShowBeatleader);
        CONFIG_INIT_VALUE(Modifiers);
    )
)

inline bool UploadEnabled() {
    char* value = getenv("disable_ss_upload");

    return value == NULL || strcmp(value, "0") == 0;
}