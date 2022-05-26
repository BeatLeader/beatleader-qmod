#pragma once

#include "UnityEngine/MonoBehaviour.hpp"
#include "UnityEngine/Material.hpp"
#include "UnityEngine/Texture2D.hpp"
#include "UnityEngine/CanvasRenderer.hpp"
#include "HMUI/ImageView.hpp"

#include "custom-types/shared/macros.hpp"

#include <string>
using namespace std;

#define GET_FIND_METHOD(mPtr) il2cpp_utils::il2cpp_type_check::MetadataGetter<mPtr>::get()

DECLARE_CLASS_CODEGEN(BeatLeader, PlayerAvatar, UnityEngine::MonoBehaviour,
    DECLARE_INSTANCE_FIELD(HMUI::ImageView*, imageView);
    DECLARE_INSTANCE_FIELD(UnityEngine::Material*, materialInstance);
    DECLARE_INSTANCE_FIELD(int, AvatarTexturePropertyId);
    DECLARE_INSTANCE_FIELD(int, FadeValuePropertyId);

    DECLARE_INSTANCE_FIELD(int, currentFrame);
    DECLARE_INSTANCE_FIELD(float, frameTime);
    DECLARE_INSTANCE_FIELD(bool, play);
    
    DECLARE_INSTANCE_FIELD(Array<UnityEngine::Texture2D*>*, animationFrames);
    DECLARE_INSTANCE_FIELD(Array<float>*, animationTimings);
     
    DECLARE_INSTANCE_METHOD(void, Init, HMUI::ImageView* imageView);
    DECLARE_INSTANCE_METHOD(void, SetPlayer, Il2CppString* url, Il2CppString* roles);
    DECLARE_INSTANCE_METHOD(void, Update);
)