#pragma once

#include "GlobalNamespace/IDifficultyBeatmap.hpp"
#include "GlobalNamespace/IPreviewBeatmapLevel.hpp"
#include "GlobalNamespace/PlayerSpecificSettings.hpp"
#include "GlobalNamespace/GameplayModifiers.hpp"
#include "GlobalNamespace/PracticeSettings.hpp"
#include "GlobalNamespace/EnvironmentInfoSO.hpp"
#include "GlobalNamespace/ColorScheme.hpp"

#include "include/Models/Replay.hpp"

using namespace GlobalNamespace;

class MapEnhancer
{
private:
    /* data */
public:
    MapEnhancer();
    ~MapEnhancer();
    IDifficultyBeatmap* difficultyBeatmap;
    IPreviewBeatmapLevel* previewBeatmapLevel;
    GameplayModifiers* gameplayModifiers;
    PlayerSpecificSettings* playerSpecificSettings;
    PracticeSettings* practiceSettings;
    bool useTestNoteCutSoundEffects;
    EnvironmentInfoSO* environmentInfo;
    ColorScheme* colorScheme;
    float energy;

    void Enhance(Replay* replay);
private:
    string Join(vector<string> list);
    vector<string> Modifiers();
    string DiffName(int diff);
};