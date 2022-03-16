#pragma once

#include <string>
#include "UnityEngine/Sprite.hpp"

using namespace std;
/// <summary>
/// Helper code taken from BeatSaverDownloader
/// Copyright (c) 2018 andruzzzhka (MIT Licensed)
/// </summary>
class Sprites {
    static const string BeatLeaderB64;
    static UnityEngine::Sprite* BeatLeaderIcon;

    static const string StarB64;
    static UnityEngine::Sprite* StarIcon;

    static const string GraphB64;
    static UnityEngine::Sprite* GraphIcon;
public:
    static UnityEngine::Sprite* get_BeatLeaderIcon();
    static UnityEngine::Sprite* get_StarIcon();
    static UnityEngine::Sprite* get_GraphIcon();
};