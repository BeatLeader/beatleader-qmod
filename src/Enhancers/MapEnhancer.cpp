#include "include/Enhancers/MapEnhancer.hpp"

#include "GlobalNamespace/BeatmapDifficulty.hpp"
#include "GlobalNamespace/BeatmapCharacteristicSO.hpp"
#include "GlobalNamespace/BeatmapLevel.hpp"

#include "ModifiersCoreQuest/shared/Core/ModifiersManager.hpp"
#include "ModifiersCoreQuest/shared/Utils/ModifierUtils.hpp"

#include "conditional-dependencies/shared/main.hpp"
#include "main.hpp"

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

    static auto reBeatEnabledFunc = CondDeps::Find<bool>("rebeat", "GetEnabled");
    static auto reBeatSameColorFunc = CondDeps::Find<bool>("rebeat", "GetSameColor");
    static auto reBeatEasyFunc = CondDeps::Find<bool>("rebeat", "GetEasyMode");
    static auto reBeatOneHpFunc = CondDeps::Find<bool>("rebeat", "GetOneHP");

    bool reBeatEnabled = reBeatEnabledFunc.has_value() && reBeatEnabledFunc.value()();
    bool reBeatSameColor = reBeatSameColorFunc.has_value() && reBeatSameColorFunc.value()();
    bool reBeatEasy = reBeatEasyFunc.has_value() && reBeatEasyFunc.value()();
    bool reBeatOneHp = reBeatOneHpFunc.has_value() && reBeatOneHpFunc.value();

    if (reBeatEnabled) {
        static auto reBeatHiddenFunc = CondDeps::Find<bool>("rebeat", "GetHidden");
        if (reBeatHiddenFunc.has_value() && reBeatHiddenFunc.value()()) {
            result.emplace_back("HD");
        }
    }

    for(auto m : ModifiersCoreQuest::ModifiersManager::get_Modifiers()) {
        if(ModifiersCoreQuest::ModifiersManager::GetModifierState(m.Id)) {
            result.emplace_back(m.Id);
        }
    }

    // ReBeat Modifier Support

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
