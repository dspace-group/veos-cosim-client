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

namespace DsVeosCoSim {

class SignalExchange {
protected:
    SignalExchange() = default;

public:
    virtual ~SignalExchange() noexcept = default;

    SignalExchange(const SignalExchange&) = delete;
    SignalExchange& operator=(const SignalExchange&) = delete;

    SignalExchange(SignalExchange&&) = delete;
    SignalExchange& operator=(SignalExchange&&) = delete;

    virtual void ClearData() const = 0;

    [[nodiscard]] virtual Result Write(IoSignalId signalId, uint32_t length, const void* value) const = 0;
    [[nodiscard]] virtual Result Read(IoSignalId signalId, uint32_t& length, void* value) const = 0;
    [[nodiscard]] virtual Result Read(IoSignalId signalId, uint32_t& length, const void** value) const = 0;

    [[nodiscard]] virtual Result Serialize(ChannelWriter& writer) const = 0;
    [[nodiscard]] virtual Result Deserialize(ChannelReader& reader, SimulationTime simulationTime, const Callbacks& callbacks) const = 0;
};

[[nodiscard]] Result CreateSignalExchange(CoSimType coSimType,
                                          ConnectionKind connectionKind,
                                          std::string_view name,
                                          const std::vector<IoSignal>& incomingSignals,
                                          const std::vector<IoSignal>& outgoingSignals,
                                          IProtocol& protocol,
                                          std::unique_ptr<SignalExchange>& signalExchange);

}  // namespace DsVeosCoSim
