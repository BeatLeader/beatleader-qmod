#pragma once

#include "UnityEngine/MonoBehaviour.hpp"
#include "UnityEngine/Material.hpp"

#include "custom-types/shared/macros.hpp"

#define GET_FIND_METHOD(mPtr) il2cpp_utils::il2cpp_type_check::MetadataGetter<mPtr>::get()

DECLARE_CLASS_CODEGEN(BeatLeader, LogoAnimation, UnityEngine::MonoBehaviour,
    DECLARE_INSTANCE_FIELD(HMUI::ImageView*, imageView);
    DECLARE_INSTANCE_FIELD(UnityEngine::Material*, materialInstance);
    DECLARE_INSTANCE_FIELD(bool, glowing);

    DECLARE_INSTANCE_METHOD(void, Init, HMUI::ImageView* imageView);
    DECLARE_INSTANCE_METHOD(void, SetAnimating, bool animating);
    DECLARE_INSTANCE_METHOD(void, SetGlowing, bool glowing);
    DECLARE_INSTANCE_METHOD(void, Update);
)