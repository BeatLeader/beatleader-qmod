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
    info.environment = (string)environmentName._environmentName;
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

    static auto reBeatEnabledFunc = CondDeps::Find<bool>("rebeat", "GetEnabled");
    static auto reBeatHiddenFunc = CondDeps::Find<bool>("rebeat", "GetHidden");
    static auto reBeatSameColorFunc = CondDeps::Find<bool>("rebeat", "GetSameColor");
    static auto reBeatEasyFunc = CondDeps::Find<bool>("rebeat", "GetEasyMode");
    static auto reBeatOneHpFunc = CondDeps::Find<bool>("rebeat", "GetOneHP");

    bool reBeatEnabled = reBeatEnabledFunc.has_value() && reBeatEnabledFunc.value()();
    bool reBeatHidden = reBeatHiddenFunc.has_value() && reBeatHiddenFunc.value()();
    bool reBeatSameColor = reBeatSameColorFunc.has_value() && reBeatSameColorFunc.value()();
    bool reBeatEasy = reBeatEasyFunc.has_value() && reBeatEasyFunc.value()();
    bool reBeatOneHp = reBeatOneHpFunc.has_value() && reBeatOneHpFunc.value();

    if (gameplayModifiers->disappearingArrows) { result.emplace_back("DA"); }
    if (gameplayModifiers->songSpeed == GameplayModifiers::SongSpeed::Faster) { result.emplace_back("FS"); }
    if (gameplayModifiers->songSpeed == GameplayModifiers::SongSpeed::Slower) { result.emplace_back("SS"); }
    if (gameplayModifiers->songSpeed == GameplayModifiers::SongSpeed::SuperFast) { result.emplace_back("SF"); }
    if (gameplayModifiers->ghostNotes && !(reBeatEnabled && reBeatHidden)) { result.emplace_back("GN"); }
    if (gameplayModifiers->noArrows) { result.emplace_back("NA"); }
    if (gameplayModifiers->noBombs) { result.emplace_back("NB"); }
    if (gameplayModifiers->noFailOn0Energy && energy == 0) { result.emplace_back("NF"); }
    if (gameplayModifiers->enabledObstacleType == GameplayModifiers::EnabledObstacleType::NoObstacles) { result.emplace_back("NO"); }
    if (gameplayModifiers->strictAngles) { result.emplace_back("SA"); }
    if (gameplayModifiers->proMode) { result.emplace_back("PM"); }
    if (gameplayModifiers->smallCubes) { result.emplace_back("SC"); }
    if (gameplayModifiers->failOnSaberClash) { result.emplace_back("CS"); }
    if (gameplayModifiers->instaFail) { result.emplace_back("IF"); }
    if (gameplayModifiers->energyType == GameplayModifiers::EnergyType::Battery && !reBeatEnabled) { result.emplace_back("BE"); }

    // ReBeat Modifier Support

    if (reBeatEnabled && reBeatHidden) { result.emplace_back("HD"); }
    if (reBeatEnabled && reBeatSameColor) { result.emplace_back("SMC"); }
    if (reBeatEnabled && reBeatEasy) { result.emplace_back("EZ"); }
    if (reBeatEnabled && reBeatOneHp) { result.emplace_back("OHP"); }

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
