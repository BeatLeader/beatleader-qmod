#pragma once

#include "UnityEngine/MonoBehaviour.hpp"
#include "UnityEngine/EventSystems/UIBehaviour.hpp"
#include "UnityEngine/EventSystems/IPointerDownHandler.hpp"
#include "UnityEngine/EventSystems/PointerEventData.hpp"
#include "UnityEngine/UI/Graphic.hpp"
#include "UnityEngine/GameObject.hpp"

#include "custom-types/shared/macros.hpp"

#include <string>
using namespace std;

#define GET_FIND_METHOD(mPtr) il2cpp_utils::il2cpp_type_check::MetadataGetter<mPtr>::methodInfo()

DECLARE_CLASS_CODEGEN_INTERFACES(
    BeatLeader,
    SimpleClickHandler,
    UnityEngine::EventSystems::UIBehaviour,
    UnityEngine::EventSystems::IPointerDownHandler* ) {

    DECLARE_CTOR(ctor);
    
    public:
        using ClickEvent = UnorderedEventCallback<bool>;
        
        ClickEvent& get_clickEvent();
        ClickEvent clickEvent;

        // Factory method
        static SimpleClickHandler* Custom(UnityEngine::GameObject* gameObject, std::function<void(bool)> clickHandler);

        DECLARE_OVERRIDE_METHOD(void, Awake, il2cpp_utils::il2cpp_type_check::MetadataGetter<&UnityEngine::EventSystems::UIBehaviour::Awake>::methodInfo());
        DECLARE_OVERRIDE_METHOD(void, OnPointerDown, il2cpp_utils::il2cpp_type_check::MetadataGetter<&UnityEngine::EventSystems::IPointerDownHandler::OnPointerDown>::methodInfo(), UnityEngine::EventSystems::PointerEventData*);
    };