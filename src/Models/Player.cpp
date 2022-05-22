#include "include/Models/Player.hpp"

Player::Player(rapidjson::Value const& document) {
    id = document["id"].GetString();
    name = document["name"].GetString();
    country = document["country"].GetString();
    avatar = document["avatar"].GetString();
    role = document["role"].GetString();

    rank = document["rank"].GetInt();
    countryRank = document["countryRank"].GetInt();
    pp = document["pp"].GetFloat();

    auto clansList = document["clans"].GetArray();

    for (int index = 0; index < (int)clansList.Size(); ++index) {
        auto const& clan = clansList[index];
        clans.push_back(Clan(clan));
    }

    if (document.HasMember("patreonFeatures") && !document["patreonFeatures"].GetObject().ObjectEmpty()) {
        sponsorMessage = document["patreonFeatures"].GetObject()["message"].GetString();
    } else {
        sponsorMessage = "  ";
    }
    
}