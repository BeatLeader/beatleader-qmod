#pragma once

#include <limits>
#include <string>
#include <concepts>
#include <cmath>
#include <algorithm>
#include "Concepts.hpp"

#ifdef SOMBRERO_DEBUG

#ifdef HAS_CODEGEN
#warning CODEGEN DETECTED
#else
#warning NO CODEGEN DETECTED
#include "beatsaber-hook/shared/types.hpp"
#endif

#endif

namespace Sombrero {
    
    constexpr float Clamp01(float value)
    {
        return value < 0.0f ? 0.0f : (value < 1.0f ? value : 1.0f);
    }

    template<Sombrero::Lerpable T>
    constexpr T Lerp(T a, T b, float t)
    {
        return T::Lerp(a, b, Clamp01(t));
    }

    template<Sombrero::Slerpable T>
    constexpr T Slerp(T a, T b, float t)
    {
        return T::Slerp(a, b, Clamp01(t));
    }
    
    inline static float PingPong(float t, float length)
    {
        float graph = std::fmod(t, length * 2);
        if (graph > length) graph = ((2 * length) - graph);
        // should yield
        //length_______________
        //                 /\      /\      /
        //                /  \    /  \    /
        //               /    \  /    \  /
        //zero_________ /      \/      \/
        return graph;
    }

    // Credit to sc2ad for making the sqrt and pow constexpr
    namespace detail {
        constexpr float sqrt(float x, float curr, float prev) {
            return curr == prev
                ? curr
                : sqrt(x, 0.5 * (curr + x / curr), curr);
        }
    }

    constexpr float power(float x, int p) 
    {
        if (p == 0) {
            return 1;
        } else if (p == 1) {
            return x;
        }
        // if p > 0, return x * pow x, p
        // else divide x by pow x, -p
        return p > 0 ? x * power(x, p - 1) : 1 / power(x, -p);
    }

    constexpr float sqroot(float x) 
    {
        return x >= 0 && x < std::numeric_limits<float>::infinity()
            ? detail::sqrt(x, x, 0)
            : std::numeric_limits<float>::quiet_NaN();
    }

    // Convert degrees to radians
    constexpr double degreesToRadians(double degrees) {
      return degrees * M_PI / 180.0;
    }
}
