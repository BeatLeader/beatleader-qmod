#pragma once
#include <string>
#include "beatsaber-hook/shared/rapidjson.hpp"

class MapDetail {
public:
    std::string id;
    std::string hash;
    std::string coverImage;
    std::string author;
    std::string mapper;
    std::string name;
    std::string downloadUrl;

    MapDetail(rapidjson::Value const& document);
}; 