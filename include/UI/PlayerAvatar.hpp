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
    DECLARE_INSTANCE_FIELD(StringW, materialName);

    DECLARE_INSTANCE_FIELD(int, currentFrame);
    DECLARE_INSTANCE_FIELD(float, frameTime);
    DECLARE_INSTANCE_FIELD(bool, play);
    
    DECLARE_INSTANCE_FIELD(ArrayW<UnityEngine::Texture2D*>, animationFrames);
    DECLARE_INSTANCE_FIELD(ArrayW<float>, animationTimings);
     
    DECLARE_INSTANCE_METHOD(void, Init, HMUI::ImageView* imageView);
    DECLARE_INSTANCE_METHOD(void, SetPlayer, StringW url, UnityEngine::Material* roleMaterial, float hueShift, float saturation);
    DECLARE_INSTANCE_METHOD(void, SetHiddenPlayer);
    DECLARE_INSTANCE_METHOD(void, Update);
)