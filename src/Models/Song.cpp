#include "include/Models/Song.hpp"

Song::Song(rapidjson::Value const& document){
    auto difficultiesList = document["difficulties"].GetArray();

    for(const auto& difficulty : difficultiesList){
        difficulties.insert({string(difficulty["difficultyName"].GetString()) + difficulty["modeName"].GetString(), Difficulty(difficulty)});
    }
}