#pragma once

#include "beatsaber-hook/shared/config/rapidjson-utils.hpp"

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

    Difficulty(rapidjson::Value const& document);
    Difficulty(float starsGiven, int statusGiven, int typeGiven, vector<float> votesGive, unordered_map<string, float> modifierValuesGiven);
    Difficulty() = default;
};