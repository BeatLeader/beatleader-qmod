#pragma once

#include "UnityEngine/Texture2D.hpp"

struct AllFramesResult {
    ArrayW<UnityEngine::Texture2D*>  frames;
    ArrayW<float> timings;
};