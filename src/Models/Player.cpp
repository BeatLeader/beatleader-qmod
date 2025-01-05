#include "shared/Models/Player.hpp"

#include "include/Utils/ModConfig.hpp"
#include "include/main.hpp"

Player::Player(rapidjson::Value const& userModInterface) {

    id = userModInterface["id"].GetString();
    name = userModInterface["name"].GetString();
    country = userModInterface["country"].GetString();
    avatar = userModInterface["avatar"].GetString();
    role = userModInterface["role"].GetString();

    if (userModInterface.HasMember("questId") && !userModInterface["questId"].IsNull()) {
        questId = userModInterface["questId"].GetString();
    } else {
        questId = nullopt;
    }

    // For standard context and players from v3/scores (where contextExtension is null cause we request for one context) we use the main player
    int currentContext = getModConfig().Context.GetValue();
    std::optional<rapidjson::GenericArray<true, rapidjson::Value>> contextExtensions = 
        userModInterface.HasMember("contextExtensions") && !userModInterface["contextExtensions"].IsNull() 
        ? userModInterface["contextExtensions"].GetArray() 
        : std::optional<rapidjson::GenericArray<true, rapidjson::Value>>();
    // If we are Standard Context or we have no contexts or our selected context is not in contextextensions we use the normal rank. Else we use the correct context extension rank
    rapidjson::Value const& contextRank = 
        currentContext == 0 || 
        !contextExtensions ? userModInterface : [&]() -> const rapidjson::Value& {
            for (auto& ext : contextExtensions.value()) {
                if (ext["context"].GetInt() == currentContext) {
                    return ext;
                }
            }
            return userModInterface;
        }();

    rank = contextRank["rank"].GetInt();
        countryRank = contextRank["countryRank"].GetInt();
    pp = contextRank["pp"].GetFloat();

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

    if (userModInterface.HasMember("playlistsToInstall") && !userModInterface["playlistsToInstall"].IsNull()) {
        playlistsToInstall = userModInterface["playlistsToInstall"].GetString();
    } else {
        playlistsToInstall = "";
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

void Player::SetFromScore(rapidjson::Value const& score) {

    auto const& userModInterface = score["player"];
    int currentContext = getModConfig().Context.GetValue();
    std::optional<rapidjson::GenericArray<true, rapidjson::Value>> contextExtensions = 
        userModInterface.HasMember("contextExtensions") && !userModInterface["contextExtensions"].IsNull() 
        ? userModInterface["contextExtensions"].GetArray() 
        : std::optional<rapidjson::GenericArray<true, rapidjson::Value>>();
    // If we are Standard Context or we have no contexts or our selected context is not in contextextensions we use the normal rank. Else we use the correct context extension rank
    rapidjson::Value const& contextRank = 
        currentContext == 0 || 
        !contextExtensions ? userModInterface : [&]() -> const rapidjson::Value& {
            for (auto& ext : contextExtensions.value()) {
                if (ext["context"].GetInt() == currentContext) {
                    return ext;
                }
            }
            return userModInterface;
        }();

    rank = contextRank["rank"].GetInt();
    countryRank = contextRank["countryRank"].GetInt();
    pp = contextRank["pp"].GetFloat();
}