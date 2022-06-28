#pragma once
#include <vector>

using namespace std;

namespace BeatLeader {
    template <typename T>
    class CyclicBuffer {
        public:
        int size = 0;

        CyclicBuffer(int size) noexcept;
        bool Add(T point);
        vector<T> GetBuffer();

        private:
        int capacity = 0;
        int nextElementIndex = 0;
        int lastIndex = 0;

        vector<T> nodes;
        vector<T> buffer;
    };
}

template <typename T>
BeatLeader::CyclicBuffer<T>::CyclicBuffer(int size) noexcept : nodes(size), buffer(size) {
    this->capacity = size;
}

template <typename T>
bool BeatLeader::CyclicBuffer<T>::Add(T point) {
    lastIndex = nextElementIndex;
    nodes[lastIndex] = point;

    nextElementIndex -= 1;
    if (nextElementIndex < 0) nextElementIndex = capacity - 1;

    if (size >= capacity) return true;
    size += 1;
    return false;
}

template <typename T>
vector<T> BeatLeader::CyclicBuffer<T>::GetBuffer() {
    int bufferIndex = 0;

    for (int i = lastIndex; i < capacity; i++) {
        buffer[bufferIndex++] = nodes[i];
    }

    for (int i = 0; i < lastIndex; i++) {
        buffer[bufferIndex++] = nodes[i];
    }

    return buffer;
}