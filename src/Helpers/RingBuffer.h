// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <vector>

#include "DsVeosCoSim/CoSimTypes.h"

namespace DsVeosCoSim {

template <typename T>
class RingBuffer final {
public:
    RingBuffer() = default;
    explicit RingBuffer(size_t capacity) {
        _items.resize(capacity);
    }

    ~RingBuffer() = default;

    RingBuffer(const RingBuffer&) = delete;
    RingBuffer& operator=(const RingBuffer&) = delete;

    RingBuffer(RingBuffer&& other) = default;
    RingBuffer& operator=(RingBuffer&& other) = default;

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

    [[nodiscard]] Result PushBack(const T& item) {
        if (IsFull()) {
            LogError("Ring buffer is full.");
            return Result::Error;
        }

        size_t currentWriteIndex = _writeIndex;
        _items[currentWriteIndex] = item;
        _writeIndex = (_writeIndex + 1) % _items.size();
        ++_size;
        return Result::Ok;
    }

    [[nodiscard]] Result PushBack(T&& item) {
        if (IsFull()) {
            LogError("Ring buffer is full.");
            return Result::Error;
        }

        size_t currentWriteIndex = _writeIndex;
        _items[currentWriteIndex] = std::move(item);
        _writeIndex = (_writeIndex + 1) % _items.size();
        ++_size;
        return Result::Ok;
    }

    [[nodiscard]] T* EmplaceBack() {
        if (IsFull()) {
            LogError("Ring buffer is full.");
            return nullptr;
        }

        size_t currentWriteIndex = _writeIndex;
        T* item = &_items[currentWriteIndex];
        _writeIndex = (_writeIndex + 1) % _items.size();
        ++_size;
        return item;
    }

    [[nodiscard]] T* PopFront() {
        if (IsEmpty()) {
            LogError("Ring buffer is empty.");
            return nullptr;
        }

        --_size;
        T* item = &_items[_readIndex];
        _readIndex = (_readIndex + 1) % _items.size();
        return item;
    }

private:
    size_t _readIndex{};
    size_t _writeIndex{};
    size_t _size{};
    std::vector<T> _items{};
};

}  // namespace DsVeosCoSim
