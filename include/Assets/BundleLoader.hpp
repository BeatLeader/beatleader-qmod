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

#define GET_FIND_METHOD(mPtr) il2cpp_utils::il2cpp_type_check::MetadataGetter<mPtr>::get()

DECLARE_CLASS_CODEGEN(BeatLeader, Bundle, UnityEngine::MonoBehaviour,
    DECLARE_INSTANCE_FIELD(Material*, logoMaterial);
    DECLARE_INSTANCE_FIELD(Material*, playerAvatarMaterial);
    DECLARE_INSTANCE_FIELD(Material*, UIAdditiveGlowMaterial);
    DECLARE_INSTANCE_FIELD(Material*, scoreBackgroundMaterial);
    DECLARE_INSTANCE_FIELD(Material*, scoreUnderlineMaterial);
    DECLARE_INSTANCE_FIELD(Material*, VotingButtonMaterial);
    DECLARE_INSTANCE_FIELD(Material*, handAccIndicatorMaterial);
    DECLARE_INSTANCE_FIELD(Material*, accGridBackgroundMaterial);
    DECLARE_INSTANCE_FIELD(Material*, accuracyGraphMaterial);
    DECLARE_INSTANCE_FIELD(Material*, accuracyGraphLine);
    DECLARE_INSTANCE_FIELD(Material*, accDetailsRowMaterial);

    DECLARE_INSTANCE_FIELD(Sprite*, locationIcon);
    DECLARE_INSTANCE_FIELD(Sprite*, rowSeparatorIcon);
    DECLARE_INSTANCE_FIELD(Sprite*, beatLeaderLogoGradient);
    DECLARE_INSTANCE_FIELD(Sprite*, transparentPixel);
    DECLARE_INSTANCE_FIELD(Sprite*, fileError);
    DECLARE_INSTANCE_FIELD(Sprite*, modifiersIcon);

    DECLARE_INSTANCE_FIELD(Sprite*, overviewIcon);
    DECLARE_INSTANCE_FIELD(Sprite*, detailsIcon);
    DECLARE_INSTANCE_FIELD(Sprite*, gridIcon);
    DECLARE_INSTANCE_FIELD(Sprite*, graphIcon);
    
    DECLARE_INSTANCE_FIELD(Sprite*, websiteLinkIcon);
    DECLARE_INSTANCE_FIELD(Sprite*, discordLinkIcon);
    DECLARE_INSTANCE_FIELD(Sprite*, patreonLinkIcon);

    DECLARE_INSTANCE_FIELD(Shader*, TMP_SpriteCurved);

    DECLARE_INSTANCE_METHOD(void, Init, AssetBundle* bundle);
)

class BundleLoader {
public:
    static AssetBundle* assetBundle;
    static BeatLeader::Bundle* bundle;

    static custom_types::Helpers::Coroutine LoadBundle(UnityEngine::GameObject* container);
};