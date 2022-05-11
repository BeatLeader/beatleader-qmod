#pragma once

#include "beatsaber-hook/shared/config/rapidjson-utils.hpp"

#include <string>
using namespace std;

struct Player
{
    string id;
    string name;
    string country;
    string avatar;
    int rank;
    int countryRank;
    float pp;

    Player(rapidjson::Value const& document);
    Player() = default;
};
