#pragma once
#include <string>
#include "beatsaber-hook/shared/config/rapidjson-utils.hpp"

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