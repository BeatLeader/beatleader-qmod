#pragma once

#include "include/Models/Replay.hpp"

enum MapStatus {
    cleared = 0,
    failed = 1
};

namespace ReplayRecorder {
    void StartRecording(std::function<void(Replay*, MapStatus, bool)> callback);
}