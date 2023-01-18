#pragma once

#include <unordered_map>
#include <string>

using namespace std;

namespace ModifiersUI {
    extern unordered_map<string, float> songModifiers;

    void setup();
    void refreshMultiplierAndMaxRank();
    void refreshAllModifiers();
    void SetModifiersActive(bool active);
    void ResetModifiersUI();
}