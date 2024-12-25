#pragma once

#include "custom-types/shared/macros.hpp"
#include "UnityEngine/MonoBehaviour.hpp"
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Time.hpp"
#include <cmath>

DECLARE_CLASS_CODEGEN(BeatLeader, VectorAnimator, UnityEngine::MonoBehaviour,
    DECLARE_INSTANCE_FIELD(UnityEngine::Vector3, value);
    DECLARE_INSTANCE_FIELD(float, lerpCoefficient);
    DECLARE_INSTANCE_FIELD(UnityEngine::Vector3, targetValue);
    DECLARE_INSTANCE_FIELD(bool, set);
    DECLARE_INSTANCE_FIELD(bool, shouldBeSet);

    public:
        UnityEngine::Vector3 get_Value() const { return value; }
        void set_Value(UnityEngine::Vector3 val) { value = val; }
        
        float get_LerpCoefficient() const { return lerpCoefficient; }
        void set_LerpCoefficient(float coeff) { lerpCoefficient = coeff; }
        
        bool get_Set() const { return set; }

    DECLARE_INSTANCE_METHOD(void, SetTarget, UnityEngine::Vector3 targetVal);
    DECLARE_INSTANCE_METHOD(void, EvaluateImmediate);
    DECLARE_INSTANCE_METHOD(void, Update);
    DECLARE_INSTANCE_METHOD(void, LateUpdate);
    DECLARE_CTOR(ctor);
) 