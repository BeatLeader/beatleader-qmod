#pragma once
#include <string>
#include "beatsaber-hook/shared/config/rapidjson-utils.hpp"

class NewsPost {
public:
    int64_t timepost;
    std::string owner;
    std::string ownerIcon;
    std::string body;
    std::string image;

    NewsPost(rapidjson::Value const& document);
    NewsPost() = default;
}; 