#pragma once

#include "beatsaber-hook/shared/config/rapidjson-utils.hpp"
#include "include/Models/Player.hpp"

#include <string>
using namespace std;

class Score
{
public:
    // int id;
    // int baseScore;
    int modifiedScore;
    float accuracy;
    string playerId;
    float pp;
    // float Weight;
    int rank;
    int countryRank;
    // string Replay;
    string modifiers;
    int badCuts;
    int missedNotes;
    int bombCuts;
    int wallsHit;
    int pauses;
    bool fullCombo;
    int hmd;
    string timeset;

    Player player;
    Score();
    Score(rapidjson::GenericObject<false, rapidjson::GenericValue<rapidjson::UTF8<>>>& document);
};
