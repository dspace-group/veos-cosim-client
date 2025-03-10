// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#ifdef _WIN32

#include <cstddef>
#include <optional>
#include <string>

#include "Handle.h"

namespace DsVeosCoSim {

class SharedMemory final {
    SharedMemory(const std::string& name, size_t size, Handle handle);

public:
    SharedMemory() = default;
    ~SharedMemory() noexcept = default;

    SharedMemory(const SharedMemory&) = delete;
    SharedMemory& operator=(const SharedMemory&) = delete;

    SharedMemory(SharedMemory&&) noexcept;
    SharedMemory& operator=(SharedMemory&&) noexcept;

    [[nodiscard]] static SharedMemory CreateOrOpen(const std::string& name, size_t size);
    [[nodiscard]] static SharedMemory OpenExisting(const std::string& name, size_t size);
    [[nodiscard]] static std::optional<SharedMemory> TryOpenExisting(const std::string& name, size_t size);

    [[nodiscard]] void* data() const noexcept;   // NOLINT
    [[nodiscard]] size_t size() const noexcept;  // NOLINT

private:
    size_t _size{};
    Handle _handle;
    void* _data{};
};

}  // namespace DsVeosCoSim

#endif
