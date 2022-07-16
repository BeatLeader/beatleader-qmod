#include "include/Utils/Range.hpp"

using namespace BeatLeader;

Range::Range(float start, float end) {
    this->start = start;
    this->end = end;
    this->amplitude = end - start;
}

float Range::GetRatioClamped(float value) const {
    float ratio = GetRatio(value);
    if (ratio <= 0) return 0;
    if (ratio >= 1) return 1;
    return ratio;
}

float Range::GetValueClamped(float ratio) const {
    if (ratio <= 0) return start;
    if (ratio >= 1) return end;
    return SlideBy(ratio);
}

float Range::GetRatio(float value) const {
    return (value - start) / amplitude;
}

float Range::SlideBy(float ratio) const {
    return start + amplitude * ratio;
}