#include "UI/Components/SimpleClickHandler.hpp"
#include "bsml/shared/BSML-Lite.hpp"

#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/UI/Graphic.hpp"

using namespace UnityEngine;
using namespace UnityEngine::EventSystems;
using namespace UnityEngine::UI;
using namespace BSML;

DEFINE_TYPE(BeatLeader, SimpleClickHandler);

void BeatLeader::SimpleClickHandler::ctor() {
    INVOKE_CTOR();
    clickEvent = ClickEvent();
    
    static auto base_ctor = il2cpp_utils::FindMethod(classof(UnityEngine::EventSystems::UIBehaviour*), ".ctor");
    if (base_ctor) {
        il2cpp_utils::RunMethod(this, base_ctor);
    }
}

BeatLeader::SimpleClickHandler* BeatLeader::SimpleClickHandler::Custom(UnityEngine::GameObject* gameObject, std::function<void(bool)> clickHandler) {
    auto* component = gameObject->AddComponent<BeatLeader::SimpleClickHandler*>();
    component->get_clickEvent() += clickHandler;
    return component;
}

void BeatLeader::SimpleClickHandler::Awake() {
    GetComponent<Graphic*>()->set_raycastTarget(true);
}

void BeatLeader::SimpleClickHandler::OnPointerDown(PointerEventData* eventData) {
    if (eventData->get_button() != PointerEventData::InputButton::Left) return;
    // BasicUIAudioManager::HandleButtonClickEvent();
    clickEvent.invoke(true);
}

BeatLeader::SimpleClickHandler::ClickEvent& BeatLeader::SimpleClickHandler::get_clickEvent() {
    return clickEvent;
} 