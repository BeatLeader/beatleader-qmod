#pragma once

#include "beatsaber-hook/shared/arrayw.hpp"

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

    /// @brief Returns the first element matching the predicate, or a default-constructed T if none match
    template<typename T, typename Pred>
    T FirstOrDefault(ArrayW<T> arr, Pred&& pred) {
        if (!arr) return T();
        for (auto item : arr) {
            if (pred(item)) return item;
        }
        return T();
    }

    /// @brief Returns the last element matching the predicate, or a default-constructed T if none match
    template<typename T, typename Pred>
    T LastOrDefault(ArrayW<T> arr, Pred&& pred) {
        if (!arr) return T();
        for (auto i = arr.size(); i > 0; i--) {
            if (pred(arr[i - 1])) return arr[i - 1];
        }
        return T();
    }
}
