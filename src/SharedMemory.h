// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#ifdef _WIN32

#include <string>

#include "CoSimTypes.h"

namespace DsVeosCoSim {

class SharedMemory {
public:
    explicit SharedMemory(const std::string& name);
    ~SharedMemory() noexcept;

    SharedMemory(const SharedMemory&) = delete;
    SharedMemory& operator=(const SharedMemory&) = delete;

    SharedMemory(const SharedMemory&&) = delete;
    SharedMemory& operator=(const SharedMemory&&) = delete;

    [[nodiscard]] Result Create();

    [[nodiscard]] void* GetBuffer();

private:
    std::string _name;
    void* _handle{};
    void* _buffer{};
};

}  // namespace DsVeosCoSim

#endif
