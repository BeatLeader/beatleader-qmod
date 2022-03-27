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
    // int ModifiedScore;
    // float Accuracy;
    // string PlayerId;
    // float Pp;
    // float Weight;
    // int Rank;
    // int CountryRank;
    // string Replay;
    // string Modifiers;
    // int BadCuts;
    // int MissedNotes;
    // int BombCuts;
    // int WallsHit;
    // int Pauses;
    // bool FullCombo;
    // int Hmd;
    // string Timeset;

    Player player;
    Score();
    Score(rapidjson::GenericObject<false, rapidjson::GenericValue<rapidjson::UTF8<>>>& document);
};
