#pragma once

#include <string>
#include <unordered_map>

namespace DanceModifier {
    inline const std::string Id = "DNC";
}

namespace GolfModifier {
    inline const std::string Id = "GOLF";
}

namespace NegativeModifiers {
    void UpdateExclusiveModifiers(std::unordered_map<std::string, float> const& modifierValues);
    bool HasNegativeModifiers(std::string const& id);
}

namespace CustomModifiers {
    void Register();
    bool IsEnabled(std::string const& id);

    void LoadReplayModifiers(std::string const& replayPath);
    void BeginLevel();
    bool IsActive(std::string const& id);
}
