// Copyright dSPACE SE & Co. KG. All rights reserved.

#pragma once

#ifdef _WIN32

#include <cstdint>
#include <type_traits>

namespace DsVeosCoSim {

// This is view to a ring buffer, that is located in a shared memory.
// It does not provide real thread safety because that is enforced by the bus buffer.
template <typename T>
class RingBufferView final {
public:
    static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable.");

    RingBufferView() = default;
    ~RingBufferView() noexcept = default;

    RingBufferView(const RingBufferView&) = delete;
    RingBufferView& operator=(const RingBufferView&) = delete;

    RingBufferView(RingBufferView&&) = delete;
    RingBufferView& operator=(RingBufferView&&) = delete;

    void Initialize(uint32_t capacity) noexcept {
        _capacity = capacity;
    }

    void Clear() noexcept {
        _readIndex = 0;
        _writeIndex = 0;
    }

    void PushBack(const T& item) noexcept {
        _items[_writeIndex] = item;
        AdvanceIndex(_writeIndex);
    }

    void PushBack(T&& item) noexcept {
        _items[_writeIndex] = std::move(item);
        AdvanceIndex(_writeIndex);
    }

    [[nodiscard]] T& PopFront() noexcept {
        T& item = _items[_readIndex];
        AdvanceIndex(_readIndex);
        return item;
    }

private:
    void AdvanceIndex(uint32_t& index) noexcept {
        index++;
        if (index == _capacity) {
            index = 0;
        }
    }

    uint32_t _capacity{};
    uint32_t _readIndex{};
    uint32_t _writeIndex{};

    // Zero sized array would be correct here, since the items are inside a shared memory. But that leads to
    // warnings, so we set the size to 1
    T _items[1]{};
};

}  // namespace DsVeosCoSim

#endif
