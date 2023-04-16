#pragma once

#include "include/Models/Difficulty.hpp"

namespace LevelInfoUI {
    void setup();
    void reset();
    void SetLevelInfoActive(bool active);

    void resetStars();
    void addVoteToCurrentLevel(bool rankable, int type);

    void setLabels(Difficulty selectedDifficulty);
    void refreshLabelsDiff();
}