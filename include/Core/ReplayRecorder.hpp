#pragma once

#include "include/Models/Replay.hpp"

enum MapStatus {
    cleared = 0,
    failed = 1
};

namespace ReplayRecorder {
    void StartRecording(function<void(Replay const &, MapStatus, bool)> const &callback);
    static bool recording;
}