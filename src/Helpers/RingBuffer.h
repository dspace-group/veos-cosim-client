// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <stdexcept>
#include <vector>

namespace DsVeosCoSim {

template <typename T>
class RingBuffer final {
public:
    RingBuffer() = default;
    explicit RingBuffer(size_t capacity) {
        _items.resize(capacity);
    }

    ~RingBuffer() noexcept = default;

    RingBuffer(const RingBuffer&) = delete;
    RingBuffer& operator=(const RingBuffer&) = delete;

    RingBuffer(RingBuffer&& other) noexcept = default;
    RingBuffer& operator=(RingBuffer&& other) noexcept = default;

    void Clear() {
        _readIndex = 0;
        _writeIndex = 0;
        _size = 0;
    }

    [[nodiscard]] size_t Size() const {
        return _size;
    }

    [[nodiscard]] bool IsEmpty() const {
        return _size == 0;
    }

    [[nodiscard]] bool IsFull() const {
        return _size == _items.size();
    }

    void PushBack(const T& element) {
        if (IsFull()) {
            throw std::runtime_error("Ring buffer is full.");
        }

        size_t currentWriteIndex = _writeIndex;
        _items[currentWriteIndex] = element;

        ++_writeIndex;
        if (_writeIndex == _items.size()) {
            _writeIndex = 0;
        }

        ++_size;
    }

    [[nodiscard]] T& PopFront() {
        if (IsEmpty()) {
            throw std::runtime_error("Ring buffer is empty.");
        }

        --_size;

        T& item = _items[_readIndex];

        ++_readIndex;
        if (_readIndex == _items.size()) {
            _readIndex = 0;
        }

        return item;
    }

private:
    size_t _readIndex{};
    size_t _writeIndex{};
    size_t _size{};
    std::vector<T> _items{};
};

}  // namespace DsVeosCoSim
