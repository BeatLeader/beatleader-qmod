#pragma once

#include "shared/Models/Difficulty.hpp"
#include "shared/Models/TriangleRating.hpp"

namespace LevelInfoUI {
    void setup();
    void reset();
    void SetLevelInfoActive(bool active);

    void resetStars();
    void addVoteToCurrentLevel(bool rankable, int type);

    void setLabels(Difficulty selectedDifficulty);
    void refreshRatingLabels();
    void setRatingLabels(TriangleRating rating);
}