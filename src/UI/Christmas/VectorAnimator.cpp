#include "UI/Christmas/VectorAnimator.hpp"

DEFINE_TYPE(BeatLeader, VectorAnimator);

namespace BeatLeader {
    void VectorAnimator::ctor() {
        lerpCoefficient = 10.0f;
        set = true;
        shouldBeSet = true;
    }

    void VectorAnimator::SetTarget(UnityEngine::Vector3 targetVal) {
        targetValue = targetVal;
        set = false;
        shouldBeSet = false;
    }

    void VectorAnimator::EvaluateImmediate() {
        value = targetValue;
    }

    void VectorAnimator::Update() {
        if (set) return;

        if (std::abs(targetValue.x - value.x) < 0.001f) {
            value = targetValue;
            shouldBeSet = true;
        } else {
            value = UnityEngine::Vector3::Lerp(value, targetValue, UnityEngine::Time::get_deltaTime() * lerpCoefficient);
        }
    }

    void VectorAnimator::LateUpdate() {
        set = shouldBeSet;
    }
} 