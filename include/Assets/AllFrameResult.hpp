#pragma once

#include "UnityEngine/Texture2D.hpp"

struct AllFramesResult {
    Array<UnityEngine::Texture2D*>* frames;
    Array<float>* timings;
};