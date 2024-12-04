#include "UI/Abstract/ReeModalSystem.hpp"

#include "UnityEngine/WaitForEndOfFrame.hpp"
#include "UnityEngine/SceneManagement/SceneManager.hpp"
#include "UnityEngine/SceneManagement/Scene.hpp"

#include "custom-types/shared/delegate.hpp"
#include "bsml/shared/BSML/MainThreadScheduler.hpp"
#include "HMUI/Touchable.hpp"
#include "main.hpp"

std::unordered_map<int, BeatLeader::ReeModalSystem*> BeatLeader::ReeModalSystem::activeModals;
void (*BeatLeader::ReeModalSystem::interruptAllEvent)() = nullptr;

DEFINE_TYPE(BeatLeader, ReeModalSystemComponent);

void BeatLeader::ReeModalSystem::Construct(HMUI::Screen* screen) {
    LocalComponent()->_screen = screen;
}

void BeatLeader::ReeModalSystem::InitializeModal() {
    if (!LocalComponent()->_modalView) return;
    
    auto background = LocalComponent()->_modalView->GetComponentInChildren<HMUI::ImageView*>();
    if (background) background->set_enabled(false);
    
    auto touchable = LocalComponent()->_modalView->GetComponentInChildren<HMUI::Touchable*>();
    if (touchable) touchable->set_enabled(false);
    
    auto area = LocalComponent()->_container->GetComponent<HMUI::ImageView*>();
    if (area) area->set_raycastTarget(true);
        
    LocalComponent()->_modalView->add_blockerClickedEvent(custom_types::MakeDelegate<System::Action*>(std::function<void()>([this]() { this->OnBlockerClicked(); })));
}

void BeatLeader::ReeModalSystem::PopOpen(BeatLeader::IReeModal* modal, Il2CppObject* state) {
    if (!hasActiveModal) {
        modalStack.push({modal, state});
        OpenImmediately();
    } else {
        modalStack.push({activeModal, activeModalState});
        modalStack.push({modal, state});
        CloseOrPop();
    }
}

void BeatLeader::ReeModalSystem::OpenImmediately() {
    if (modalStack.empty()) return;
    
    auto [modal, state] = modalStack.top();
    modalStack.pop();
    
    activeModal = modal;
    activeModalState = state;
    hasActiveModal = true;
        
    auto closeCallback = custom_types::MakeDelegate<System::Action*>(std::function<void()>([this]() { this->CloseOrPop(); }));
    
    activeModal->Resume(activeModalState, closeCallback);
    ShowModal(true);
}

void BeatLeader::ReeModalSystem::CloseOrPop() {
    if (!activeModal) return;
    
    activeModal->Pause();
    
    if (!modalStack.empty()) {
        OpenImmediately();
        return;
    }
    
    activeModal = nullptr;
    activeModalState = nullptr;
    hasActiveModal = false;
    HideModal(true);
}

void BeatLeader::ReeModalSystem::OnDisable() {
    InterruptAll();
}

void BeatLeader::ReeModalSystem::OnBlockerClicked() {
    if (!hasActiveModal) return;
    activeModal->HandleOffClick();
}

void BeatLeader::ReeModalSystem::ShowModal(bool animated) {
    if (!LocalComponent()->_modalView) return;
    
    if (LocalComponent()->_modalView->_viewIsValid) {
        LocalComponent()->_modalView->get_transform()->SetParent(LocalComponent()->get_transform(), false);
        LocalComponent()->_modalView->_viewIsValid = false;
    }
    
    LocalComponent()->_modalView->_animateParentCanvas = true;
    LocalComponent()->_modalView->Show(animated, true, nullptr);
}

void BeatLeader::ReeModalSystem::HideModal(bool animated) {
    if (!LocalComponent()->_modalView) return;
    LocalComponent()->_modalView->Hide(animated, nullptr);
}

void BeatLeader::ReeModalSystem::InterruptAll() {
    if (!hasActiveModal) return;
    
    activeModal->Interrupt();
    
    while (!modalStack.empty()) {
        auto [modal, state] = modalStack.top();
        modal->Interrupt();
        modalStack.pop();
    }
    
    activeModal = nullptr;
    activeModalState = nullptr;
    hasActiveModal = false;
    HideModal(false);
}

void BeatLeader::ReeModalSystem::ForceUpdate() {
    if (!hasActiveModal) return;

    BSML::MainThreadScheduler::Schedule([this] {
        this->HideModal(false);
        this->ShowModal(false);
    });
}

void BeatLeader::ReeModalSystem::ForceUpdateAll() {
    for (auto& [_, modal] : activeModals) {
        modal->ForceUpdate();
    }
}

StringW BeatLeader::ReeModalSystem::GetContent() {
    return StringW(R"(
        <horizontal ignore-layout="true">
            <modal id="_modalView" click-off-closes="false" size-delta-x="100" size-delta-y="100">
                <vertical vertical-fit="PreferredSize">
                    <horizontal id="_container" horizontal-fit="PreferredSize"/>
                </vertical>
            </modal>
        </horizontal>
    )");
}


void BeatLeader::ReeModalSystem::OnInitialize() {
    ReeUIComponentV2::OnInitialize();

    if (!interruptAllEvent) {
        interruptAllEvent = []() {
            for (auto& [_, modal] : activeModals) {
                modal->InterruptAll();
            }
        };
    }

    InitializeModal();
}

void BeatLeader::ReeModalSystem::OnDispose() {
    InterruptAll();
    pool.clear();
    ReeUIComponentV2::OnDispose();
}
