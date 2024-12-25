#pragma once

#include "custom-types/shared/macros.hpp"
#include "UnityEngine/MonoBehaviour.hpp"
#include "UnityEngine/Vector3.hpp"
#include "VectorAnimator.hpp"

DECLARE_CLASS_CODEGEN(BeatLeader, ChristmasTreeAnimator, UnityEngine::MonoBehaviour,
    DECLARE_INSTANCE_FIELD(float, _animationSpeed);
    DECLARE_INSTANCE_FIELD(VectorAnimator*, _scaleAnimator);
    DECLARE_INSTANCE_FIELD(VectorAnimator*, _positionAnimator);
    DECLARE_INSTANCE_FIELD(VectorAnimator*, _rotationAnimator);

    public:
        void set_TargetPosition(UnityEngine::Vector3 value) {
            _positionAnimator->set_Value(get_transform()->get_position());
            _positionAnimator->SetTarget(value);
        }

        void set_TargetRotation(float value) {
            _rotationAnimator->set_Value(get_transform()->get_localEulerAngles());
            _rotationAnimator->SetTarget(UnityEngine::Vector3::op_Multiply(value, UnityEngine::Vector3::get_one()));
        }

        void set_TargetScale(float value) {
            _scaleAnimator->set_Value(get_transform()->get_localScale());
            _scaleAnimator->SetTarget(UnityEngine::Vector3::op_Multiply(value, UnityEngine::Vector3::get_one()));
        }

    DECLARE_INSTANCE_METHOD(void, EvaluateScaleImmediate);
    DECLARE_INSTANCE_METHOD(void, EvaluatePosImmediate);
    DECLARE_INSTANCE_METHOD(void, Update);
    DECLARE_INSTANCE_METHOD(void, Awake);

    DECLARE_CTOR(ctor);
) 