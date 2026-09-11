#pragma once
#include <concepts>

namespace Sombrero
{
    template<typename T, typename U>
    concept SomberoConvertible = std::is_convertible_v<T, U>;

#ifdef USE_SOMBRERO_IMPLICIT_CONVERSIONS
    template<typename T>
    concept Has4D = requires (T a)
    { 
        { a.x } -> SomberoConvertible<float>;
        { a.y } -> SomberoConvertible<float>;
        { a.z } -> SomberoConvertible<float>;
        { a.w } -> SomberoConvertible<float>;
    };

    template<typename T>
    concept Has3D = !Has4D<T> && requires (T a)
    { 
        { a.x } -> SomberoConvertible<float>;
        { a.y } -> SomberoConvertible<float>;
        { a.z } -> SomberoConvertible<float>;
    };

    template<typename T>
    concept Has2D = !Has3D<T> && !Has4D<T> && requires (T a)
    { 
        { a.x } -> SomberoConvertible<float>;
        { a.y } -> SomberoConvertible<float>;
    };

    template<typename T>
    concept HasRGBA = requires (T c)
    {
        { c.r } -> SomberoConvertible<float>;
        { c.g } -> SomberoConvertible<float>;
        { c.b } -> SomberoConvertible<float>;
        { c.a } -> SomberoConvertible<float>;
    };

    template<typename T>
    concept HasRGB = !HasRGBA<T> && requires (T c)
    {
        { c.r } -> SomberoConvertible<float>;
        { c.g } -> SomberoConvertible<float>;
        { c.b } -> SomberoConvertible<float>;
    };
#endif
    template<typename T>
    concept Lerpable = requires (T a, T b)
    {
        { T::Lerp(a, b, 1.0f) } -> SomberoConvertible<T>;
    };

    template<typename T>
    concept Slerpable = requires (T a, T b)
    {
        { T::Slerp(a, b, 1.0f) } -> SomberoConvertible<T>;
    };

    template<typename T>
    concept Float = requires (T a) { { a } -> SomberoConvertible<float>; };
}