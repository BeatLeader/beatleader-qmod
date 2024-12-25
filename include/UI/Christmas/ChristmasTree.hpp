#pragma once

#include "custom-types/shared/macros.hpp"
#include "UnityEngine/MonoBehaviour.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/EventSystems/IPointerDownHandler.hpp"
#include "UnityEngine/EventSystems/IPointerEnterHandler.hpp"
#include "UnityEngine/EventSystems/IPointerExitHandler.hpp"
#include "UnityEngine/EventSystems/PointerEventData.hpp"
#include "System/Collections/Generic/List_1.hpp"
#include "ChristmasTreeLevel.hpp"
#include "ChristmasTreeAnimator.hpp"
#include "ChristmasTreeMover.hpp"
#include "ChristmasTreeOrnament.hpp"
#include "Models/ChristmasTreeSettings.hpp"

DECLARE_CLASS_CODEGEN_INTERFACES(BeatLeader, ChristmasTree, UnityEngine::MonoBehaviour, 
    (std::vector<Il2CppClass*> { 
        classof(UnityEngine::EventSystems::IPointerDownHandler*),
        classof(UnityEngine::EventSystems::IPointerEnterHandler*),
        classof(UnityEngine::EventSystems::IPointerExitHandler*)
    }),
    DECLARE_INSTANCE_FIELD(System::Collections::Generic::List_1<BeatLeader::ChristmasTreeLevel*>*, _levels);
    DECLARE_INSTANCE_FIELD(BeatLeader::ChristmasTreeAnimator*, _animator);
    DECLARE_INSTANCE_FIELD(BeatLeader::ChristmasTreeMover*, _mover);
    DECLARE_INSTANCE_FIELD(UnityEngine::Transform*, _mesh);
    DECLARE_INSTANCE_FIELD(float, _radius);
    DECLARE_INSTANCE_FIELD(bool, gizmos);
    DECLARE_INSTANCE_FIELD(bool, _moverFull);
    DECLARE_INSTANCE_FIELD(bool, _moverRestricted);
    DECLARE_INSTANCE_FIELD(bool, _isSpinning);
    DECLARE_INSTANCE_FIELD(float, _spinSpeed);
    DECLARE_INSTANCE_FIELD(bool, _hovered);
    DECLARE_INSTANCE_FIELD(System::Collections::Generic::List_1<ChristmasTreeOrnament*>*, _ornaments);

    DECLARE_INSTANCE_METHOD(void, Present);
    DECLARE_INSTANCE_METHOD(void, Dismiss);
    DECLARE_INSTANCE_METHOD(void, MoveTo, UnityEngine::Vector3 pos, bool immediate);
    DECLARE_INSTANCE_METHOD(void, ScaleTo, float size, bool immediate);
    DECLARE_INSTANCE_METHOD(void, Awake);
    DECLARE_INSTANCE_METHOD(void, Update);
    DECLARE_INSTANCE_METHOD(UnityEngine::Vector3, Align, UnityEngine::Vector3 pos);
    DECLARE_INSTANCE_METHOD(bool, HasAreaContact, UnityEngine::Vector3 pos);
    DECLARE_INSTANCE_METHOD(void, SetOrnamentsMovement, bool value);
    DECLARE_INSTANCE_METHOD(void, SetMoverFull, bool value);
    DECLARE_INSTANCE_METHOD(void, SetMoverRestricted, bool value);
    DECLARE_INSTANCE_METHOD(void, StartSpinning);
    DECLARE_INSTANCE_METHOD(void, StopSpinning);
    DECLARE_INSTANCE_METHOD(void, ClearOrnaments);
    DECLARE_INSTANCE_METHOD(void, AddOrnament, ChristmasTreeOrnament* ornament);
    DECLARE_INSTANCE_METHOD(void, RemoveOrnament, ChristmasTreeOrnament* ornament);

    // IPointerHandler implementations
    DECLARE_OVERRIDE_METHOD(void, OnPointerEnter, il2cpp_utils::il2cpp_type_check::MetadataGetter<&UnityEngine::EventSystems::IPointerEnterHandler::OnPointerEnter>::methodInfo(), UnityEngine::EventSystems::PointerEventData*);
    DECLARE_OVERRIDE_METHOD(void, OnPointerExit, il2cpp_utils::il2cpp_type_check::MetadataGetter<&UnityEngine::EventSystems::IPointerExitHandler::OnPointerExit>::methodInfo(), UnityEngine::EventSystems::PointerEventData*);
    DECLARE_OVERRIDE_METHOD(void, OnPointerDown, il2cpp_utils::il2cpp_type_check::MetadataGetter<&UnityEngine::EventSystems::IPointerDownHandler::OnPointerDown>::methodInfo(), UnityEngine::EventSystems::PointerEventData*);

    public:
        UnityEngine::Transform* get_Origin() { return _mesh; }
        void LoadOrnaments(ChristmasTreeSettings settings, int index, std::function<void()> callback);
        void LoadSettings(ChristmasTreeSettings settings, bool move);

    private:
        ChristmasTreeSettings settings;
) 