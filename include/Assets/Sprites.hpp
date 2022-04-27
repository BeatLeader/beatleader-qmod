#pragma once

#include <string>
#include <map>

#include "UnityEngine/Sprite.hpp"

using namespace std;
/// <summary>
/// Helper code taken from BeatSaverDownloader
/// Copyright (c) 2018 andruzzzhka (MIT Licensed)
/// </summary>
class Sprites {
    static const string BeatLeaderB64;
    static const string StarB64;
    static const string GraphB64;
    static const string DownB64;
    static const string UpB64;
    static const string CellBGB64;
    static const string ModifiersIconB64;

    static map<string, std::vector<uint8_t>> iconCache;
public:
    static UnityEngine::Sprite* get_BeatLeaderIcon();
    static UnityEngine::Sprite* get_StarIcon();
    static UnityEngine::Sprite* get_GraphIcon();
    static UnityEngine::Sprite* get_DownIcon();
    static UnityEngine::Sprite* get_UpIcon();
    static UnityEngine::Sprite* get_CellBG();
    static UnityEngine::Sprite* get_ModifiersIcon();

    static void get_Icon(string url, std::function<void(UnityEngine::Sprite*)> completion);
    static void GetCountryIcon(string country, std::function<void(UnityEngine::Sprite*)> completion);
    static void ResetCache();
};