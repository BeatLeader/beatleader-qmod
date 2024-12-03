#include "shared/Models/MapData.hpp"

MapData::MapData(rapidjson::Value const& document) : 
    song(document["song"]) {
} 