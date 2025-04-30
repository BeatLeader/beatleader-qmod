#include "shared/Models/TrendingMapData.hpp"

TrendingMapData::TrendingMapData(rapidjson::Value const& document) : MapData(document),
    trendingValue(document["trendingValue"].GetString()),
    description(document["description"].GetString()) {
}

