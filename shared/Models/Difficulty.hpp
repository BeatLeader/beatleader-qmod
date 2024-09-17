#pragma once

#include "beatsaber-hook/shared/config/rapidjson-utils.hpp"
#include "shared/Models/TriangleRating.hpp"
#include "shared/Models/Clan.hpp"

#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
using namespace std;

struct Difficulty 
{
    int status;
    int type;
    vector<float> votes;
    unordered_map<string, float> modifierValues;
    unordered_map<string, TriangleRating> modifiersRating;
    TriangleRating rating;
    ClanRankingStatus clanStatus;

    Difficulty(rapidjson::Value const& document);
    Difficulty(int statusGiven, int typeGiven, vector<float> votesGive, unordered_map<string, float> modifierValuesGiven, unordered_map<string, TriangleRating> modifiersRatingGiven, TriangleRating ratingGiven);
    Difficulty() = default;
};