#pragma once

#include "UnityEngine/AssetBundle.hpp"
#include "UnityEngine/Material.hpp"
#include "UnityEngine/MonoBehaviour.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Sprite.hpp"
#include "UnityEngine/Shader.hpp"
#include "UnityEngine/AssetBundleCreateRequest.hpp"

#include "custom-types/shared/macros.hpp"
#include "custom-types/shared/coroutine.hpp"

using namespace UnityEngine;
using namespace std;

#include <string>

#define DECLARE_FILE(name, prefix) extern "C" uint8_t _binary_##name##_start[]; extern "C" uint8_t _binary_##name##_end[]; struct prefix##name { static size_t getLength() { return _binary_##name##_end - _binary_##name##_start; } static uint8_t* getData() { return _binary_##name##_start; } };
DECLARE_FILE(bl_bundle,)

DECLARE_CLASS_CODEGEN(BeatLeader, Bundle, MonoBehaviour,
    DECLARE_INSTANCE_FIELD(Material*, logoMaterial);
    DECLARE_INSTANCE_FIELD(Material*, defaultAvatarMaterial);
    DECLARE_INSTANCE_FIELD(Material*, UIAdditiveGlowMaterial);
    DECLARE_INSTANCE_FIELD(Material*, scoreBackgroundMaterial);
    DECLARE_INSTANCE_FIELD(Material*, scoreUnderlineMaterial);
    DECLARE_INSTANCE_FIELD(Material*, VotingButtonMaterial);
    DECLARE_INSTANCE_FIELD(Material*, handAccIndicatorMaterial);
    DECLARE_INSTANCE_FIELD(Material*, accGridBackgroundMaterial);
    DECLARE_INSTANCE_FIELD(Material*, accuracyGraphMaterial);
    DECLARE_INSTANCE_FIELD(Material*, accuracyGraphLine);
    DECLARE_INSTANCE_FIELD(Material*, accDetailsRowMaterial);
    DECLARE_INSTANCE_FIELD(Material*, miniProfileBackgroundMaterial);
    DECLARE_INSTANCE_FIELD(Material*, skillTriangleMaterial);

    DECLARE_INSTANCE_FIELD(Sprite*, locationIcon);
    DECLARE_INSTANCE_FIELD(Sprite*, rowSeparatorIcon);
    DECLARE_INSTANCE_FIELD(Sprite*, beatLeaderLogoGradient);
    DECLARE_INSTANCE_FIELD(Sprite*, transparentPixel);
    DECLARE_INSTANCE_FIELD(Sprite*, fileError);
    DECLARE_INSTANCE_FIELD(Sprite*, modifiersIcon);

    DECLARE_INSTANCE_FIELD(Sprite*, overview1Icon);
    DECLARE_INSTANCE_FIELD(Sprite*, overview2Icon);
    DECLARE_INSTANCE_FIELD(Sprite*, detailsIcon);
    DECLARE_INSTANCE_FIELD(Sprite*, gridIcon);
    DECLARE_INSTANCE_FIELD(Sprite*, graphIcon);
    DECLARE_INSTANCE_FIELD(Sprite*, replayIcon);
    
    DECLARE_INSTANCE_FIELD(Sprite*, websiteLinkIcon);
    DECLARE_INSTANCE_FIELD(Sprite*, discordLinkIcon);
    DECLARE_INSTANCE_FIELD(Sprite*, patreonLinkIcon);

    DECLARE_INSTANCE_FIELD(Sprite*, twitterIcon);
    DECLARE_INSTANCE_FIELD(Sprite*, twitchIcon);
    DECLARE_INSTANCE_FIELD(Sprite*, youtubeIcon);
    DECLARE_INSTANCE_FIELD(Sprite*, profileIcon);
    DECLARE_INSTANCE_FIELD(Sprite*, friendsIcon);
    DECLARE_INSTANCE_FIELD(Sprite*, friendsSelectorIcon);
    DECLARE_INSTANCE_FIELD(Sprite*, globeIcon);
    DECLARE_INSTANCE_FIELD(Sprite*, incognitoIcon);
    DECLARE_INSTANCE_FIELD(Sprite*, defaultAvatar);
    DECLARE_INSTANCE_FIELD(Sprite*, settingsIcon);

    DECLARE_INSTANCE_FIELD(Sprite*, generalContextIcon);
    DECLARE_INSTANCE_FIELD(Sprite*, noModifiersIcon);
    DECLARE_INSTANCE_FIELD(Sprite*, noPauseIcon);
    DECLARE_INSTANCE_FIELD(Sprite*, golfIcon);

    DECLARE_INSTANCE_FIELD(Shader*, TMP_SpriteCurved);

    DECLARE_INSTANCE_METHOD(void, Init, AssetBundle* bundle);
    DECLARE_INSTANCE_METHOD(Material*, GetAvatarMaterial, StringW effectName);
    DECLARE_INSTANCE_METHOD(Sprite*, GetCountryIcon, StringW country);
)

class BundleLoader {
public:
    static AssetBundle* assetBundle;
    static BeatLeader::Bundle* bundle;

    static custom_types::Helpers::Coroutine LoadBundle(GameObject* container);
};