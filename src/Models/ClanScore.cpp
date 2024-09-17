#include "shared/Models/ClanScore.hpp"

ClanScore::ClanScore() {}

ClanScore::ClanScore(rapidjson::Value const& document) {
    auto const& clanObject = document["clan"];
    clan = Clan(clanObject);

    pp = document["pp"].GetFloat();
    rank = document["rank"].GetInt();
    modifiedScore = document["modifiedScore"].GetInt();
    timeset = document["timepost"].GetString();
    accuracy = document["accuracy"].GetFloat();
}