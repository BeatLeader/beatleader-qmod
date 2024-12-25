#include "UI/Christmas/ValueAnimator.hpp"

DEFINE_TYPE(BeatLeader, ValueAnimator);

namespace BeatLeader {
    void ValueAnimator::ctor() {
        value = 0.0f;
        lerpCoefficient = 10.0f;
        targetValue = 0.0f;
        set = true;
    }

    void ValueAnimator::SetTarget(float targetVal) {
        targetValue = targetVal;
        set = false;
    }

    void ValueAnimator::Update() {
        if (set) return;

        if (std::abs(targetValue - value) < 0.001f) {
            value = targetValue;
            set = true;
        } else {
            value = UnityEngine::Mathf::Lerp(value, targetValue, UnityEngine::Time::get_deltaTime() * lerpCoefficient);
        }
    }
} 