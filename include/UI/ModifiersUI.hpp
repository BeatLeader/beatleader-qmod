#pragma once

#include "shared/Models/TriangleRating.hpp"

#include <unordered_map>
#include <string>

using namespace std;

namespace ModifiersUI {
    extern unordered_map<string, float> songModifiers;
    extern unordered_map<string, TriangleRating> songModifierRatings;

    void setup();
    void SetSongModifiers(unordered_map<string, float> const& modifiers, unordered_map<string, TriangleRating> const& ratings);
    void ClearSongModifiers();
    TriangleRating refreshMultiplierAndMaxRank();
    TriangleRating refreshAllModifiers();
    TriangleRating ApplyCurrentModifiers(TriangleRating rating);
    void SetModifiersActive(bool active);
    void ResetModifiersUI();
    bool ModifiersAvailable();
}
