#pragma once

#include "conditional-dependencies/shared/main.hpp"
#include "include/Core/CustomModifiers.hpp"

inline bool ReplayInstalled() noexcept {
    return !!CondDeps::FindUnsafe<bool, std::string>("replay", "PlayBSORFromFile");
}

inline bool PlayReplayFromFile(std::string path) noexcept {
    static auto function = CondDeps::FindUnsafe<bool, std::string>("replay", "PlayBSORFromFile");
    CustomModifiers::LoadReplayModifiers(path);
    return function.value()(path);
}

inline bool PlayReplayFromFileWithoutSettings(std::string path) noexcept {
    static auto function = CondDeps::FindUnsafe<bool, std::string>("replay", "PlayBSORFromFileForced");
    CustomModifiers::LoadReplayModifiers(path);
    return function.value()(path);
}

inline bool IsInReplay() noexcept {
    static auto function = CondDeps::FindUnsafe<bool>("replay", "IsInReplay");
    return function.value()();
}