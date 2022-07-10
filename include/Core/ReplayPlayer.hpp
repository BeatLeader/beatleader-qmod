#include "conditional-dependencies/shared/main.hpp"

inline bool ReplayInstalled() noexcept {
    return !!CondDeps::FindUnsafe<bool, std::string>("replay", "PlayBSORFromFile");
}

inline bool PlayReplayFromFile(std::string path) noexcept {
    static auto function = CondDeps::FindUnsafe<bool, std::string>("replay", "PlayBSORFromFile");
    return function.value()(path);
}