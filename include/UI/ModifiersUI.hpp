#pragma once

#include "include/Models/TriangleRating.hpp"

#include <unordered_map>
#include <string>

using namespace std;

namespace ModifiersUI {
    extern unordered_map<string, float> songModifiers;
    extern unordered_map<string, TriangleRating> songModifierRatings;

    void setup();
    TriangleRating refreshMultiplierAndMaxRank();
    TriangleRating refreshAllModifiers();
    void SetModifiersActive(bool active);
    void ResetModifiersUI();
    bool ModifiersAvailable();
}