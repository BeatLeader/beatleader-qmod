#pragma once
#include "MapData.hpp"
#include "beatsaber-hook/shared/config/rapidjson-utils.hpp"

class TrendingMapData : public MapData {
public:
    std::string trendingValue;
    std::string description;
    TrendingMapData(rapidjson::Value const& document);
}; 