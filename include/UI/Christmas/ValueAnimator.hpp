#pragma once

#include "custom-types/shared/macros.hpp"
#include "UnityEngine/MonoBehaviour.hpp"
#include "UnityEngine/Time.hpp"
#include "UnityEngine/Mathf.hpp"
#include <cmath>

DECLARE_CLASS_CODEGEN(BeatLeader, ValueAnimator, UnityEngine::MonoBehaviour,
    DECLARE_INSTANCE_FIELD(float, value);
    DECLARE_INSTANCE_FIELD(float, lerpCoefficient);
    DECLARE_INSTANCE_FIELD(float, targetValue);
    DECLARE_INSTANCE_FIELD(bool, set);

    public:
        float get_Value() const { return value; }
        void set_Value(float val) { value = val; }
        
        float get_LerpCoefficient() const { return lerpCoefficient; }
        void set_LerpCoefficient(float coeff) { lerpCoefficient = coeff; }

    DECLARE_INSTANCE_METHOD(void, SetTarget, float targetVal);
    DECLARE_INSTANCE_METHOD(void, Update);
    DECLARE_CTOR(ctor);
) 