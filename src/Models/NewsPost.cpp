#include "shared/Models/NewsPost.hpp"

NewsPost::NewsPost(rapidjson::Value const& document) {
    timepost = document["timepost"].GetInt64();
    owner = document["owner"].GetString();
    ownerIcon = document["ownerIcon"].GetString();
    body = document["body"].GetString();
    image = document["image"].GetString();
} 