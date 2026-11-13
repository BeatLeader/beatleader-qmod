#pragma once

#include "shared/Models/PlatformEvent.hpp"

#include "GlobalNamespace/BeatmapKey.hpp"

#include <functional>
#include <optional>
#include <vector>

namespace DanceEvent {
    inline constexpr int EventType = 2;

    std::optional<PlatformEvent> const& Current();
    bool IsEventLevel(GlobalNamespace::BeatmapKey const& key);

    void Load();
    void SetEvents(std::vector<PlatformEvent> const& events);
    void AddCurrentChangedListener(std::function<void()> listener);
}
