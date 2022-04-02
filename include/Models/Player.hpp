#pragma once

#include "beatsaber-hook/shared/config/rapidjson-utils.hpp"

#include <string>
using namespace std;

class Player
{
public:
    string id;
    string name;
    string country;
    string avatar;
    int rank;
    int countryRank;
    float pp;

    Player(rapidjson::GenericObject<false, rapidjson::GenericValue<rapidjson::UTF8<>>>& document);
    Player();
};
