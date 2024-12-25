#pragma once

struct ChristmasSettings {
    bool treeEnabled;
    bool snowEnabled;

    ChristmasSettings() : treeEnabled(false), snowEnabled(false) {}
    ChristmasSettings(bool tree, bool snow) : treeEnabled(tree), snowEnabled(snow) {}
}; 