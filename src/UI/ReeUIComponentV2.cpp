#include "include/UI/ReeUIComponentV2.hpp"
#include "include/Utils/WebUtils.hpp"
#include "include/main.hpp"

#include "System/Action_1.hpp"

#include "bsml/shared/BSML.hpp"
#include "bsml/shared/BSMLDataCache.hpp"

#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/SceneManagement/Scene.hpp"
#include "UnityEngine/SceneManagement/SceneManager.hpp"

#include "System/Reflection/Assembly.hpp"
#include "custom-types/shared/delegate.hpp"

using namespace UnityEngine;
using namespace UnityEngine::SceneManagement;

DEFINE_TYPE(BeatLeader, ContentStateListener);
DEFINE_TYPE(BeatLeader, ReeComponent);

namespace BeatLeader {

    void ContentStateListener::OnEnable() {
        if (StateChangedEvent) {
            StateChangedEvent->Invoke(true);
        }
    }

    void ContentStateListener::OnDisable() {
        if (StateChangedEvent) {
            StateChangedEvent->Invoke(false); 
        }
    }

    void ReeComponent::OnInitialize() {
        auto reeComponent = (ReeUIComponentV2<ReeComponent*>*)nativeComponent;
        reeComponent->OnInitialize();
    }

    void ReeComponent::OnInstantiate() {
        auto reeComponent = (ReeUIComponentV2<ReeComponent*>*)nativeComponent;
        reeComponent->OnInstantiate();
    }

    void ReeComponent::OnDispose() {
        auto reeComponent = (ReeUIComponentV2<ReeComponent*>*)nativeComponent;
        reeComponent->OnDispose();
    }

    void ReeComponent::OnRootStateChange(bool active) {
        auto reeComponent = (ReeUIComponentV2<ReeComponent*>*)nativeComponent;
        reeComponent->OnRootStateChange(active);
    }

    void ReeComponent::OnDestroy() {
        if (_state != ReeComponentState::HierarchySet) return;
        OnDispose();
        if (_content) Destroy(_content->get_gameObject());
        _state = ReeComponentState::Uninitialized;
    }

    void ReeComponent::Setup(UnityEngine::Transform* parent, bool parseImmediately) {
        _parent = parent;
        _uiComponent = get_transform();
        _uiComponent->SetParent(parent, false);
        if (parseImmediately) ParseSelfIfNeeded();
        get_gameObject()->SetActive(false);
    }

    void ReeComponent::SetParent(UnityEngine::Transform* parent) {
        _parent = parent;
        get_transform()->SetParent(parent, false);
    }

    void ReeComponent::ManualInit(UnityEngine::Transform* rootNode) {
        DisposeIfNeeded();
        get_transform()->SetParent(rootNode, true);
        ApplyHierarchy();
        OnInitialize();
    }

    void ReeComponent::SetRootActive(bool active) {
        _content->get_gameObject()->SetActive(active);
    }

    UnityEngine::Transform* ReeComponent::GetRootTransform() {
        return _content;
    }

    void ReeComponent::PostParse() {
        if (_state == ReeComponentState::Parsing) return;
        DisposeIfNeeded();
        ParseSelfIfNeeded();
        ApplyHierarchy();
        OnInitialize();
    }

    void ReeComponent::DisposeIfNeeded() {
        if (_state != ReeComponentState::HierarchySet) return;
        OnDispose();
        _state = ReeComponentState::Uninitialized;
    }

    void ReeComponent::ParseSelfIfNeeded() {
        if (_state != ReeComponentState::Uninitialized) return;

        _state = ReeComponentState::Parsing;

        auto reeComponent = (ReeUIComponentV2<ReeComponent*>*)nativeComponent;

        auto bsml = (std::string)reeComponent->GetContent();
        BSML::parse_and_construct(bsml, get_gameObject()->get_transform(), this);

        _content = _uiComponent->GetChild(0);
        auto listener = _content->get_gameObject()->AddComponent<ContentStateListener*>();
        listener->StateChangedEvent = custom_types::MakeDelegate<System::Action_1<bool>*>(std::function<void(bool)>([this](bool active) {
            this->OnRootStateChange(active);
        }));
        
        _state = ReeComponentState::Parsed;
    }

    void ReeComponent::ApplyHierarchy() {
        _content->SetParent(_uiComponent->get_parent(), true);
        _uiComponent->SetParent(_parent, false);
        get_gameObject()->SetActive(true);
        _state = ReeComponentState::HierarchySet;
    }
}
