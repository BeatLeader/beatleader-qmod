#include "include/Managers/LeaderboardContextSwitcher.hpp"

#include "include/Core/CustomModifiers.hpp"
#include "include/Core/SpeedModifiers.hpp"
#include "include/Models/ScoresContext.hpp"
#include "include/UI/LeaderboardUI.hpp"
#include "include/Utils/ModConfig.hpp"

#include <optional>
#include <string>

using namespace GlobalNamespace;

namespace BeatLeader {
    static bool initialized = false;
    static std::optional<std::string> modifiersContextKey;

    static std::optional<std::string> GetContextKey(GameplayModifiers* modifiers) {
        if (CustomModifiers::IsEnabled(GolfModifier::Id)) return "golf";
        if (CustomModifiers::IsEnabled(DanceModifier::Id)) return "dnc";
        if (CustomModifiers::IsEnabled(SpeedModifiers::BSFId)) return "bsf";
        if (CustomModifiers::IsEnabled(SpeedModifiers::BFSId)) return "bfs";
        if (!modifiers) return std::nullopt;
        switch (modifiers->songSpeed) {
            case GameplayModifiers::SongSpeed::Slower: return "ss";
            case GameplayModifiers::SongSpeed::Faster: return "fs";
            case GameplayModifiers::SongSpeed::SuperFast: return "sf";
            default: return std::nullopt;
        }
    }

    static void SwitchTo(std::optional<std::string> const& key) {
        auto& config = getModConfig();
        if (!key) {
            auto manualContext = config.ManualContext.GetValue();
            if (manualContext < 0) return;
            config.ManualContext.SetValue(-1);
            LeaderboardUI::selectContext(manualContext);
            return;
        }
        ScoresContext* context = nullptr;
        for (auto candidate : ScoresContexts::allContexts) {
            if (candidate->key == *key) {
                context = candidate;
                break;
            }
        }
        if (!context) return;
        if (config.ManualContext.GetValue() < 0) {
            config.ManualContext.SetValue(config.Context.GetValue());
        }
        if (config.Context.GetValue() != context->id) {
            LeaderboardUI::selectContext(context->id);
        }
    }

    void LeaderboardContextSwitcher::HandleModifiersChanged(GameplayModifiers* modifiers) {
        auto key = GetContextKey(modifiers);
        if (initialized && key == modifiersContextKey) return;
        bool keepSessionStartContext = !initialized && getModConfig().ManualContext.GetValue() < 0;
        initialized = true;
        modifiersContextKey = key;
        if (!keepSessionStartContext) SwitchTo(key);
    }

    void LeaderboardContextSwitcher::HandleManualContextChange() {
        getModConfig().ManualContext.SetValue(-1);
    }
}
