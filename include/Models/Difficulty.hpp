#pragma once

#include "beatsaber-hook/shared/config/rapidjson-utils.hpp"
#include "include/Models/ModifierRating.hpp"

#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
using namespace std;

struct Difficulty 
{
    float stars;
    int status;
    int type;
    vector<float> votes;
    unordered_map<string, float> modifierValues;
    unordered_map<string, ModifierRating> modifiersRating;
    float passRating;
    float accRating;
    float techRating;

    Difficulty(rapidjson::Value const& document);
    Difficulty(float starsGiven, int statusGiven, int typeGiven, vector<float> votesGive, unordered_map<string, float> modifierValuesGiven, unordered_map<string, ModifierRating> modifiersRatingGiven, float passRatingGiven, float accRatingGiven, float techRatingGiven);
    Difficulty() = default;
};