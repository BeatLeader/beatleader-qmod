#pragma once

#include "beatsaber-hook/shared/config/rapidjson-utils.hpp"
#include "include/Models/Player.hpp"

#include <string>
using namespace std;

struct ScoreImprovement
{
    int score = 0;
    float accuracy;
    int rank;
    float pp;

    int totalRank;
    float totalPp;

    ScoreImprovement();
    ScoreImprovement(rapidjson::Value const& document);
};

struct Score
{
    int id;
    // int baseScore;
    int modifiedScore;
    float accuracy;
    string playerId;
    float pp;
    // float Weight;
    int rank;
    int countryRank;
    string replay;
    string modifiers;
    int badCuts;
    int missedNotes;
    int bombCuts;
    int wallsHit;
    int pauses;
    bool fullCombo;
    int hmd;
    string timeset;
    string platform;

    Player player;
    ScoreImprovement scoreImprovement;
    Score();
    Score(rapidjson::Value const& document);
};
