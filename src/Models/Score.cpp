#include "include/Models/Score.hpp"

Score::Score() {}

Score::Score(rapidjson::GenericObject<false, rapidjson::GenericValue<rapidjson::UTF8<>>>& document) {
    auto playerObject = document["player"].GetObject();
    player = Player(playerObject);
}