// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <cstdint>
#include <memory>
#include <string_view>

#include "BusBuffer.h"    // IWYU pragma: keep
#include "CoSimHelper.h"  // IWYU pragma: keep
#include "DsVeosCoSim/CoSimTypes.h"
#include "LogHelper.h"  // IWYU pragma: keep
#include "Socket.h"

[[maybe_unused]] constexpr uint32_t Infinite = UINT32_MAX;

#ifndef CTRL
#define CTRL(c) ((c) & 037)
#endif

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

[[maybe_unused]] constexpr uint32_t DefaultTimeout = 1000;

[[nodiscard]] int32_t GetChar();

[[nodiscard]] DsVeosCoSim::Result StartUp();

void SetEnvVariable(std::string_view name, std::string_view value);

[[nodiscard]] std::string_view GetLoopBackAddress(DsVeosCoSim::AddressFamily addressFamily);

[[nodiscard]] DsVeosCoSim::Result SendComplete(const DsVeosCoSim::Socket& socket, const void* buffer, size_t length);

[[nodiscard]] DsVeosCoSim::Result ReceiveComplete(const DsVeosCoSim::Socket& socket, void* buffer, size_t length);

[[nodiscard]] DsVeosCoSim::Result CreateBusBuffer(DsVeosCoSim::CoSimType coSimType,
                                                  DsVeosCoSim::ConnectionKind connectionKind,
                                                  std::string_view name,
                                                  const std::vector<DsVeosCoSim::CanController>& controllers,
                                                  std::unique_ptr<DsVeosCoSim::BusBuffer>& busBuffer);

[[nodiscard]] DsVeosCoSim::Result CreateBusBuffer(DsVeosCoSim::CoSimType coSimType,
                                                  DsVeosCoSim::ConnectionKind connectionKind,
                                                  std::string_view name,
                                                  const std::vector<DsVeosCoSim::EthController>& controllers,
                                                  std::unique_ptr<DsVeosCoSim::BusBuffer>& busBuffer);

[[nodiscard]] DsVeosCoSim::Result CreateBusBuffer(DsVeosCoSim::CoSimType coSimType,
                                                  DsVeosCoSim::ConnectionKind connectionKind,
                                                  std::string_view name,
                                                  const std::vector<DsVeosCoSim::LinController>& controllers,
                                                  std::unique_ptr<DsVeosCoSim::BusBuffer>& busBuffer);
