#include "Utils/BSMLUtility.hpp"

namespace BeatLeader::Utils {
    void BSMLUtility::AddSpriteToBSMLCache(std::string_view name, UnityEngine::Sprite* sprite) {
        if (!sprite) return;
        spriteCache[StringW(name)] = sprite;
    }

    UnityEngine::Sprite* BSMLUtility::LoadSprite(std::string_view location) {
        UnityEngine::Sprite* sprite = nullptr;
        
        if (location.length() > 1 && location[0] == '#') {
            std::string_view text = location.substr(1);
            sprite = FindSpriteCached(text);
        } else {
            // Note: Quest modding typically uses QuestUI for sprite loading
            // You might need to adjust this based on your specific needs
            // sprite = QuestUI::BeatSaberUI::FileToSprite(std::string(location));
            
            if (sprite && sprite->get_texture()) {
                sprite->get_texture()->set_wrapMode(UnityEngine::TextureWrapMode::Clamp);
            }
        }
        return sprite;
    }

    UnityEngine::Sprite* BSMLUtility::FindSpriteCached(std::string_view name) {
        StringW nameW(name);
        
        // Check cache first
        auto it = spriteCache.find(nameW);
        if (it != spriteCache.end() && it->second) {
            return it->second;
        }

        // Search all sprites if not in cache
        auto sprites = UnityEngine::Resources::FindObjectsOfTypeAll<UnityEngine::Sprite*>();
        UnityEngine::Sprite* result = nullptr;

        for (auto sprite : sprites) {
            if (!sprite || sprite->get_name()->get_Length() == 0) continue;

            // Cache the sprite if not already cached
            StringW spriteName = sprite->get_name();
            auto cacheIt = spriteCache.find(spriteName);
            if (cacheIt == spriteCache.end() || !cacheIt->second) {
                spriteCache[spriteName] = sprite;
            }

            // Check if this is the sprite we're looking for
            if (spriteName == nameW) {
                result = sprite;
            }
        }

        return result;
    }
} 