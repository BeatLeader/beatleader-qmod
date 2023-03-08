#include "include/Models/Player.hpp"

#include "include/Utils/ModConfig.hpp"
#include "include/main.hpp"

Player::Player(rapidjson::Value const& userModInterface) {
    getLogger().info("CreatePlayer");
    id = userModInterface["id"].GetString();
    name = userModInterface["name"].GetString();
    country = userModInterface["country"].GetString();
    avatar = userModInterface["avatar"].GetString();
    role = userModInterface["role"].GetString();

    rank = userModInterface["rank"].GetInt();
    countryRank = userModInterface["countryRank"].GetInt();
    pp = userModInterface["pp"].GetFloat();

    auto clansList = userModInterface["clans"].GetArray();
    for (int index = 0; index < (int)clansList.Size(); ++index) {
        auto const& clan = clansList[index];
        clans.push_back(Clan(clan));
    }

    if (userModInterface.HasMember("friends") && !userModInterface["friends"].IsNull()) {
        auto friendsList = userModInterface["friends"].GetArray();
        for (int index = 0; index < (int)friendsList.Size(); ++index) {
            auto const& friendId = friendsList[index].GetString();
            friends.push_back(friendId);
        }
    }

    if (userModInterface.HasMember("socials") && !userModInterface["socials"].IsNull()) {
        auto socialsList = userModInterface["socials"].GetArray();
        for (int index = 0; index < (int)socialsList.Size(); ++index) {
            auto const& social = socialsList[index];
            socials.push_back(Social(social));
        }
    }

    if (userModInterface.HasMember("profileSettings")) {
        profileSettings.emplace(ProfileSettings(userModInterface["profileSettings"].GetObject()));
    }
}

void Player::SetHistory(rapidjson::Value const& history) {
    getLogger().info("CreateHistory");
    lastRank = history["rank"].GetInt();
    lastCountryRank = history["countryRank"].GetInt();
    lastPP = history["pp"].GetFloat();
}

Social::Social(rapidjson::Value const& document) {
    service = document["service"].GetString();
    link = document["link"].IsNull() ? "" : document["link"].GetString();
    user = document["user"].IsNull() ? "" : document["user"].GetString();
}

ProfileSettings::ProfileSettings(rapidjson::Value const& document) {
    message = document["message"].IsNull() ? "" : document["message"].GetString();
    effectName = document["effectName"].IsNull() ? "" : document["effectName"].GetString();
    hue = document["hue"].GetInt();
    saturation = document["saturation"].GetFloat();
}