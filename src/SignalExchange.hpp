// Copyright dSPACE SE & Co. KG. All rights reserved.

#pragma once

#include <cstdint>
#include <memory>
#include <string_view>
#include <vector>

#include "Channel.hpp"
#include "CoSimTypes.hpp"
#include "Protocol.hpp"
#include "Result.hpp"

// Forward declaration to avoid including implementation header
namespace DsVeosCoSim::SignalExchangeDetail {

class ISignalExchangePart;

}  // namespace DsVeosCoSim::SignalExchangeDetail

namespace DsVeosCoSim {

class SignalExchange final {
public:
    SignalExchange(std::unique_ptr<SignalExchangeDetail::ISignalExchangePart> writePart, std::unique_ptr<SignalExchangeDetail::ISignalExchangePart> readPart);
    ~SignalExchange() noexcept;

    SignalExchange(const SignalExchange&) = delete;
    SignalExchange& operator=(const SignalExchange&) = delete;

    SignalExchange(SignalExchange&&) = delete;
    SignalExchange& operator=(SignalExchange&&) = delete;

    void ClearData() const;

    [[nodiscard]] Result Write(IoSignalId signalId, uint32_t length, const void* value) const;
    [[nodiscard]] Result Read(IoSignalId signalId, uint32_t& length, void* value) const;
    [[nodiscard]] Result Read(IoSignalId signalId, uint32_t& length, const void** value) const;

    [[nodiscard]] Result Serialize(ChannelWriter& writer) const;
    [[nodiscard]] Result Deserialize(ChannelReader& reader, SimulationTime simulationTime, const Callbacks& callbacks) const;

private:
    std::unique_ptr<SignalExchangeDetail::ISignalExchangePart> _writePart;
    std::unique_ptr<SignalExchangeDetail::ISignalExchangePart> _readPart;
};

[[nodiscard]] Result CreateSignalExchange(CoSimType coSimType,
                                          ConnectionKind connectionKind,
                                          std::string_view name,
                                          const std::vector<IoSignal>& incomingSignals,
                                          const std::vector<IoSignal>& outgoingSignals,
                                          IProtocol& protocol,
                                          std::unique_ptr<SignalExchange>& signalExchange);

}  // namespace DsVeosCoSim
