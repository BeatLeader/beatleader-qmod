#include "include/Models/Difficulty.hpp"

Difficulty::Difficulty(rapidjson::Value const& document) {
    stars = document["stars"].GetFloat();
    nominated = document["nominated"].GetBool();
    qualified = document["qualified"].GetBool();
    ranked = document["ranked"].GetBool();
    type = document["type"].GetInt();
}