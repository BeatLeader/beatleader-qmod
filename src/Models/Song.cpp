#include "shared/Models/Song.hpp"

Song::Song(rapidjson::Value const& document){
    // Sometimes when selecting songs really fast we get something that is not an array (maybe too many requests but with status 200?)
    // If we would proceed, that would lead to a npe, therefor we just do nothing in this case
    if(!document.IsArray()){
        return;
    }

    auto difficultiesList = document.GetArray();

    for(const auto& difficulty : difficultiesList) {
        difficulties.insert({string(difficulty["difficultyName"].GetString()) + difficulty["modeName"].GetString(), Difficulty(difficulty)});
    }
}