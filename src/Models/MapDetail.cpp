#include "shared/Models/MapDetail.hpp"

MapDetail::MapDetail(rapidjson::Value const& document) {
    id = document["id"].GetString();
    hash = document["hash"].GetString();
    coverImage = document["coverImage"].GetString();
    author = document["author"].GetString();
    mapper = document["mapper"].GetString();
    name = document["name"].GetString();
    downloadUrl = document["downloadUrl"].GetString();
} 