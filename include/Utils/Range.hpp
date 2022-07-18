#pragma once

namespace BeatLeader {
    class Range {
        public:
        float start;
        float end;
        
        Range(float start, float end);

        float GetRatioClamped(float value) const;
        float GetValueClamped(float ratio) const;
        float GetRatio(float value) const;
        float SlideBy(float ratio) const;

        private:
        float amplitude;
    };
}