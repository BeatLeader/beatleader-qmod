#include "include/Models/Score.hpp"

Score::Score() {}

Score::Score(rapidjson::Value const& document) {
    auto const& playerObject = document["player"];
    player = Player(playerObject);

    pp = document["pp"].GetFloat();
    rank = document["rank"].GetInt();
    accuracy = document["accuracy"].GetFloat();
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