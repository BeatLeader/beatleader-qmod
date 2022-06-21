#include "include/Models/ScoreStats.hpp"

AccuracyTracker::AccuracyTracker() {}

AccuracyTracker::AccuracyTracker(rapidjson::Value const& document) {
    auto gridAccArray = document["gridAcc"].GetArray();

    for (int index = 0; index < (int)gridAccArray.Size(); ++index) {
        gridAcc[index] = gridAccArray[index].GetFloat();
    }

    accLeft = document["accLeft"].GetFloat();
    accRight = document["accRight"].GetFloat();

    auto leftAverageCutArray = document["leftAverageCut"].GetArray();
    for (int index = 0; index < (int)leftAverageCutArray.Size(); ++index) {
        leftAverageCut[index] = leftAverageCutArray[index].GetFloat();
    }

    leftPreswing = document["leftPreswing"].GetFloat();
    leftPostswing = document["leftPostswing"].GetFloat();
    leftTimeDependence = document["leftTimeDependence"].GetFloat();

    auto rightAverageCutArray = document["rightAverageCut"].GetArray();
    for (int index = 0; index < (int)rightAverageCutArray.Size(); ++index) {
        rightAverageCut[index] = rightAverageCutArray[index].GetFloat();
    }

    rightPreswing = document["rightPreswing"].GetFloat();
    rightPostswing = document["rightPostswing"].GetFloat();
    rightTimeDependence = document["rightTimeDependence"].GetFloat();
}

HitTracker::HitTracker() {}

HitTracker::HitTracker(rapidjson::Value const& document) {
    maxCombo = document["maxCombo"].GetInt();

    leftBadCuts = document["leftBadCuts"].GetInt();
    leftBombs = document["leftBombs"].GetInt();
    leftMiss = document["leftMiss"].GetInt();

    rightBadCuts = document["rightBadCuts"].GetInt();
    rightBombs = document["rightBombs"].GetInt();
    rightMiss = document["rightMiss"].GetInt();
}

WinTracker::WinTracker() {}

WinTracker::WinTracker(rapidjson::Value const& document) {

    jumpDistance = document["jumpDistance"].GetFloat();
    nbOfPause = document["nbOfPause"].GetInt();
    totalScore = document["totalScore"].GetInt();
    endTime = document["endTime"].GetFloat();
    won = document["won"].GetBool();
}

ScoreGraphTracker::ScoreGraphTracker() {}

ScoreGraphTracker::ScoreGraphTracker(rapidjson::Value const& document) {
    auto graphArray  = document["graph"].GetArray();
    for (int index = 0; index < (int)graphArray.Size(); ++index) {
        graph.emplace_back(graphArray[index].GetFloat());
    }
}

ScoreStats::ScoreStats() {}

ScoreStats::ScoreStats(rapidjson::Value const& document) {
    auto const& accuracyTrackerObject = document["accuracyTracker"];
    accuracyTracker = AccuracyTracker(accuracyTrackerObject);
    auto const& hitTrackerObject = document["hitTracker"];
    hitTracker = HitTracker(hitTrackerObject);
    auto const& winTrackerObject = document["winTracker"];
    winTracker = WinTracker(winTrackerObject);
    auto const& scoreGraphTrackerObject = document["scoreGraphTracker"];
    scoreGraphTracker = ScoreGraphTracker(scoreGraphTrackerObject);
};