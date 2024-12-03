#pragma once

#include "UnityEngine/Transform.hpp"
#include "UnityEngine/GameObject.hpp"
#include "HMUI/Screen.hpp"
#include "HMUI/ModalView.hpp"
#include "HMUI/ImageView.hpp"
#include "UnityEngine/MonoBehaviour.hpp"
#include "System/Collections/Generic/Dictionary_2.hpp"
#include "System/Collections/Generic/Stack_1.hpp"
#include "System/Action.hpp"
#include "UI/Abstract/AbstractReeModal.hpp"
#include "UI/ReeUIComponentV2.hpp"
#include "UnityEngine/SceneManagement/Scene.hpp"
#include "UnityEngine/SceneManagement/SceneManager.hpp"

#include <unordered_map>
#include <stack>

DECLARE_CLASS_CUSTOM(BeatLeader, ReeModalSystemComponent, BeatLeader::ReeComponent,
    DECLARE_INSTANCE_FIELD(UnityEngine::Transform*, _container);
    DECLARE_INSTANCE_FIELD(HMUI::ModalView*, _modalView);
    DECLARE_INSTANCE_FIELD(HMUI::Screen*, _screen);
)

namespace BeatLeader {

class ReeModalSystem : public ReeUIComponentV2<BeatLeader::ReeModalSystemComponent*> {
private:
    static std::unordered_map<int, ReeModalSystem*> activeModals;
    static void (*interruptAllEvent)();
    
    std::unordered_map<std::string, IReeModal*> pool;
    std::stack<std::pair<IReeModal*, Il2CppObject*>> modalStack;
    IReeModal* activeModal;
    Il2CppObject* activeModalState;
    bool hasActiveModal;

    void PopOpen(IReeModal* modal, Il2CppObject* state);
    void OpenImmediately();
    void CloseOrPop();
    void OnActiveSceneChanged(UnityEngine::SceneManagement::Scene from, UnityEngine::SceneManagement::Scene to);

public:
    void Construct(HMUI::Screen* screen);
    void OnInitialize() override;
    void OnDispose() override;
    void OnDisable();
    void InitializeModal();
    void OnBlockerClicked();
    void ShowModal(bool animated);
    void HideModal(bool animated);
    void InterruptAll();
    void ForceUpdate();
    StringW GetContent() override;

    template<typename T> 
    static BeatLeader::IReeModal* OpenModal(UnityEngine::Transform* screenChild, Il2CppObject* state) 
        requires std::is_base_of_v<IReeModal, T> && 
                 std::is_base_of_v<ReeUIComponentV2<typename T::ComponentType>, T>
    {
        auto screen = screenChild->GetComponentInParent<HMUI::Screen*>();
        return OpenModal<T>(screen, state);
    }

    template<typename T>
    static BeatLeader::IReeModal* OpenModal(HMUI::Screen* screen, Il2CppObject* state, bool interruptActiveModals = true)
        requires std::is_base_of_v<IReeModal, T> && 
                 std::is_base_of_v<ReeUIComponentV2<typename T::ComponentType>, T>
    {
        ReeModalSystem* controller;
        int key = screen->GetHashCode();

        auto it = activeModals.find(key);
        if (it != activeModals.end()) {
            controller = it->second;
        } else {
            controller = ReeModalSystem::Instantiate<ReeModalSystem>(screen->get_transform());
            controller->Construct(screen);
            controller->LocalComponent()->ManualInit(screen->get_transform());
            activeModals[key] = controller;
        }

        if (interruptActiveModals) {
            if (interruptAllEvent) (*interruptAllEvent)();
        }

        return controller->OpenModalInternal<T>(state);
    }

    template<typename T>
    BeatLeader::IReeModal* GetOrInstantiateModal()
        requires std::is_base_of_v<IReeModal, T> && 
                 std::is_base_of_v<ReeUIComponentV2<typename T::ComponentType>, T>
    {
        auto type = (std::string)typeid(T).name();
        auto it = pool.find(type);
        if (it != pool.end()) return it->second;

        auto component = T::template Instantiate<T>(LocalComponent()->_container);
        component->LocalComponent()->ManualInit(LocalComponent()->_container);
        
        pool[type] = component;
        return component;
    }

    template<typename T>
    BeatLeader::IReeModal* OpenModalInternal(Il2CppObject* state)
        requires std::is_base_of_v<IReeModal, T> && 
                 std::is_base_of_v<ReeUIComponentV2<typename T::ComponentType>, T>
    {
        auto modal = GetOrInstantiateModal<T>();
        PopOpen(modal, state);
        return modal;
    }

    static void ForceUpdateAll();
};

} // namespace BeatLeader