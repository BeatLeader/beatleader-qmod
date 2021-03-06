#pragma once

#include "beatsaber-hook/shared/config/rapidjson-utils.hpp"
#include "include/Models/Clan.hpp"

#include <string>
using namespace std;

struct Player
{
    string id;
    string name;
    string country;
    string avatar;
    string role;
    int rank;
    int countryRank;
    float pp;
    vector<Clan> clans;
    string sponsorMessage;

    Player(rapidjson::Value const& document);
    Player() = default;
};
