// Copyright dSPACE SE & Co. KG. All rights reserved.

#pragma once

#include <memory>
#include <mutex>

#include "SignalExchangeCommon.hpp"

namespace DsVeosCoSim::SignalExchangeDetail {

class LockedSignalExchangePart final : public ISignalExchangePart {
public:
    explicit LockedSignalExchangePart(std::unique_ptr<ISignalExchangePart> proxiedPart) : _proxiedPart(std::move(proxiedPart)) {
    }

    ~LockedSignalExchangePart() noexcept override = default;

    LockedSignalExchangePart(const LockedSignalExchangePart&) = delete;
    LockedSignalExchangePart& operator=(const LockedSignalExchangePart&) = delete;

    LockedSignalExchangePart(LockedSignalExchangePart&&) = delete;
    LockedSignalExchangePart& operator=(LockedSignalExchangePart&&) = delete;

    void ClearData() override {
        std::scoped_lock lock(_mutex);
        _proxiedPart->ClearData();
    }

    [[nodiscard]] Result Write(IoSignalId signalId, uint32_t length, const void* value) override {
        std::scoped_lock lock(_mutex);
        return _proxiedPart->Write(signalId, length, value);
    }

    [[nodiscard]] Result Read(IoSignalId signalId, uint32_t& length, void* value) override {
        std::scoped_lock lock(_mutex);
        return _proxiedPart->Read(signalId, length, value);
    }

    [[nodiscard]] Result Read(IoSignalId signalId, uint32_t& length, const void** value) override {
        std::scoped_lock lock(_mutex);
        return _proxiedPart->Read(signalId, length, value);
    }

    [[nodiscard]] Result Serialize(ChannelWriter& writer) override {
        std::scoped_lock lock(_mutex);
        return _proxiedPart->Serialize(writer);
    }

    [[nodiscard]] Result Deserialize(ChannelReader& reader, SimulationTime simulationTime, const Callbacks& callbacks) override {
        std::scoped_lock lock(_mutex);
        return _proxiedPart->Deserialize(reader, simulationTime, callbacks);
    }

private:
    std::unique_ptr<ISignalExchangePart> _proxiedPart;
    std::mutex _mutex;
};

}  // namespace DsVeosCoSim::SignalExchangeDetail
