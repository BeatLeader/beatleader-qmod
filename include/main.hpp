#pragma once

#include "_config.hpp"

#include "scotland2/shared/loader.hpp"

// beatsaber-hook is a modding framework that lets us call functions and fetch field values from in the game
// It also allows creating objects, configuration, and importantly, hooking methods to modify their values
#include "beatsaber-hook/shared/utils/logging.hpp"
#include "beatsaber-hook/shared/config/config-utils.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-functions.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"

#include "paper/shared/logger.hpp"

inline modloader::ModInfo modInfo = {MOD_ID, VERSION, 0};

constexpr auto BeatLeaderLogger = Paper::ConstLoggerContext("BeatLeader");