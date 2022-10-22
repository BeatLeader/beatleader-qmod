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

    if (document.HasMember("friends") && !document["friends"].IsNull()) {
        auto friendsList = document["friends"].GetArray();
        for (int index = 0; index < (int)friendsList.Size(); ++index) {
            auto const& friendId = friendsList[index].GetString();
            friends.push_back(friendId);
        }
    }

    if (document.HasMember("socials") && !document["socials"].IsNull()) {
        auto socialsList = document["socials"].GetArray();
        for (int index = 0; index < (int)socialsList.Size(); ++index) {
            auto const& social = socialsList[index];
            socials.push_back(Social(social));
        }
    }

    if (document.HasMember("profileSettings")) {
        profileSettings.emplace(ProfileSettings(document["profileSettings"].GetObject()));
    }
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