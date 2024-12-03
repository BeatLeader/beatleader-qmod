#pragma once

#include "UnityEngine/Sprite.hpp"
#include "UnityEngine/TextureWrapMode.hpp"
#include "UnityEngine/Resources.hpp"
#include "System/Collections/Generic/Dictionary_2.hpp"
#include <string>
#include <stdexcept>
#include <unordered_map>

namespace BeatLeader::Utils {
    class BSMLUtility {
    public:
        static void AddSpriteToBSMLCache(std::string_view name, UnityEngine::Sprite* sprite);
        static UnityEngine::Sprite* LoadSprite(std::string_view location);
        static UnityEngine::Sprite* FindSpriteCached(std::string_view name);
        
    private:
        static inline std::unordered_map<std::string, UnityEngine::Sprite*> spriteCache;
    };
} 