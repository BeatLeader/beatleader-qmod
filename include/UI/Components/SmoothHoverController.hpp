#pragma once

#include "UnityEngine/MonoBehaviour.hpp"
#include "UnityEngine/EventSystems/IPointerEnterHandler.hpp"
#include "UnityEngine/EventSystems/IPointerExitHandler.hpp"
#include "UnityEngine/EventSystems/PointerEventData.hpp"
#include "HMUI/ImageView.hpp"

#include "custom-types/shared/macros.hpp"

#include <string>
using namespace std;

#define GET_FIND_METHOD(mPtr) il2cpp_utils::il2cpp_type_check::MetadataGetter<mPtr>::get()

DECLARE_CLASS_CODEGEN_INTERFACES(
        BeatLeader, 
        SmoothHoverController, 
        UnityEngine::MonoBehaviour, 
        (vector<Il2CppClass*> { 
            classof(UnityEngine::EventSystems::IPointerEnterHandler*), 
            classof(UnityEngine::EventSystems::IPointerExitHandler*) 
        }),
    DECLARE_INSTANCE_FIELD(bool, IsHovered);
    DECLARE_INSTANCE_FIELD(float, Progress);
    DECLARE_INSTANCE_FIELD(bool, _set);
    DECLARE_INSTANCE_FIELD(float, _targetValue);

    DECLARE_CTOR(ctor);

    public:
    using HoverStateChangedEvent = UnorderedEventCallback<bool, float>;

    HoverStateChangedEvent & get_hoverStateChangedEvent();
    HoverStateChangedEvent hoverStateChangedEvent;
     
    DECLARE_OVERRIDE_METHOD(void, OnPointerEnter, il2cpp_utils::il2cpp_type_check::MetadataGetter<&UnityEngine::EventSystems::IPointerEnterHandler::OnPointerEnter>::get(), UnityEngine::EventSystems::PointerEventData*);
    DECLARE_OVERRIDE_METHOD(void, OnPointerExit, il2cpp_utils::il2cpp_type_check::MetadataGetter<&UnityEngine::EventSystems::IPointerExitHandler::OnPointerExit>::get(), UnityEngine::EventSystems::PointerEventData*);

    DECLARE_INSTANCE_METHOD(void, OnDisable);
    DECLARE_INSTANCE_METHOD(void, Update);
)