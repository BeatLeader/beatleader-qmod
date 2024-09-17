#pragma once

#include "beatsaber-hook/shared/config/rapidjson-utils.hpp"
#include "shared/Models/Difficulty.hpp"

#include <string>
#include <map>
using namespace std;

struct Song 
{
    map<string, Difficulty> difficulties = {};

    Song(rapidjson::Value const& document);
    Song() = default;
};