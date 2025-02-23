#include "shared/Models/Score.hpp"

Score::Score() {}

Score::Score(rapidjson::Value const& document) {
    auto const& playerObject = document["player"];
    player = Player(playerObject);

    id = document["id"].GetInt();
    pp = document["pp"].GetFloat();
    rank = document["rank"].GetInt();
    accuracy = document["accuracy"].GetFloat();
    replay = document["replay"].GetString();
    playerId = document["playerId"].GetString();
    fullCombo = document["fullCombo"].GetBool();
    modifiers = document["modifiers"].GetString();
    modifiedScore = document["modifiedScore"].GetInt();

    badCuts = document["badCuts"].GetInt();
    missedNotes = document["missedNotes"].GetInt();
    bombCuts = document["bombCuts"].GetInt();
    wallsHit = document["wallsHit"].GetInt();
    pauses = document["pauses"].GetInt();

    if (document.HasMember("headsetName") && !document["headsetName"].IsNull()) {
        headsetName = document["headsetName"].GetString();
    } else {
        headsetName = "Unknown";
    }

    if (document.HasMember("leaderboardId") && !document["leaderboardId"].IsNull()) {
        leaderboardId = document["leaderboardId"].GetString();
    } else {
        leaderboardId = "";
    }

    timeset = document["timeset"].GetString();
    platform = document["platform"].GetString();

    if (!document["scoreImprovement"].IsNull()) {
        scoreImprovement = ScoreImprovement(document["scoreImprovement"]);
    }
}

ScoreImprovement::ScoreImprovement() {}

ScoreImprovement::ScoreImprovement(rapidjson::Value const& document) {
    score = document["score"].GetInt();
    accuracy = document["accuracy"].GetFloat();
    rank = document["rank"].GetInt();
    pp = document["pp"].GetFloat();

    totalRank = document["totalRank"].GetInt();
    totalPp = document["totalPp"].GetFloat();
}