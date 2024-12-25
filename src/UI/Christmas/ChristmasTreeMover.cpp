#include "UI/Christmas/ChristmasTreeMover.hpp"
#include "UnityEngine/Time.hpp"
#include "UnityEngine/Shader.hpp"
#include "VRUIControls/VRInputModule.hpp"
#include "VRUIControls/VRPointer.hpp"
#include "UI/Utils/ReeTransform.hpp"

DEFINE_TYPE(BeatLeader, ChristmasTreeMover);

namespace BeatLeader {
    void ChristmasTreeMover::ctor() {
        _highlightAnimator = ValueAnimator::New_ctor();
        _scaleAnimator = ValueAnimator::New_ctor();
    }
    
    void ChristmasTreeMover::Awake() {
        _highlightAnimator = ValueAnimator::New_ctor();
        _scaleAnimator = ValueAnimator::New_ctor();
    }

    void ChristmasTreeMover::SetEnabled(bool full, bool restricted) {
        this->full = full;
        this->restricted = restricted;
        _scaleAnimator->SetTarget(full || restricted ? 1.0f : 0.0f);
    }

    void ChristmasTreeMover::RefreshHighlight() {
        _highlightAnimator->SetTarget(hovered ? (grabbed ? 0.8f : 0.3f) : 0.0f);
    }

    void ChristmasTreeMover::RefreshColor() {
        _highlightAnimator->Update();
        auto color = UnityEngine::Color::Lerp(
            UnityEngine::Color::get_white(),
            UnityEngine::Color::get_red(),
            _highlightAnimator->get_Value()
        );
        _material->SetColor("_TextureTint", color);
    }

    void ChristmasTreeMover::RefreshScale() {
        _scaleAnimator->Update();
        get_transform()->set_localScale(UnityEngine::Vector3::op_Multiply(_scaleAnimator->get_Value(), UnityEngine::Vector3::get_one()));
    }

    float SignedAngle(UnityEngine::Vector3 from, UnityEngine::Vector3 to, UnityEngine::Vector3 axis) {
        float unsignedAngle = UnityEngine::Vector3::Angle(from, to);

        float cross_x = from.y * to.z - from.z * to.y;
        float cross_y = from.z * to.x - from.x * to.z;
        float cross_z = from.x * to.y - from.y * to.x;
        float sign = UnityEngine::Mathf::Sign(axis.x * cross_x + axis.y * cross_y + axis.z * cross_z);
        return unsignedAngle * sign;
    }

    void ChristmasTreeMover::Update() {
        RefreshColor();
        RefreshScale();

        if (!grabbed) return;

        auto controllerPose = ReeTransform::FromTransform(grabbingController);
        
        auto targetWorldPose = ReeTransform(
            controllerPose.LocalToWorldPosition(attachmentLocalPose.Position),
            controllerPose.LocalToWorldRotation(attachmentLocalPose.Rotation)
        );

        float t = UnityEngine::Time::get_unscaledDeltaTime() * 10.0f;

        // Rotation with X axis
        auto stickVec = UnityEngine::Vector2::op_Multiply(grabbingVRController->thumbstick, t);
        rotOffset -= stickVec.x * 20.0f;
        
        auto currentLocalUp = controllerPose.WorldToLocalDirection(UnityEngine::Vector3::get_up());
        currentLocalUp.z = 0;
        
        auto currentWorldForward = controllerPose.LocalToWorldDirection(UnityEngine::Vector3::get_forward());
        currentWorldForward.y = 0;
        
        float doorknobAngle = SignedAngle(
            attachmentLocalUp,
            currentLocalUp,
            UnityEngine::Vector3::get_forward()
        );
        float forwardAngle = SignedAngle(
            attachmentWorldForward,
            currentWorldForward,
            UnityEngine::Vector3::get_up()
        );
        
        auto rot = UnityEngine::Quaternion::op_Multiply(
            UnityEngine::Quaternion::AngleAxis(rotOffset + forwardAngle - doorknobAngle, UnityEngine::Vector3::get_up()),
            grabRotation
        );

        // Scale
        auto scale = grabScale;
        scale = UnityEngine::Vector3::op_Multiply(
            scale,
            targetWorldPose.Position.y / grabWorldPose.Position.y
        );
        float newScale = UnityEngine::Mathf::Clamp(scale.y, 0.3f, 2.0f);
        scale = UnityEngine::Vector3::op_Multiply(newScale, UnityEngine::Vector3::get_one());

        // Position
        auto pos = targetWorldPose.Position;
        pos.y = 0.0f;

        if (full) {
            _container->set_position(UnityEngine::Vector3::Lerp(
                _container->get_position(),
                pos,
                t
            ));
        }

        if (full || restricted) {
            _container->set_localScale(UnityEngine::Vector3::Lerp(
                _container->get_localScale(),
                scale,
                t
            ));
            _container->set_rotation(UnityEngine::Quaternion::Lerp(
                _container->get_rotation(),
                rot,
                t
            ));
        }
    }

    void ChristmasTreeMover::OnPointerDown(UnityEngine::EventSystems::PointerEventData* eventData) {
        if (!hovered) return;
        
        UnityEngine::EventSystems::BaseInputModule* baseModule = eventData->get_currentInputModule();
        auto module = reinterpret_cast<VRUIControls::VRInputModule*>(baseModule);
        if (!module) return;

        grabbingVRController = module->_vrPointer->lastSelectedVrController;
        grabbingController = grabbingVRController->get_transform();

        auto controllerPose = ReeTransform::FromTransform(grabbingController);

        attachmentLocalPose = ReeTransform(
            controllerPose.WorldToLocalPosition(get_transform()->get_position()),
            controllerPose.WorldToLocalRotation(get_transform()->get_rotation())
        );

        attachmentLocalUp = controllerPose.WorldToLocalDirection(UnityEngine::Vector3::get_up());
        attachmentLocalUp.z = 0;

        attachmentWorldForward = controllerPose.LocalToWorldDirection(UnityEngine::Vector3::get_forward());
        attachmentWorldForward.y = 0;

        grabWorldPose = ReeTransform(
            get_transform()->get_position(),
            get_transform()->get_rotation()
        );

        rotOffset = 0.0f;
        grabScale = _container->get_localScale();
        grabRotation = _container->get_rotation();

        grabbed = true;
        RefreshHighlight();
    }

    void ChristmasTreeMover::OnPointerUp(UnityEngine::EventSystems::PointerEventData* eventData) {
        if (!grabbed) return;
        
        grabbingController = nullptr;
        grabbingVRController = nullptr;
        grabbed = false;
        RefreshHighlight();
    }

    void ChristmasTreeMover::OnPointerEnter(UnityEngine::EventSystems::PointerEventData* eventData) {
        hovered = true;
        RefreshHighlight();
    }

    void ChristmasTreeMover::OnPointerExit(UnityEngine::EventSystems::PointerEventData* eventData) {
        hovered = false;
        RefreshHighlight();
    }
} 