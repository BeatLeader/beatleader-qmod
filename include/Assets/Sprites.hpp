#pragma once

#include <string>
#include <map>

#include "UnityEngine/Sprite.hpp"
#include "AllFrameResult.hpp"

using namespace std;
/// <summary>
/// Helper code taken from BeatSaverDownloader
/// Copyright (c) 2018 andruzzzhka (MIT Licensed)
/// </summary>
class Sprites {

public:
    static UnityEngine::Sprite* get_StarIcon();
    static UnityEngine::Sprite* get_GraphIcon();
    static UnityEngine::Sprite* get_DownIcon();
    static UnityEngine::Sprite* get_UpIcon();
    static UnityEngine::Sprite* get_CellBG();
    static UnityEngine::Sprite* get_ModifiersIcon();
    static UnityEngine::Sprite* get_TransparentPixel();

    static void get_Icon(const string& url, const std::function<void(UnityEngine::Sprite*)>& completion, bool nullable = false);
    static void get_AnimatedIcon(const string& url, const std::function<void(AllFramesResult)>& completion);
    static void GetCountryIcon(string country, const std::function<void(UnityEngine::Sprite*)>& completion);
    static void ResetCache();
};