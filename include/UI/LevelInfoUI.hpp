#pragma once

#include "include/Models/Difficulty.hpp"

namespace LevelInfoUI {
    void setup();
    void reset();

    void resetStars();
    void addVoteToCurrentLevel(bool rankable, int type);

    void setLabels(Difficulty selectedDifficulty);
}