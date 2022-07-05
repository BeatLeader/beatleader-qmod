#pragma once

namespace BeatLeader {
    class Range {
        public:
        float start;
        float end;
        
        Range(float start, float end);

        float GetRatioClamped(float value);
        float GetValueClamped(float ratio);
        float GetRatio(float value);
        float SlideBy(float ratio);

        private:
        float amplitude;
    };
}