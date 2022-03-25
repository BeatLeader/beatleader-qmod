#pragma once
#include "config-utils/shared/config-utils.hpp"

DECLARE_CONFIG(ModConfig,

    CONFIG_VALUE(ServerType, std::string, "ServerType", "Main", "");
    CONFIG_VALUE(Save, bool, "Keep local replays", true, "Save replays on this device");

    CONFIG_INIT_FUNCTION(
        CONFIG_INIT_VALUE(ServerType);
        CONFIG_INIT_VALUE(Save);
    )
)