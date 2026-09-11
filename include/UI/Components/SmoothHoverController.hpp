#pragma once

#include "beatsaber-hook/shared/callback.hpp"
#include "UnityEngine/MonoBehaviour.hpp"
#include "UnityEngine/EventSystems/IPointerEnterHandler.hpp"
#include "UnityEngine/EventSystems/IPointerExitHandler.hpp"
#include "UnityEngine/EventSystems/PointerEventData.hpp"
#include "HMUI/ImageView.hpp"

#include "custom-types/shared/macros.hpp"

#include <string>
using namespace std;

#define GET_FIND_METHOD(mPtr) ::i2c::metadata_getter<mPtr>::method_info()

DECLARE_CLASS_CODEGEN_INTERFACES(
        BeatLeader, 
        SmoothHoverController, 
        UnityEngine::MonoBehaviour, 
        UnityEngine::EventSystems::IPointerEnterHandler*, 
        UnityEngine::EventSystems::IPointerExitHandler*
        ) {
    DECLARE_INSTANCE_FIELD(bool, IsHovered);
    DECLARE_INSTANCE_FIELD(float, Progress);
    DECLARE_INSTANCE_FIELD(bool, _set);
    DECLARE_INSTANCE_FIELD(float, _targetValue);
    DECLARE_INSTANCE_FIELD(float, lerpCoefficient);

    DECLARE_CTOR(ctor);

    public:
    using HoverStateChangedEvent = unordered_event_callback<bool, float>;

    HoverStateChangedEvent & get_hoverStateChangedEvent();
    HoverStateChangedEvent hoverStateChangedEvent;

    // Factory methods
    static SmoothHoverController* Custom(UnityEngine::GameObject* gameObject, std::function<void(bool, float)> handler);
    static SmoothHoverController* Scale(UnityEngine::GameObject* gameObject, float defaultScale, float hoverScale);
    static SmoothHoverController* Scale(UnityEngine::GameObject* gameObject, UnityEngine::Transform* target, float defaultScale, float hoverScale);
     
    DECLARE_OVERRIDE_METHOD(void, OnPointerEnter, ::i2c::metadata_getter<&UnityEngine::EventSystems::IPointerEnterHandler::OnPointerEnter>::method_info(), UnityEngine::EventSystems::PointerEventData*);
    DECLARE_OVERRIDE_METHOD(void, OnPointerExit, ::i2c::metadata_getter<&UnityEngine::EventSystems::IPointerExitHandler::OnPointerExit>::method_info(), UnityEngine::EventSystems::PointerEventData*);

    DECLARE_INSTANCE_METHOD(void, OnDisable);
    DECLARE_INSTANCE_METHOD(void, Update);
        };