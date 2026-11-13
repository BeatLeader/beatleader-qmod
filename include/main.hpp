#pragma once

#include "Utils/ReplayManager.hpp"
#include "_config.hpp"

#include "scotland2/shared/loader.hpp"
#include "EventCallback/invokable.hpp"
#include "Models/ScoreUpload.hpp"

// beatsaber-hook is a modding framework that lets us call functions and fetch field values from in the game
// It also allows creating objects, configuration, and importantly, hooking methods to modify their values
#include "paper2_scotland2/shared/logger.hpp"
#include "beatsaber-hook/shared/utils.hpp"
#include "beatsaber-hook/shared/api.hpp"
#include "beatsaber-hook/shared/hooking.hpp"

#include "paper2_scotland2/shared/logger.hpp"

inline modloader::ModInfo modInfo = {MOD_ID, VERSION, 0};

constexpr auto BeatLeaderLogger = Paper::ConstLoggerContext("BeatLeader");

namespace BeatLeader 
{
    using UploadStateFunc = std::function<void(std::optional<ScoreUpload>, ReplayUploadStatus)>;
    using UploadStateCallbackFunc = callback<std::optional<ScoreUpload>, ReplayUploadStatus>;
    extern invokable<std::optional<ScoreUpload>, ReplayUploadStatus> UploadStateCallback;
}