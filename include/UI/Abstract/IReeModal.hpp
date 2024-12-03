#pragma once

#include "custom-types/shared/macros.hpp"
#include "System/Action.hpp"

namespace BeatLeader {
    class IReeModal {
        public:
            virtual ~IReeModal() = default;
            virtual void Resume(void* state, System::Action* closeAction) = 0;
            virtual void Pause() = 0;
            virtual void Interrupt() = 0;
            virtual void Close() = 0;
            virtual void HandleOffClick() = 0;
    };
} 