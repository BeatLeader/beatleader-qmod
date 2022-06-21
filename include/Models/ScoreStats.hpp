#pragma once

#include "beatsaber-hook/shared/config/rapidjson-utils.hpp"
#include "include/Models/Player.hpp"

#include <string>
using namespace std;

struct AccuracyTracker {
    float gridAcc[12];
    float accLeft;
    float accRight;

    float leftAverageCut[3];
    float leftPreswing;
    float leftPostswing;
    float leftTimeDependence;

    float rightAverageCut[3];
    float rightPreswing;
    float rightPostswing;
    float rightTimeDependence;

    AccuracyTracker();
    AccuracyTracker(rapidjson::Value const& document);
};

struct HitTracker {
    int maxCombo;

    int leftBadCuts;
    int leftBombs;
    int leftMiss;

    int rightBadCuts;
    int rightBombs;
    int rightMiss;

    HitTracker();
    HitTracker(rapidjson::Value const& document);
};

struct WinTracker {
    float jumpDistance;
    int nbOfPause;
    int totalScore;
    float endTime;
    bool won;

    WinTracker();
    WinTracker(rapidjson::Value const& document);
};

struct ScoreGraphTracker {
    vector<float> graph;

    ScoreGraphTracker();
    ScoreGraphTracker(rapidjson::Value const& document);
};

struct ScoreStats {
    AccuracyTracker accuracyTracker;
    HitTracker hitTracker;
    WinTracker winTracker;
    ScoreGraphTracker scoreGraphTracker;

    ScoreStats();
    ScoreStats(rapidjson::Value const& document);
};
