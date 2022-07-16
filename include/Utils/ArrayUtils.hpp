#pragma once

#include "beatsaber-hook/shared/utils/typedefs-array.hpp"

#include <span>

namespace BeatLeader {
    /// @brief Converts a vector to an Array*
    /// @tparam T Inner type of the vector and array
    /// @param vec Vector to create the Array from
    /// @return The created Array<T>*
    template<typename T>
    ArrayW<T> toArray(::std::span<T> vec) {
        ArrayW<T> arr(vec.size());

        std::copy(vec.begin(), vec.end(), arr.begin());
        return arr;
    }
}