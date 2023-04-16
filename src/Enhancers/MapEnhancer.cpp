#include "include/Enhancers/MapEnhancer.hpp"

#include "GlobalNamespace/BeatmapDifficulty.hpp"
#include "GlobalNamespace/IBeatmapLevel.hpp"
#include "GlobalNamespace/IDifficultyBeatmapSet.hpp"
#include "GlobalNamespace/BeatmapCharacteristicSO.hpp"
#include "GlobalNamespace/IPreviewBeatmapLevel.hpp"

#include <regex>
#include <sstream>

void MapEnhancer::Enhance(Replay &replay)
{
    ReplayInfo& info = replay.info;
    info.hash = regex_replace((string)previewBeatmapLevel->get_levelID(), basic_regex("custom_level_"), "");
    IPreviewBeatmapLevel* levelData = reinterpret_cast<IPreviewBeatmapLevel*>(difficultyBeatmap->get_level());
    info.songName = (string)levelData->get_songName();
    info.mapper = (string)levelData->get_levelAuthorName();
    info.difficulty = DiffName(difficultyBeatmap->get_difficulty().value);

    info.mode = (string)difficultyBeatmap->get_parentDifficultyBeatmapSet()->get_beatmapCharacteristic()->serializedName;
    info.environment = (string)environmentInfo->get_environmentName();
    info.modifiers = Join(Modifiers());
    info.leftHanded = playerSpecificSettings->leftHanded;
    info.height = playerSpecificSettings->automaticPlayerHeight ? 0 : playerSpecificSettings->playerHeight;

    if (practiceSettings != NULL)
    {
        info.startTime = practiceSettings->startSongTime;
        info.speed = practiceSettings->songSpeedMul;
    }
}

vector<string> MapEnhancer::Modifiers() const {
    vector<string> result;

    if (gameplayModifiers->disappearingArrows) { result.emplace_back("DA"); }
    if (gameplayModifiers->songSpeed == GameplayModifiers::SongSpeed::Faster) { result.emplace_back("FS"); }
    if (gameplayModifiers->songSpeed == GameplayModifiers::SongSpeed::Slower) { result.emplace_back("SS"); }
    if (gameplayModifiers->songSpeed == GameplayModifiers::SongSpeed::SuperFast) { result.emplace_back("SF"); }
    if (gameplayModifiers->ghostNotes) { result.emplace_back("GN"); }
    if (gameplayModifiers->noArrows) { result.emplace_back("NA"); }
    if (gameplayModifiers->noBombs) { result.emplace_back("NB"); }
    if (gameplayModifiers->noFailOn0Energy && energy == 0) { result.emplace_back("NF"); }
    if (gameplayModifiers->enabledObstacleType == GameplayModifiers::EnabledObstacleType::NoObstacles) { result.emplace_back("NO"); }
    if (gameplayModifiers->strictAngles) { result.emplace_back("SA"); }
    if (gameplayModifiers->proMode) { result.emplace_back("PM"); }
    if (gameplayModifiers->smallCubes) { result.emplace_back("SC"); }
    if (gameplayModifiers->failOnSaberClash) { result.emplace_back("CS"); }
    if (gameplayModifiers->instaFail) { result.emplace_back("IF"); }
    if (gameplayModifiers->energyType == GameplayModifiers::EnergyType::Battery) { result.emplace_back("BE"); }

    return result;
}

string MapEnhancer::DiffName(int diff) {
    switch (diff)
    {
    case 0:
        return "Easy";
        break;
    case 1:
        return "Normal";
        break;
    case 2:
        return "Hard";
        break;
    case 3:
        return "Expert";
        break;
    case 4:
        return "ExpertPlus";
        break;
    default:
        break;
    }
    return "Error";
}

string MapEnhancer::Join(span<string const> const list) {
    stringstream ss;

    for (auto const& s : list) {
        ss << s << ",";
    }


    auto str = ss.str();
    str.pop_back();

    return str;
}
