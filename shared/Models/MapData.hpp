#pragma once
#include "MapDetail.hpp"
#include "beatsaber-hook/shared/rapidjson.hpp"

class MapData {
public:
    MapDetail song;

    MapData(rapidjson::Value const& document);
}; 