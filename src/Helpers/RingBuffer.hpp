// Copyright dSPACE SE & Co. KG. All rights reserved.

#pragma once

#include <type_traits>
#include <utility>
#include <vector>

namespace DsVeosCoSim {

// Not thread safe
template <typename T>
class RingBuffer final {
public:
    static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable.");

    RingBuffer() = default;

    // No guard for zero capacity, but that's ok
    explicit RingBuffer(size_t capacity) : _capacity(capacity) {
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

    [[nodiscard]] size_t Size() const noexcept {
        return _size;
    }

    [[nodiscard]] bool IsEmpty() const noexcept {
        return _size == 0;
    }

    [[nodiscard]] bool IsFull() const noexcept {
        return _size == _capacity;
    }

    [[nodiscard]] bool TryPushBack(const T& item) noexcept {
        if (IsFull()) {
            return false;
        }

        _items[_writeIndex] = item;
        AdvanceIndex(_writeIndex);

        ++_size;
        return true;
    }

    [[nodiscard]] bool TryPushBack(T&& item) noexcept {
        if (IsFull()) {
            return false;
        }

        _items[_writeIndex] = std::move(item);
        AdvanceIndex(_writeIndex);

        ++_size;
        return true;
    }

    [[nodiscard]] bool TryPopFront(T& item) noexcept {
        if (IsEmpty()) {
            return false;
        }

        item = std::move(_items[_readIndex]);
        AdvanceIndex(_readIndex);

        --_size;
        return true;
    }

    [[nodiscard]] T* TryPeekFront() noexcept {
        if (IsEmpty()) {
            return nullptr;
        }

        return &_items[_readIndex];
    }

    [[nodiscard]] T* TryPeekFront() const noexcept {
        if (IsEmpty()) {
            return nullptr;
        }

        return &_items[_readIndex];
    }

    void RemoveFront() noexcept {
        if (IsEmpty()) {
            return;
        }

        AdvanceIndex(_readIndex);
        --_size;
    }

private:
    void AdvanceIndex(size_t& index) noexcept {
        index++;
        if (index == _capacity) {
            index = 0;
        }
    }

    size_t _capacity{};
    size_t _size{};
    size_t _readIndex{};
    size_t _writeIndex{};

    std::vector<T> _items{};
};

}  // namespace DsVeosCoSim
