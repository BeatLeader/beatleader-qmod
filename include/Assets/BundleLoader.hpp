#include "UnityEngine/AssetBundle.hpp"
#include "UnityEngine/Material.hpp"
#include "UnityEngine/Sprite.hpp"
#include "UnityEngine/Shader.hpp"
#include "UnityEngine/AssetBundleCreateRequest.hpp"

#include "custom-types/shared/coroutine.hpp"

using namespace UnityEngine;

#define DECLARE_FILE(name, prefix) extern "C" uint8_t _binary_##name##_start[]; extern "C" uint8_t _binary_##name##_end[]; struct prefix##name { static size_t getLength() { return _binary_##name##_end - _binary_##name##_start; } static uint8_t* getData() { return _binary_##name##_start; } };
DECLARE_FILE(bl_bundle,)

class BundleLoader {
public:
    static AssetBundle* bundle;
    static Material* logoMaterial;
    static Material* playerAvatarMaterial;
    static Material* UIAdditiveGlowMaterial;
    static Material* scoreBackgroundMaterial;
    static Material* scoreUnderlineMaterial;
    static Material* VotingButtonMaterial;
    static Material* handAccIndicatorMaterial;
    static Material* accGridBackgroundMaterial;
    static Material* accuracyGraphMaterial;
    static Material* accuracyGraphLine;
    static Material* accDetailsRowMaterial;

    static Sprite* locationIcon;
    static Sprite* rowSeparatorIcon;
    static Sprite* beatLeaderLogoGradient;
    static Sprite* transparentPixel;
    static Sprite* fileError;
    static Sprite* modifiersIcon;

    static Sprite* overviewIcon;
    static Sprite* detailsIcon;
    static Sprite* gridIcon;
    static Sprite* graphIcon;
    static Sprite* websiteLinkIcon;
    static Sprite* discordLinkIcon;
    static Sprite* patreonLinkIcon;

    static Shader* TMP_SpriteCurved;

    static custom_types::Helpers::Coroutine LoadBundle();
    static void LoadAssets(AssetBundle* assetBundle);
};