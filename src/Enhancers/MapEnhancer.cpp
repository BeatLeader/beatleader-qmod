#include "include/Enhancers/MapEnhancer.hpp"

#include "GlobalNamespace/BeatmapDifficulty.hpp"
#include "GlobalNamespace/BeatmapCharacteristicSO.hpp"
#include "GlobalNamespace/BeatmapLevel.hpp"

#include "conditional-dependencies/shared/main.hpp"

#include <regex>
#include <sstream>

void MapEnhancer::Enhance(Replay &replay)
{
    ReplayInfo& info = replay.info;
    info.hash = regex_replace((string)beatmapLevel->levelID, basic_regex("custom_level_"), "");
    info.songName = (string)beatmapLevel->songName;
    info.mapper = (string)beatmapLevel->songAuthorName;
    info.difficulty = DiffName(difficultyBeatmap.difficulty.value__);

    info.mode = (string)difficultyBeatmap.beatmapCharacteristic->serializedName;
    info.environment = (string)environmentInfo->environmentName;
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

    // ReBeat Modifier Support

    static auto enabledFunc = CondDeps::Find<bool>("rebeat", "GetEnabled");
    if (enabledFunc.has_value() && enabledFunc.value()()) {

        // Remove Battery Energy modifier (custom energy system)
        auto itr = std::find(result.begin(), result.end(), "BE");
        if (itr != result.end()) result.erase(itr);

        // Use Hidden instead of Ghost Notes
        static auto hiddenFunc = CondDeps::Find<bool>("rebeat", "GetHidden");
        if (hiddenFunc.has_value() && hiddenFunc.value()()) {
            auto itr = std::find(result.begin(), result.end(), "GN");
            if (itr != result.end()) result.erase(itr);
            result.emplace_back("HD");
        }

        // SameColor(SC)
        static auto sameColorFunc = CondDeps::Find<bool>("rebeat", "GetSameColor");
        if (sameColorFunc.has_value() && sameColorFunc.value()()) {
            result.emplace_back("SC");
        }

        // Easy mode (EZ)
        static auto easyFunc = CondDeps::Find<bool>("rebeat", "GetEasyMode");
        if (easyFunc.has_value() && easyFunc.value()()) {
            result.emplace_back("EZ");
        }

        // One HP (OHP)
        static auto oneHpFunc = CondDeps::Find<bool>("rebeat", "GetOneHp");
        if (oneHpFunc.has_value() && oneHpFunc.value()) {
            result.emplace_back("OHP");
        }
    }

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
