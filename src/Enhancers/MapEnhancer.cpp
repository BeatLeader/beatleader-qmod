#include "include/Enhancers/MapEnhancer.hpp"
#include "GlobalNamespace/BeatmapDifficulty.hpp"
#include "GlobalNamespace/IBeatmapLevel.hpp"
#include "GlobalNamespace/IDifficultyBeatmapSet.hpp"
#include "GlobalNamespace/BeatmapCharacteristicSO.hpp"
#include "GlobalNamespace/IPreviewBeatmapLevel.hpp"

#include <regex>

MapEnhancer::MapEnhancer()
{
}

MapEnhancer::~MapEnhancer()
{
}

void MapEnhancer::Enhance(Replay* replay)
{
    ReplayInfo* info = replay->info;
    info->hash = regex_replace((string)previewBeatmapLevel->get_levelID(), basic_regex("\\custom_level_"), "");
    IPreviewBeatmapLevel* levelData = reinterpret_cast<IPreviewBeatmapLevel*>(difficultyBeatmap->get_level());
    info->songName = (string)levelData->get_songName();
    info->mapper = (string)levelData->get_levelAuthorName();
    info->difficulty = DiffName(difficultyBeatmap->get_difficulty().value);

    info->mode = (string)difficultyBeatmap->get_parentDifficultyBeatmapSet()->get_beatmapCharacteristic()->serializedName;
    info->environment = (string)environmentInfo->get_environmentName();
    info->modifiers = Join(Modifiers());
    info->leftHanded = playerSpecificSettings->leftHanded;
    info->height = playerSpecificSettings->automaticPlayerHeight ? 0 : playerSpecificSettings->playerHeight;

    if (practiceSettings != NULL)
    {
        info->startTime = practiceSettings->startSongTime;
        info->speed = practiceSettings->songSpeedMul;
    }
}

vector<string> MapEnhancer::Modifiers() {
    vector<string> result;

    if (gameplayModifiers->disappearingArrows) { result.push_back("DA"); }
    if (gameplayModifiers->songSpeed == GameplayModifiers::SongSpeed::Faster) { result.push_back("FS"); }
    if (gameplayModifiers->songSpeed == GameplayModifiers::SongSpeed::Slower) { result.push_back("SS"); }
    if (gameplayModifiers->songSpeed == GameplayModifiers::SongSpeed::SuperFast) { result.push_back("SF"); }
    if (gameplayModifiers->ghostNotes) { result.push_back("GN"); }
    if (gameplayModifiers->noArrows) { result.push_back("NA"); }
    if (gameplayModifiers->noBombs) { result.push_back("NB"); }
    if (gameplayModifiers->noFailOn0Energy && energy == 0) { result.push_back("NF"); }
    if (gameplayModifiers->enabledObstacleType == GameplayModifiers::EnabledObstacleType::NoObstacles) { result.push_back("NO"); }

    return result;
}

string MapEnhancer::DiffName(int diff) {
    switch (diff)
    {
    case 1:
        return "Easy";
        break;
    case 3:
        return "Normal";
        break;
    case 5:
        return "Hard";
        break;
    case 7:
        return "Expert";
        break;
    case 9:
        return "ExpertPlus";
        break;
    default:
        break;
    }
    return "Error";
}

string MapEnhancer::Join(vector<string> list) {
    string result = "";
    for(size_t i = 0; i < list.size(); ++i) {
        result += list[i];
        result += ",";
    }

    result.pop_back();

    return result;
}
