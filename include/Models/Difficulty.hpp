#pragma once

#include "beatsaber-hook/shared/config/rapidjson-utils.hpp"

#include <vector>
#include <string>
using namespace std;

struct Difficulty 
{
    float stars;
    int status;
    int type;
    vector<float> votes;

    Difficulty(rapidjson::Value const& document);
    Difficulty() = default;
};