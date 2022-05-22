#pragma once

#include "beatsaber-hook/shared/config/rapidjson-utils.hpp"

#include <string>
using namespace std;

struct Clan
{
    string name;
    string tag;
    string icon;
    string color;

    Clan(rapidjson::Value const& document);
    Clan() = default;
};
