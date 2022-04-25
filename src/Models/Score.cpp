#include "include/Models/Score.hpp"

Score::Score() {}

Score::Score(rapidjson::GenericObject<false, rapidjson::GenericValue<rapidjson::UTF8<>>>& document) {
    auto playerObject = document["player"].GetObject();
    player = Player(playerObject);

    pp = document["pp"].GetDouble();
    rank = document["rank"].GetInt();
    accuracy = document["accuracy"].GetDouble();
    playerId = document["playerId"].GetString();
    fullCombo = document["fullCombo"].GetBool();
    modifiers = document["modifiers"].GetString();
    modifiedScore = document["modifiedScore"].GetInt();

    badCuts = document["badCuts"].GetInt();
    missedNotes = document["missedNotes"].GetInt();
    bombCuts = document["bombCuts"].GetInt();
    wallsHit = document["wallsHit"].GetInt();
    pauses = document["pauses"].GetInt();

    hmd = document["hmd"].GetInt();
    timeset = document["timeset"].GetString();
}