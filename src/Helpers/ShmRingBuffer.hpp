// Copyright dSPACE SE & Co. KG. All rights reserved.

#pragma once

#ifdef _WIN32

#include <atomic>
#include <stdexcept>

namespace DsVeosCoSim {

template <typename T>
class ShmRingBuffer final {
public:
    ShmRingBuffer() = default;
    ~ShmRingBuffer() noexcept = default;

    ShmRingBuffer(const ShmRingBuffer&) = delete;
    ShmRingBuffer& operator=(const ShmRingBuffer&) = delete;

    ShmRingBuffer(ShmRingBuffer&&) = delete;
    ShmRingBuffer& operator=(ShmRingBuffer&&) = delete;

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

    void PushBack(const T& item) {
        if (IsFull()) {
            throw std::runtime_error("SHM ring buffer is full.");
        }

        uint32_t currentWriteIndex = _writeIndex;
        _items[currentWriteIndex] = item;
        _writeIndex = (_writeIndex + 1) % _capacity;
        ++_size;
    }

    void PushBack(T&& item) {
        if (IsFull()) {
            throw std::runtime_error("SHM ring buffer is full.");
        }

        uint32_t currentWriteIndex = _writeIndex;
        _items[currentWriteIndex] = std::move(item);
        _writeIndex = (_writeIndex + 1) % _capacity;
        ++_size;
    }

    [[nodiscard]] T& EmplaceBack() {
        if (IsFull()) {
            throw std::runtime_error("SHM Ring buffer is full.");
        }

        size_t currentWriteIndex = _writeIndex;
        T& item = _items[currentWriteIndex];
        _writeIndex = (_writeIndex + 1) % _capacity;
        ++_size;
        return item;
    }

    [[nodiscard]] T& PopFront() {
        if (IsEmpty()) {
            throw std::runtime_error("SHM ring buffer is empty.");
        }

        --_size;
        T& item = _items[_readIndex];
        _readIndex = (_readIndex + 1) % _capacity;
        return item;
    }

private:
    uint32_t _capacity{};           // Read by reader and writer
    std::atomic<uint32_t> _size{};  // Read and written by reader and writer
    uint32_t _readIndex{};          // Read and written by reader
    uint32_t _writeIndex{};         // Read and written by writer

    // Zero sized array would be correct here, since the items are inside a shared memory. But that leads to
    // warnings, so we set the size to 1
    T _items[1]{};
};

}  // namespace DsVeosCoSim

#endif
