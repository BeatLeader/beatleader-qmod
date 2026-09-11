#pragma once

#include "beatsaber-hook/shared/callback.hpp"
#include "UnityEngine/MonoBehaviour.hpp"
#include "UnityEngine/EventSystems/UIBehaviour.hpp"
#include "UnityEngine/EventSystems/IPointerDownHandler.hpp"
#include "UnityEngine/EventSystems/PointerEventData.hpp"
#include "UnityEngine/UI/Graphic.hpp"
#include "UnityEngine/GameObject.hpp"

#include "custom-types/shared/macros.hpp"

#include <string>
using namespace std;

#define GET_FIND_METHOD(mPtr) ::i2c::metadata_getter<mPtr>::method_info()

DECLARE_CLASS_CODEGEN_INTERFACES(
    BeatLeader,
    SimpleClickHandler,
    UnityEngine::EventSystems::UIBehaviour,
    UnityEngine::EventSystems::IPointerDownHandler* ) {

    DECLARE_CTOR(ctor);
    
    public:
        using ClickEvent = unordered_event_callback<bool>;
        
        ClickEvent& get_clickEvent();
        ClickEvent clickEvent;

        // Factory method
        static SimpleClickHandler* Custom(UnityEngine::GameObject* gameObject, std::function<void(bool)> clickHandler);

        DECLARE_OVERRIDE_METHOD(void, Awake, ::i2c::metadata_getter<&UnityEngine::EventSystems::UIBehaviour::Awake>::method_info());
        DECLARE_OVERRIDE_METHOD(void, OnPointerDown, ::i2c::metadata_getter<&UnityEngine::EventSystems::IPointerDownHandler::OnPointerDown>::method_info(), UnityEngine::EventSystems::PointerEventData*);
    };