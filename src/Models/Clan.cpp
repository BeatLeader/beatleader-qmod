#include "include/Models/Clan.hpp"

Clan::Clan(rapidjson::Value const& document) {
    tag = document["tag"].GetString();
    color = document["color"].GetString();
}