#pragma once

#include "UnityEngine/AssetBundle.hpp"
#include "UnityEngine/Material.hpp"
#include "UnityEngine/MonoBehaviour.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Sprite.hpp"
#include "UnityEngine/Shader.hpp"
#include "UnityEngine/AssetBundleCreateRequest.hpp"
#include "HMUI/IconSegmentedControl.hpp"

#include "custom-types/shared/macros.hpp"
#include "custom-types/shared/coroutine.hpp"

#include "System/Collections/Generic/Dictionary_2.hpp"

using namespace UnityEngine;
using namespace std;

#include <string>

#define DECLARE_FILE(name, prefix) extern "C" uint8_t _binary_##name##_start[]; extern "C" uint8_t _binary_##name##_end[]; struct prefix##name { static size_t getLength() { return _binary_##name##_end - _binary_##name##_start; } static uint8_t* getData() { return _binary_##name##_start; } };
DECLARE_FILE(bl_bundle,)

typedef System::Collections::Generic::Dictionary_2<StringW, Material *> MaterialsDictionary;
typedef System::Collections::Generic::Dictionary_2<StringW, Sprite *> SpritesDictionary;

DECLARE_CLASS_CODEGEN(BeatLeader, Bundle, MonoBehaviour) {

    DECLARE_INSTANCE_FIELD(MaterialsDictionary*, materials);
    DECLARE_INSTANCE_FIELD(SpritesDictionary*, sprites);
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
    DECLARE_INSTANCE_FIELD(Material*, experienceBarMaterial);
    DECLARE_INSTANCE_FIELD(Material*, clanTagBackgroundMaterial);
    DECLARE_INSTANCE_FIELD(Material*, roundTexture10Material);
    DECLARE_INSTANCE_FIELD(Material*, roundTexture2Material);

    DECLARE_INSTANCE_FIELD(Sprite*, locationIcon);
    DECLARE_INSTANCE_FIELD(Sprite*, rowSeparatorIcon);
    DECLARE_INSTANCE_FIELD(Sprite*, beatLeaderLogoGradient);
    DECLARE_INSTANCE_FIELD(Sprite*, transparentPixel);
    DECLARE_INSTANCE_FIELD(Sprite*, fileError);
    DECLARE_INSTANCE_FIELD(Sprite*, modifiersIcon);
    DECLARE_INSTANCE_FIELD(Sprite*, upIcon);
    DECLARE_INSTANCE_FIELD(Sprite*, downIcon);

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
    DECLARE_INSTANCE_FIELD(Sprite*, unknownIcon);
    DECLARE_INSTANCE_FIELD(Shader*, TMP_SpriteCurved);

    DECLARE_INSTANCE_FIELD(Sprite*, BlackTransparentBG);
    DECLARE_INSTANCE_FIELD(Sprite*, BlackTransparentBGOutline);
    DECLARE_INSTANCE_FIELD(Sprite*, CyanBGOutline);
    DECLARE_INSTANCE_FIELD(Sprite*, WhiteBG);
    DECLARE_INSTANCE_FIELD(Sprite*, ClosedDoorIcon);
    DECLARE_INSTANCE_FIELD(Sprite*, OpenedDoorIcon);
    DECLARE_INSTANCE_FIELD(Sprite*, EditLayoutIcon);
    DECLARE_INSTANCE_FIELD(Sprite*, ReplayerSettingsIcon);
    DECLARE_INSTANCE_FIELD(Sprite*, LeftArrowIcon);
    DECLARE_INSTANCE_FIELD(Sprite*, RightArrowIcon);
    DECLARE_INSTANCE_FIELD(Sprite*, PlayIcon);
    DECLARE_INSTANCE_FIELD(Sprite*, PauseIcon);
    DECLARE_INSTANCE_FIELD(Sprite*, LockIcon);
    DECLARE_INSTANCE_FIELD(Sprite*, WarningIcon);
    DECLARE_INSTANCE_FIELD(Sprite*, CrossIcon);
    DECLARE_INSTANCE_FIELD(Sprite*, PinIcon);
    DECLARE_INSTANCE_FIELD(Sprite*, AlignIcon);
    DECLARE_INSTANCE_FIELD(Sprite*, AnchorIcon);
    DECLARE_INSTANCE_FIELD(Sprite*, ProgressRingIcon);
    DECLARE_INSTANCE_FIELD(Sprite*, RotateRightIcon);

    DECLARE_INSTANCE_FIELD(::HMUI::IconSegmentedControl::DataItem*, IconSegmentedControl);

    DECLARE_INSTANCE_METHOD(void, Init, AssetBundle* bundle);
    DECLARE_INSTANCE_METHOD(Material*, GetAvatarMaterial, StringW effectName);
    DECLARE_INSTANCE_METHOD(Sprite*, GetCountryIcon, StringW country);

    DECLARE_INSTANCE_METHOD(Material*, GetMaterial, StringW name);
    DECLARE_INSTANCE_METHOD(Sprite*, GetSprite, StringW name);
};

class BundleLoader {
public:
    static AssetBundle* assetBundle;
    static BeatLeader::Bundle* bundle;

    static custom_types::Helpers::Coroutine LoadBundle(GameObject* container);
};