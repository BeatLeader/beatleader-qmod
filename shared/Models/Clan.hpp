#pragma once

#include "beatsaber-hook/shared/config/rapidjson-utils.hpp"

#include <optional>
#include <string>
using namespace std;

struct Clan
{
    string name;
    string tag;
    string icon;
    string color;

    int rank = 0;
    float pp = 0.0f;

    Clan(rapidjson::Value const& document);
    Clan() = default;
};

struct ClanRankingStatus
{
    std::optional<Clan> clan = std::nullopt;
    bool clanRankingContested = false;
    bool applicable = false;

    ClanRankingStatus(rapidjson::Value const& document);
    ClanRankingStatus() = default;
};
