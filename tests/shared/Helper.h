// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string_view>

#include "BusBuffer.h"    // IWYU pragma: keep
#include "CoSimHelper.h"  // IWYU pragma: keep
#include "DsVeosCoSim/CoSimTypes.h"
#include "LogHelper.h"  // IWYU pragma: keep
#include "Socket.h"

namespace DsVeosCoSim {

[[maybe_unused]] constexpr uint32_t DefaultTimeout = 1000;

#define MustBeOk(result)                                             \
    do {                                                             \
        Result _result_ = (result);                                  \
        if (!IsOk(_result_)) {                                       \
            LogError("Expected Ok but was {}.", ToString(_result_)); \
            exit(1);                                                 \
        }                                                            \
    } while (0)

#define MustBeDisconnected(result)                                             \
    do {                                                                       \
        Result _result_ = (result);                                            \
        if (_result_ != DsVeosCoSim::Result::Disconnected) {                   \
            LogError("Expected Disconnected but was {}.", ToString(_result_)); \
            exit(1);                                                           \
        }                                                                      \
    } while (0)

#define MustBeTrue(result)                            \
    do {                                              \
        if (!(result)) {                              \
            LogError("Expected true but was false."); \
            exit(1);                                  \
        }                                             \
    } while (0)

#ifndef CTRL
#define CTRL(c) ((c) & 037)
#endif

[[nodiscard]] int32_t GetChar();

void SetEnvVariable(std::string_view name, std::string_view value);

[[nodiscard]] bool operator==(const IoSignal& first, const IoSignal& second);
[[nodiscard]] std::ostream& operator<<(std::ostream& stream, const IoSignal& signal);

[[nodiscard]] bool operator==(const CanController& first, const CanController& second);
[[nodiscard]] std::ostream& operator<<(std::ostream& stream, const CanController& controller);
[[nodiscard]] bool operator==(const CanMessageContainer& first, const CanMessageContainer& second);
[[nodiscard]] std::ostream& operator<<(std::ostream& stream, const CanMessage& message);

[[nodiscard]] bool operator==(const EthController& first, const EthController& second);
[[nodiscard]] std::ostream& operator<<(std::ostream& stream, const EthController& controller);
[[nodiscard]] bool operator==(const EthMessageContainer& first, const EthMessageContainer& second);
[[nodiscard]] std::ostream& operator<<(std::ostream& stream, const EthMessage& message);

[[nodiscard]] bool operator==(const LinController& first, const LinController& second);
[[nodiscard]] std::ostream& operator<<(std::ostream& stream, const LinController& controller);
[[nodiscard]] bool operator==(const LinMessageContainer& first, const LinMessageContainer& second);
[[nodiscard]] std::ostream& operator<<(std::ostream& stream, const LinMessage& message);

[[nodiscard]] Result StartUp();

[[nodiscard]] std::string_view GetLoopBackAddress(AddressFamily addressFamily);

[[nodiscard]] Result SendComplete(const Socket& socket, const void* buffer, size_t length);

[[nodiscard]] Result ReceiveComplete(const Socket& socket, void* buffer, size_t length);

[[nodiscard]] Result CreateBusBuffer(CoSimType coSimType,
                                     ConnectionKind connectionKind,
                                     std::string_view name,
                                     const std::vector<CanController>& controllers,
                                     std::unique_ptr<BusBuffer>& busBuffer);

[[nodiscard]] Result CreateBusBuffer(CoSimType coSimType,
                                     ConnectionKind connectionKind,
                                     std::string_view name,
                                     const std::vector<EthController>& controllers,
                                     std::unique_ptr<BusBuffer>& busBuffer);

[[nodiscard]] Result CreateBusBuffer(CoSimType coSimType,
                                     ConnectionKind connectionKind,
                                     std::string_view name,
                                     const std::vector<LinController>& controllers,
                                     std::unique_ptr<BusBuffer>& busBuffer);

}  // namespace DsVeosCoSim
