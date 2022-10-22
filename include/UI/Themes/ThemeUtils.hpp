#pragma once

#include "UnityEngine/Material.hpp"

#include "include/Assets/BundleLoader.hpp"
#include "include/Models/Player.hpp"

using namespace std;
using namespace UnityEngine;

struct AvatarParams {
    Material* baseMaterial;
    float hueShift;
    float saturation;
};

inline AvatarParams GetAvatarParams(Player player, bool useSmallMaterialVersion) {
    if (player.profileSettings == nullopt) return { BundleLoader::bundle->defaultAvatarMaterial, 0.0f, 1.0f };

    float hueShift = (player.profileSettings->hue / 360.0f) * (3.1415 * 2);
    float saturation = player.profileSettings->saturation;

    return { BundleLoader::bundle->GetAvatarMaterial(player.profileSettings->effectName), hueShift, saturation };
}
