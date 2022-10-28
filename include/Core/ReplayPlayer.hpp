#include "conditional-dependencies/shared/main.hpp"

inline bool ReplayInstalled() noexcept {
    return !!CondDeps::FindUnsafe<bool, std::string>("replay", "PlayBSORFromFile");
}

inline bool PlayReplayFromFile(std::string path) noexcept {
    static auto function = CondDeps::FindUnsafe<bool, std::string>("replay", "PlayBSORFromFile");
    return function.value()(path);
}

inline bool PlayReplayFromFileWithoutSettings(std::string path) noexcept {
    static auto function = CondDeps::FindUnsafe<bool, std::string>("replay", "PlayBSORFromFileForced");
    return function.value()(path);
}

inline bool IsInReplay() noexcept {
    static auto function = CondDeps::FindUnsafe<bool>("replay", "IsInReplay");
    return function.value()();
}