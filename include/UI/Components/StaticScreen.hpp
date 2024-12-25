#pragma once

#include "UnityEngine/MonoBehaviour.hpp"
#include "UnityEngine/CanvasGroup.hpp"
#include "UnityEngine/Canvas.hpp"
#include "UnityEngine/Vector3.hpp"
#include "HMUI/Screen.hpp"
#include "VRUIControls/VRGraphicRaycaster.hpp"
#include "custom-types/shared/macros.hpp"


DECLARE_CLASS_CODEGEN(BeatLeader, StaticScreen, UnityEngine::MonoBehaviour,
    DECLARE_INSTANCE_FIELD(UnityEngine::CanvasGroup*, canvasGroup);
    DECLARE_INSTANCE_FIELD(float, targetAlpha);
    DECLARE_INSTANCE_FIELD(bool, set);

    DECLARE_INSTANCE_METHOD(void, Present);
    DECLARE_INSTANCE_METHOD(void, Dismiss);
    DECLARE_INSTANCE_METHOD(void, Awake);
    DECLARE_INSTANCE_METHOD(void, Update);
)