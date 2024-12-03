#pragma once
#include "MapDetail.hpp"
#include "beatsaber-hook/shared/config/rapidjson-utils.hpp"

class MapData {
public:
    MapDetail song;

    MapData(rapidjson::Value const& document);
}; 