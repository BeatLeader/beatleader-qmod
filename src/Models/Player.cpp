#include "include/Models/Player.hpp"

Player::Player(rapidjson::Value const& document) {
    id = document["id"].GetString();
    name = document["name"].GetString();
    country = document["country"].GetString();
    avatar = document["avatar"].GetString();

    rank = document["rank"].GetInt();
    countryRank = document["countryRank"].GetInt();
    pp = document["pp"].GetFloat();
}