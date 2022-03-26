#include "include/Models/Player.hpp"

Player::Player(rapidjson::GenericObject<false, rapidjson::GenericValue<rapidjson::UTF8<>>>& document) {
    id = document["id"].GetString();
    name = document["name"].GetString();
    country = document["country"].GetString();
    avatar = document["avatar"].GetString();

    rank = document["rank"].GetInt();
    countryRank = document["countryRank"].GetInt();
    pp = document["pp"].GetFloat();
}

Player::Player() {}