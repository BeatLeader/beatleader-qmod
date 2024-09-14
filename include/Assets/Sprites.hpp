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
    static const string StarB64;
    static const string GraphB64;
    static const string CellBGB64;
    static const string ModifiersIconB64;
    static const string PixelB64;
    static const string ClipboardB64;
    static const string ArrowsB64;

    static map<string, std::vector<uint8_t>> iconCache;
public:
    static UnityEngine::Sprite* get_StarIcon();
    static UnityEngine::Sprite* get_GraphIcon();
    static UnityEngine::Sprite* get_CellBG();
    static UnityEngine::Sprite* get_ModifiersIcon();
    static UnityEngine::Sprite* get_TransparentPixel();
    static UnityEngine::Sprite* get_ClipboardIcon();
    static UnityEngine::Sprite* get_ArrowIcon();

    static bool has_Icon(string url);
    static void get_Icon(string url, const std::function<void(UnityEngine::Sprite*)>& completion, bool nullable = false);
    static void get_AnimatedIcon(string url, const std::function<void(AllFramesResult)>& completion);
    static void ResetCache();
};