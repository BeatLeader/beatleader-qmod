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
    // int BadCuts;
    // int MissedNotes;
    // int BombCuts;
    // int WallsHit;
    // int Pauses;
    bool fullCombo;
    // int Hmd;
    // string Timeset;

    Player player;
    Score();
    Score(rapidjson::GenericObject<false, rapidjson::GenericValue<rapidjson::UTF8<>>>& document);
};
