#pragma once

#include "include/Models/Difficulty.hpp"

namespace LevelInfoUI {
    void setup();
    void reset();

    void resetStars();
    void setLabels(Difficulty selectedDifficulty);
}