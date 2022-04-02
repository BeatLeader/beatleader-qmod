#include "include/Models/Score.hpp"

Score::Score(rapidjson::GenericObject<false, rapidjson::GenericValue<rapidjson::UTF8<>>>& document) {
    auto playerObject = document["player"].GetObject();
    player = new Player(playerObject);
}