#pragma once

#include "beatsaber-hook/shared/config/rapidjson-utils.hpp"

#include <string>
using namespace std;

struct Clan
{
    string name;
    string tag;
    string icon;
    string color;

    int rank;
    float pp;

    Clan(rapidjson::Value const& document);
    Clan() = default;
};

struct ClanRankingStatus
{
    std::optional<Clan> clan;
    bool clanRankingContested;
    bool applicable;

    ClanRankingStatus(rapidjson::Value const& document);
    ClanRankingStatus() = default;
};
