#include "HMUI/Touchable.hpp"

#include "bsml/shared/BSML.hpp"
#include "bsml/shared/BSML-Lite.hpp"
#include "bsml/shared/BSML/MainThreadScheduler.hpp"

#include "UnityEngine/Application.hpp"
#include "UnityEngine/GUIUtility.hpp"
#include "UnityEngine/HideFlags.hpp"
#include "UnityEngine/Object.hpp"
#include "UnityEngine/Time.hpp"
#include "UnityEngine/SpriteMeshType.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/Vector3.hpp"

#include "Utils/ModConfig.hpp"
#include "Utils/WebUtils.hpp"
#include "UI/RoleColorScheme.hpp"
#include "UI/Components/SmoothHoverController.hpp"
#include "UI/ScoreDetails/AccuracyGraph/AccuracyGraphUtils.hpp"
#include "API/PlayerController.hpp"
#include "Assets/Sprites.hpp"
#include "Assets/BundleLoader.hpp"

#include "main.hpp"

using namespace BSML;
using namespace UnityEngine;

DEFINE_TYPE(BeatLeader, SmoothHoverController);

void BeatLeader::SmoothHoverController::ctor() {
    INVOKE_CTOR();
    hoverStateChangedEvent = HoverStateChangedEvent();
    lerpCoefficient = 10.0f;
    
    static auto base_ctor = il2cpp_utils::FindMethod(classof(UnityEngine::MonoBehaviour*), ".ctor");
    if (base_ctor) {
        il2cpp_utils::RunMethod(this, base_ctor);
    }
}

BeatLeader::SmoothHoverController* BeatLeader::SmoothHoverController::Custom(UnityEngine::GameObject* gameObject, std::function<void(bool, float)> handler) {
    auto* component = gameObject->AddComponent<BeatLeader::SmoothHoverController*>();
    component->get_hoverStateChangedEvent() += handler;
    handler(component->IsHovered, component->Progress);
    return component;
}

BeatLeader::SmoothHoverController* BeatLeader::SmoothHoverController::Scale(UnityEngine::GameObject* gameObject, float defaultScale, float hoverScale) {
    return Scale(gameObject, gameObject->get_transform(), defaultScale, hoverScale);
}

BeatLeader::SmoothHoverController* BeatLeader::SmoothHoverController::Scale(UnityEngine::GameObject* gameObject, UnityEngine::Transform* target, float defaultScale, float hoverScale) {
    auto* component = gameObject->AddComponent<BeatLeader::SmoothHoverController*>();
    component->get_hoverStateChangedEvent() += [target, defaultScale, hoverScale](bool hovered, float progress) {
        float scale = AccuracyGraphUtils::Lerp(defaultScale, hoverScale, progress);
        target->set_localScale(UnityEngine::Vector3(scale, scale, scale));
    };
    return component;
}

void BeatLeader::SmoothHoverController::OnDisable() {
    IsHovered = false;
    Progress = 0.0f;
    _targetValue = 0.0f;
    _set = false;

    hoverStateChangedEvent.invoke(IsHovered, Progress);
}

void BeatLeader::SmoothHoverController::Update() {
    if (_set) return;
            
    if (abs(_targetValue - Progress) < 1e-6) {
        Progress = _targetValue;
        _set = true;
    } else {
        Progress = AccuracyGraphUtils::Lerp(Progress, _targetValue, Time::get_deltaTime() * lerpCoefficient);
    }
    hoverStateChangedEvent.invoke(IsHovered, Progress);
}

void BeatLeader::SmoothHoverController::OnPointerEnter(EventSystems::PointerEventData* eventData) {
    IsHovered = true;
    _targetValue = 1.0f;
    _set = false;
}

void BeatLeader::SmoothHoverController::OnPointerExit(EventSystems::PointerEventData* eventData) {
    IsHovered = false;
    _targetValue = 0.0f;
    _set = false;
}

BeatLeader::SmoothHoverController::HoverStateChangedEvent& BeatLeader::SmoothHoverController::get_hoverStateChangedEvent() {
    return hoverStateChangedEvent;
}