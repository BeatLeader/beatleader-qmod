#include "UI/Christmas/ChristmasTreeAnimator.hpp"
#include "UnityEngine/Transform.hpp"

DEFINE_TYPE(BeatLeader, ChristmasTreeAnimator);

namespace BeatLeader {
    void ChristmasTreeAnimator::ctor() {
        _scaleAnimator = VectorAnimator::New_ctor();
        _positionAnimator = VectorAnimator::New_ctor();
        _rotationAnimator = VectorAnimator::New_ctor();
    }

    void ChristmasTreeAnimator::EvaluateScaleImmediate() {
        _scaleAnimator->EvaluateImmediate();
    }

    void ChristmasTreeAnimator::EvaluatePosImmediate() {
        _positionAnimator->EvaluateImmediate();
    }

    void ChristmasTreeAnimator::Update() {
        _scaleAnimator->Update();
        _positionAnimator->Update();
        _rotationAnimator->Update();

        if (!_scaleAnimator->get_Set()) {
            get_transform()->set_localScale(_scaleAnimator->get_Value());
        }
        if (!_rotationAnimator->get_Set()) {
            get_transform()->set_localEulerAngles(_rotationAnimator->get_Value());
        }
        if (!_positionAnimator->get_Set()) {
            get_transform()->set_position(_positionAnimator->get_Value());
        }

        _scaleAnimator->LateUpdate();
        _positionAnimator->LateUpdate();
        _rotationAnimator->LateUpdate();
    }

    void ChristmasTreeAnimator::Awake() {

        _scaleAnimator = VectorAnimator::New_ctor();
        _positionAnimator = VectorAnimator::New_ctor();
        _rotationAnimator = VectorAnimator::New_ctor();
        
        _animationSpeed = 10.0f;
        _scaleAnimator->set_LerpCoefficient(_animationSpeed);
        _positionAnimator->set_LerpCoefficient(_animationSpeed);
        _rotationAnimator->set_LerpCoefficient(_animationSpeed);

        get_transform()->set_localScale(UnityEngine::Vector3::get_zero());
        get_transform()->set_localEulerAngles(UnityEngine::Vector3(0.0f, -180.0f, 0.0f));
    }
} 