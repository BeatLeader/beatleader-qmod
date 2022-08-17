#pragma once

#include "beatsaber-hook/shared/config/rapidjson-utils.hpp"

#include <string>
using namespace std;

struct Difficulty 
{
    float stars;
    bool nominated;
    bool qualified;
    bool ranked;
    int type;

    Difficulty(rapidjson::Value const& document);
    Difficulty() = default;
};