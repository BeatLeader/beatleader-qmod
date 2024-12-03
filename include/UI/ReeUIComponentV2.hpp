#pragma once

#include "UnityEngine/MonoBehaviour.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/SceneManagement/Scene.hpp"
#include "UnityEngine/SceneManagement/SceneManager.hpp"
#include "System/Collections/Generic/Dictionary_2.hpp"
#include "System/Collections/Generic/List_1.hpp"
#include "System/Action_1.hpp"
#include "bsml/shared/BSML.hpp"
#include "bsml/shared/BSML/Parsing/BSMLParserParams.hpp"
#include "custom-types/shared/macros.hpp"
#include "main.hpp"

#include <string>
#include <unordered_map>
#include <stdexcept>

DECLARE_CLASS_CODEGEN(BeatLeader, ContentStateListener, UnityEngine::MonoBehaviour,
    DECLARE_INSTANCE_FIELD(System::Action_1<bool>*, StateChangedEvent);

    DECLARE_INSTANCE_METHOD(void, OnEnable);
    DECLARE_INSTANCE_METHOD(void, OnDisable);
)

class ReeComponentState {
    public:
    static const int Uninitialized = 0;
    static const int Parsing = 1;
    static const int Parsed = 2;
    static const int HierarchySet = 3;
};

DECLARE_CLASS_CODEGEN(BeatLeader, ReeComponent, UnityEngine::MonoBehaviour,
    DECLARE_INSTANCE_FIELD(UnityEngine::Transform*, _uiComponent);
    DECLARE_INSTANCE_FIELD(UnityEngine::Transform*, _parent);
    DECLARE_INSTANCE_FIELD(UnityEngine::Transform*, _content);
    DECLARE_INSTANCE_FIELD(int, _state);

    DECLARE_INSTANCE_METHOD(void, OnInitialize);
    DECLARE_INSTANCE_METHOD(void, OnInstantiate);
    DECLARE_INSTANCE_METHOD(void, OnDestroy);
    DECLARE_INSTANCE_METHOD(void, OnDispose);
    DECLARE_INSTANCE_METHOD(void, OnRootStateChange, bool active);
    DECLARE_INSTANCE_METHOD(void, Setup, UnityEngine::Transform* parent, bool parseImmediately);
    DECLARE_INSTANCE_METHOD(void, SetParent, UnityEngine::Transform* parent);
    DECLARE_INSTANCE_METHOD(void, ManualInit, UnityEngine::Transform* rootNode);
    DECLARE_INSTANCE_METHOD(void, SetRootActive, bool active);
    DECLARE_INSTANCE_METHOD(UnityEngine::Transform*, GetRootTransform);
    DECLARE_INSTANCE_METHOD(void, PostParse);
    DECLARE_INSTANCE_METHOD(void, DisposeIfNeeded);
    DECLARE_INSTANCE_METHOD(void, ParseSelfIfNeeded);
    DECLARE_INSTANCE_METHOD(void, ApplyHierarchy);

    public:
    void* nativeComponent = nullptr;
)

namespace BeatLeader {

template<typename C = BeatLeader::ReeComponent*>
requires(std::is_convertible_v<C, BeatLeader::ReeComponent*>) 
class ReeUIComponentV2 {
    public:
    C component;

    virtual C LocalComponent() {
        return component;
    }
    
    virtual void OnInitialize() {
        BeatLeaderLogger.error("OnInitialize ReeUIComponentV2");
    }

    virtual void OnInstantiate() {
        BeatLeaderLogger.error("OnInstantiate ReeUIComponentV2");
    }

    virtual void OnDispose() {
        BeatLeaderLogger.error("OnDispose ReeUIComponentV2");
    }

    virtual void OnRootStateChange(bool active) {
        BeatLeaderLogger.error("OnRootStateChange ReeUIComponentV2");
    }
    
    virtual StringW GetContent() { return StringW(""); }

    template<typename T>
    static T* InstantiateOnSceneRoot(bool parseImmediately = true) {
        auto lastLoadedScene = UnityEngine::SceneManagement::SceneManager::GetSceneAt(
            UnityEngine::SceneManagement::SceneManager::get_sceneCount() - 1
        );
        auto sceneRoot = lastLoadedScene.GetRootGameObjects()[0]->get_transform();
        return Instantiate<T>(sceneRoot, parseImmediately);
    }

    template<typename T>
    static T* Instantiate(UnityEngine::Transform* parent, bool parseImmediately = true) {
        auto gameObject = UnityEngine::GameObject::New_ctor(il2cpp_utils::newcsstr(typeid(T).name()));
       
        T* object = new T();
        auto component = gameObject->AddComponent<C>();
        object->component = component;
        component->nativeComponent = object;
        component->OnInstantiate();
        component->Setup(parent, parseImmediately);
        
        // object->component->reeComponent = object;
        return object;
    }
};

} // namespace BeatLeader
