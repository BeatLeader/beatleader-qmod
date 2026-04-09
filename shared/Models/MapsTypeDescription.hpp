#pragma once

#include "UnityEngine/Sprite.hpp"
#include "beatsaber-hook/shared/config/rapidjson-utils.hpp"

#include <string>

struct MapsTypeDescription {
    int id = 0;
    std::string icon;
    std::string name;
    std::string description;
    std::string color;
    UnityEngine::Sprite* sprite = nullptr;

    MapsTypeDescription() = default;
    MapsTypeDescription(rapidjson::Value const& document);
};
