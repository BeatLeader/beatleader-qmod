#pragma once

#include "custom-types/shared/macros.hpp"
#include "UnityEngine/MonoBehaviour.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/Material.hpp"
#include "UnityEngine/Color.hpp"
#include "UnityEngine/Shader.hpp"
#include "UnityEngine/EventSystems/PointerEventData.hpp"
#include "UnityEngine/EventSystems/IPointerDownHandler.hpp"
#include "UnityEngine/EventSystems/IPointerUpHandler.hpp"
#include "UnityEngine/EventSystems/IPointerEnterHandler.hpp"
#include "UnityEngine/EventSystems/IPointerExitHandler.hpp"
#include "GlobalNamespace/VRController.hpp"
#include "ValueAnimator.hpp"
#include "UI/Utils/ReeTransform.hpp"

DECLARE_CLASS_CODEGEN_INTERFACES(BeatLeader, ChristmasTreeMover, UnityEngine::MonoBehaviour,
    (std::vector<Il2CppClass*> { 
        classof(UnityEngine::EventSystems::IPointerDownHandler*),
        classof(UnityEngine::EventSystems::IPointerUpHandler*),
        classof(UnityEngine::EventSystems::IPointerEnterHandler*),
        classof(UnityEngine::EventSystems::IPointerExitHandler*)
    }),
    DECLARE_INSTANCE_FIELD(UnityEngine::Transform*, _container);
    DECLARE_INSTANCE_FIELD(UnityEngine::Material*, _material);
    DECLARE_INSTANCE_FIELD(ValueAnimator*, _highlightAnimator);
    DECLARE_INSTANCE_FIELD(ValueAnimator*, _scaleAnimator);
    
    private:
        bool hovered = false;
        bool grabbed = false;
        bool full = false;
        bool restricted = false;
        
        UnityEngine::Transform* grabbingController = nullptr;
        GlobalNamespace::VRController* grabbingVRController = nullptr;
        
        ReeTransform attachmentLocalPose;
        UnityEngine::Vector3 attachmentLocalUp;
        UnityEngine::Vector3 attachmentWorldForward;
        ReeTransform grabWorldPose;
        UnityEngine::Quaternion grabRotation;
        UnityEngine::Vector3 grabScale;
        float rotOffset = 0.0f;

    DECLARE_INSTANCE_METHOD(void, SetEnabled, bool full, bool restricted);
    DECLARE_INSTANCE_METHOD(void, RefreshHighlight);
    DECLARE_INSTANCE_METHOD(void, RefreshColor);
    DECLARE_INSTANCE_METHOD(void, RefreshScale);
    DECLARE_INSTANCE_METHOD(void, Update);
    DECLARE_INSTANCE_METHOD(void, Awake);

    DECLARE_OVERRIDE_METHOD(void, OnPointerDown, il2cpp_utils::il2cpp_type_check::MetadataGetter<&UnityEngine::EventSystems::IPointerDownHandler::OnPointerDown>::methodInfo(), UnityEngine::EventSystems::PointerEventData*);
    DECLARE_OVERRIDE_METHOD(void, OnPointerUp, il2cpp_utils::il2cpp_type_check::MetadataGetter<&UnityEngine::EventSystems::IPointerUpHandler::OnPointerUp>::methodInfo(), UnityEngine::EventSystems::PointerEventData*);
    DECLARE_OVERRIDE_METHOD(void, OnPointerEnter, il2cpp_utils::il2cpp_type_check::MetadataGetter<&UnityEngine::EventSystems::IPointerEnterHandler::OnPointerEnter>::methodInfo(), UnityEngine::EventSystems::PointerEventData*);
    DECLARE_OVERRIDE_METHOD(void, OnPointerExit, il2cpp_utils::il2cpp_type_check::MetadataGetter<&UnityEngine::EventSystems::IPointerExitHandler::OnPointerExit>::methodInfo(), UnityEngine::EventSystems::PointerEventData*);

    DECLARE_CTOR(ctor);
) 