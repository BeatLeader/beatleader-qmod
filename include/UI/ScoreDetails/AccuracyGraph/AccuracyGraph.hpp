#pragma once

#include "AccuracyGraphLine.hpp"

#include "HMUI/ImageView.hpp"
#include "HMUI/ModalView.hpp"

#include "UnityEngine/EventSystems/UIBehaviour.hpp"
#include "UnityEngine/MonoBehaviour.hpp"
#include "UnityEngine/Material.hpp"
#include "UnityEngine/Vector2.hpp"
#include "UnityEngine/Rect.hpp"
#include "UnityEngine/Canvas.hpp"
#include "UnityEngine/UI/Graphic.hpp"

#include "include/Utils/FormatUtils.hpp"
#include "include/Utils/Range.hpp"

#include "UnityEngine/Resources.hpp"
#include "UnityEngine/Component.hpp"

#include "HMUI/ImageView.hpp"
#include "HMUI/CurvedCanvasSettingsHelper.hpp"
#include "HMUI/CurvedCanvasSettings.hpp"

#include "UnityEngine/RectTransformUtility.hpp"
#include "UnityEngine/Shader.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/Time.hpp"

#include "bsml/shared/BSML-Lite.hpp"
#include "bsml/shared/BSML.hpp"

#include "VRUIControls/VRPointer.hpp"

#include "custom-types/shared/macros.hpp"

using namespace std;

#define GET_FIND_METHOD(mPtr) il2cpp_utils::il2cpp_type_check::MetadataGetter<mPtr>::methodInfo()

DECLARE_CLASS_CODEGEN(BeatLeader, AccuracyGraph, UnityEngine::EventSystems::UIBehaviour,
    DECLARE_INSTANCE_FIELD(AccuracyGraphLine*, graphLine);
    DECLARE_INSTANCE_FIELD(UnityEngine::Material*, backgroundMaterial);
    DECLARE_INSTANCE_FIELD(HMUI::ModalView*, modal);

    DECLARE_INSTANCE_METHOD(void, 
        Construct, 
        HMUI::ImageView* backgroundImage, 
        BeatLeader::AccuracyGraphLine* graphLineObject,
        HMUI::ModalView* modal);
    DECLARE_INSTANCE_METHOD(void, Setup, ArrayW<float> points, float songDuration);
    
    DECLARE_INSTANCE_METHOD(float, GetAccuracy, float viewTime);
    DECLARE_INSTANCE_METHOD(float, GetCanvasRadius);
    DECLARE_INSTANCE_METHOD(void, Update);
    DECLARE_INSTANCE_METHOD(void, LateUpdate);

    DECLARE_INSTANCE_FIELD(ArrayW<float>, points);
    DECLARE_INSTANCE_FIELD(float, songDuration);
    DECLARE_INSTANCE_FIELD(VRUIControls::VRPointer*, vrPointer);
    DECLARE_INSTANCE_FIELD(bool, cursorInitialized);

    DECLARE_INSTANCE_FIELD(float, targetViewTime);
    DECLARE_INSTANCE_FIELD(float, currentViewTime);

    DECLARE_INSTANCE_FIELD(TMPro::TextMeshProUGUI*, underlineText);
    DECLARE_INSTANCE_FIELD(HMUI::CurvedCanvasSettingsHelper*, curvedCanvasSettingsHelper);
)