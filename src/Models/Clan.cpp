#include "shared/Models/Clan.hpp"

Clan::Clan(rapidjson::Value const& document) {
    tag = document["tag"].GetString();
    color = document["color"].GetString();

    if (document.HasMember("name") && !document["name"].IsNull()) {
        name = document["name"].GetString();
    } else {
        name = "No name";
    }

    if (document.HasMember("avatar")) {
        icon = document["avatar"].GetString();
    } else {
        icon = "";
    }
    if (document.HasMember("rank")) {
        rank = document["rank"].GetInt();
    }
    if (document.HasMember("pp")) {
        pp = document["pp"].GetFloat();
    }
}

ClanRankingStatus::ClanRankingStatus(rapidjson::Value const& document) {
    if (document["clan"].IsNull()) {
        clan = nullopt;
    } else {
        clan = Clan(document["clan"].GetObject());
    }

    applicable = document["applicable"].GetBool();
    clanRankingContested = document["clanRankingContested"].GetBool();
}