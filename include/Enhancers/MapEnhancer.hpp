#pragma once

#include "GlobalNamespace/BeatmapKey.hpp"
#include "GlobalNamespace/BeatmapLevel.hpp"
#include "GlobalNamespace/PlayerSpecificSettings.hpp"
#include "GlobalNamespace/GameplayModifiers.hpp"
#include "GlobalNamespace/PracticeSettings.hpp"
#include "GlobalNamespace/EnvironmentInfoSO.hpp"
#include "GlobalNamespace/ColorScheme.hpp"

#include "include/Models/Replay.hpp"

using namespace GlobalNamespace;

class MapEnhancer
{
public:
    BeatmapKey difficultyBeatmap;
    BeatmapLevel* beatmapLevel;
    GameplayModifiers* gameplayModifiers;
    PlayerSpecificSettings* playerSpecificSettings;
    PracticeSettings* practiceSettings;
    bool useTestNoteCutSoundEffects;
    EnvironmentInfoSO* environmentInfo;
    ColorScheme* colorScheme;
    float energy;

    void Enhance(Replay &replay);
    static string DiffName(int diff);
private:
    static string Join(span<string const> list);
    [[nodiscard]] vector<string> Modifiers() const;
};