#pragma once

#include "UnityEngine/Sprite.hpp"
#include "UnityEngine/Vector2.hpp"

#include <optional>
#include <string>

namespace DanceEventAssets {
    UnityEngine::Sprite* GetSprite(std::string const& file, UnityEngine::Vector2 pivot = {0.5f, 0.5f});
    std::optional<std::string> Gunzip(std::string const& file);

    UnityEngine::Sprite* Flame();
    UnityEngine::Sprite* Glow();
    UnityEngine::Sprite* Mist();
    UnityEngine::Sprite* Fog();
    UnityEngine::Sprite* Thread();

    float PerlinNoise(float x, float y);
}
