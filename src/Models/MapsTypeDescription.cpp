#include "shared/Models/MapsTypeDescription.hpp"

MapsTypeDescription::MapsTypeDescription(rapidjson::Value const& document) {
    if (document.HasMember("Id")) {
        id = document["Id"].GetInt();
    } else if (document.HasMember("id")) {
        id = document["id"].GetInt();
    }

    if (document.HasMember("Icon")) {
        icon = document["Icon"].GetString();
    } else if (document.HasMember("icon")) {
        icon = document["icon"].GetString();
    }

    if (document.HasMember("Name")) {
        name = document["Name"].GetString();
    } else if (document.HasMember("name")) {
        name = document["name"].GetString();
    }

    if (document.HasMember("Description")) {
        description = document["Description"].GetString();
    } else if (document.HasMember("description")) {
        description = document["description"].GetString();
    }

    if (document.HasMember("Color")) {
        color = document["Color"].GetString();
    } else if (document.HasMember("color")) {
        color = document["color"].GetString();
    }
}
