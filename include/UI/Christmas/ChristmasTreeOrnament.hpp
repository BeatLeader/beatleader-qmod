#pragma once

#include "UnityEngine/MonoBehaviour.hpp"
#include "UnityEngine/Rigidbody.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Quaternion.hpp"
#include "UnityEngine/EventSystems/PointerEventData.hpp"
#include "UnityEngine/EventSystems/IPointerDownHandler.hpp"
#include "UnityEngine/EventSystems/IPointerUpHandler.hpp"
#include "UnityEngine/EventSystems/IPointerEnterHandler.hpp"
#include "UnityEngine/EventSystems/IPointerExitHandler.hpp"
#include "VRUIControls/VRPointer.hpp"
#include "Models/ChristmasTreeOrnamentSettings.hpp"
#include "System/Action.hpp"
#include "custom-types/shared/delegate.hpp"

DECLARE_CLASS_CODEGEN_INTERFACES(BeatLeader, ChristmasTreeOrnament, UnityEngine::MonoBehaviour,
    (std::vector<Il2CppClass*> { 
        classof(UnityEngine::EventSystems::IPointerDownHandler*),
        classof(UnityEngine::EventSystems::IPointerUpHandler*),
        classof(UnityEngine::EventSystems::IPointerEnterHandler*),
        classof(UnityEngine::EventSystems::IPointerExitHandler*)
    }),
    DECLARE_INSTANCE_FIELD(UnityEngine::Rigidbody*, rigidbody);
    DECLARE_INSTANCE_FIELD(UnityEngine::MonoBehaviour*, tree);
    DECLARE_INSTANCE_FIELD(bool, initialized);
    DECLARE_INSTANCE_FIELD(bool, canGrab);
    DECLARE_INSTANCE_FIELD(int, bundleId);

    // Input state
    DECLARE_INSTANCE_FIELD(bool, hadContact);
    DECLARE_INSTANCE_FIELD(bool, hovered);
    DECLARE_INSTANCE_FIELD(bool, grabbed);
    DECLARE_INSTANCE_FIELD(UnityEngine::Transform*, grabbingController);
    DECLARE_INSTANCE_FIELD(UnityEngine::Vector3, alignedOrnamentPos);
    DECLARE_INSTANCE_FIELD(UnityEngine::Vector3, grabPos);
    DECLARE_INSTANCE_FIELD(UnityEngine::Quaternion, grabRot);
    DECLARE_INSTANCE_FIELD(UnityEngine::Vector3, controllerLastPos);
    DECLARE_INSTANCE_FIELD(UnityEngine::Vector3, controllerVelocity);

    DECLARE_INSTANCE_FIELD(System::Action*, _grabbedAction);

    // Events
    // DECLARE_INSTANCE_FIELD(custom_types::Delegate<void, ChristmasTreeOrnament*>, OrnamentDeinitEvent);
    // DECLARE_INSTANCE_FIELD(custom_types::Delegate<void, ChristmasTreeOrnament*>, OrnamentGrabbedEvent);

    DECLARE_INSTANCE_METHOD(void, SetupTree, UnityEngine::MonoBehaviour* tree, int bundleId);
    DECLARE_INSTANCE_METHOD(void, Init, UnityEngine::Transform* parent);
    DECLARE_INSTANCE_METHOD(void, Awake);
    DECLARE_INSTANCE_METHOD(void, Update);

    // Input handlers
    DECLARE_OVERRIDE_METHOD(void, OnPointerDown, il2cpp_utils::il2cpp_type_check::MetadataGetter<&UnityEngine::EventSystems::IPointerDownHandler::OnPointerDown>::methodInfo(), UnityEngine::EventSystems::PointerEventData*);
    DECLARE_OVERRIDE_METHOD(void, OnPointerUp, il2cpp_utils::il2cpp_type_check::MetadataGetter<&UnityEngine::EventSystems::IPointerUpHandler::OnPointerUp>::methodInfo(), UnityEngine::EventSystems::PointerEventData*);
    DECLARE_OVERRIDE_METHOD(void, OnPointerEnter, il2cpp_utils::il2cpp_type_check::MetadataGetter<&UnityEngine::EventSystems::IPointerEnterHandler::OnPointerEnter>::methodInfo(), UnityEngine::EventSystems::PointerEventData*);
    DECLARE_OVERRIDE_METHOD(void, OnPointerExit, il2cpp_utils::il2cpp_type_check::MetadataGetter<&UnityEngine::EventSystems::IPointerExitHandler::OnPointerExit>::methodInfo(), UnityEngine::EventSystems::PointerEventData*);
public:
    ChristmasTreeOrnamentSettings GetSettings();

private:
    static constexpr float MAX_DISTANCE = 0.17f;
)