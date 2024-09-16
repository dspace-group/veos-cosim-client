// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <atomic>
#include <cstdint>
#include <stdexcept>

namespace DsVeosCoSim {

template <typename T>
class ShmRingBuffer final {
public:
    ShmRingBuffer() = default;
    ~ShmRingBuffer() noexcept = default;

    ShmRingBuffer(const ShmRingBuffer&) = delete;
    ShmRingBuffer& operator=(const ShmRingBuffer&) = delete;

    ShmRingBuffer(ShmRingBuffer&& other) = delete;
    ShmRingBuffer& operator=(ShmRingBuffer&& other) = delete;

    void Initialize(uint32_t capacity) {
        _capacity = capacity;
    }

    void Clear() {
        _readIndex = 0;
        _writeIndex = 0;
        _size = 0;
    }

    [[nodiscard]] uint32_t Size() const {
        return _size;
    }

    [[nodiscard]] bool IsEmpty() const {
        return _size == 0;
    }

    [[nodiscard]] bool IsFull() const {
        return _size == _capacity;
    }

    void PushBack(const T& element) {
        if (IsFull()) {
            throw std::runtime_error("SHM ring buffer is full.");
        }

        uint32_t currentWriteIndex = _writeIndex;
        _items[currentWriteIndex] = element;

        ++_writeIndex;
        if (_writeIndex == _capacity) {
            _writeIndex = 0;
        }

        _size++;
    }

    [[nodiscard]] T& PopFront() {
        if (IsEmpty()) {
            throw std::runtime_error("SHM ring buffer is empty.");
        }

        _size--;

        T& item = _items[_readIndex];

        ++_readIndex;
        if (_readIndex == _capacity) {
            _readIndex = 0;
        }

        return item;
    }

private:
    uint32_t _capacity{};         // Read by reader and writer
    std::atomic<uint32_t> _size;  // Read and written by reader and writer
    uint32_t _readIndex{};        // Read and written by reader
    uint32_t _writeIndex{};       // Read and written by writer

    // Zero sized array would be correct here, since the items are inside of a shared memory. But that leads to
    // warnings, so we add an additional element here
    T _items[1]{};
};

}  // namespace DsVeosCoSim
