#include "UI/Christmas/SnowController.hpp"

DEFINE_TYPE(BeatLeader, SnowController);

namespace BeatLeader {

    void SnowController::OnTransformParentChanged() {
        auto emission = _particleSystem->get_emission();
        if (!emission.get_enabled()) {
            return;
        }
        Play(true);
    }

    void SnowController::Play(bool immediately) {
        auto emission = _particleSystem->get_emission();
        emission.set_enabled(true);

        if (immediately) {
            auto main = _particleSystem->get_main();
            float duration = main.get_startLifetimeMultiplier();
            _particleSystem->Simulate(duration, true, true, true);
        }
        _particleSystem->Play();
    }

    void SnowController::Stop() {
        auto emission = _particleSystem->get_emission();
        emission.set_enabled(false);
        _particleSystem->Stop();
        _particleSystem->Clear();
    }
} 