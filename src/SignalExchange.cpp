// Copyright dSPACE SE & Co. KG. All rights reserved.

#include "SignalExchange.hpp"

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <fmt/format.h>

#include "Channel.hpp"
#include "CoSimTypes.hpp"
#include "Protocol.hpp"
#include "Result.hpp"
#include "SignalExchangeCommon.hpp"
#include "SignalExchangeLocalWin.hpp"
#include "SignalExchangeLocked.hpp"
#include "SignalExchangeRemote.hpp"

namespace DsVeosCoSim {

namespace {

using SignalExchangeDetail::ISignalExchangePart;
#ifdef _WIN32
using SignalExchangeDetail::LocalSignalExchangePart;
#endif
using SignalExchangeDetail::LockedSignalExchangePart;
using SignalExchangeDetail::RemoteSignalExchangePart;

[[nodiscard]] Result CreateSignalExchangePart(CoSimType coSimType,
                                              [[maybe_unused]] ConnectionKind connectionKind,
                                              [[maybe_unused]] std::string_view name,
                                              const std::vector<IoSignal>& signals,
                                              IProtocol& protocol,
                                              [[maybe_unused]] bool isWritePart,
                                              std::unique_ptr<ISignalExchangePart>& signalExchangePart) {
#ifdef _WIN32
    if (connectionKind == ConnectionKind::Local) {
        std::string partName = fmt::format("{}.{}", name, isWritePart ? "Outgoing" : "Incoming");
        if (coSimType == CoSimType::Server) {
            partName = fmt::format("{}.{}", name, isWritePart ? "Incoming" : "Outgoing");
        }

        CheckResult(LocalSignalExchangePart::Create(protocol, signals, std::move(partName), signalExchangePart));
    } else {
#endif
        CheckResult(RemoteSignalExchangePart::Create(protocol, signals, signalExchangePart));
#ifdef _WIN32
    }
#endif

    if (coSimType == CoSimType::Client) {
        signalExchangePart = std::make_unique<LockedSignalExchangePart>(std::move(signalExchangePart));
    }

    return CreateOk();
}

class SignalExchangeImpl final : public SignalExchange {
public:
    SignalExchangeImpl(std::unique_ptr<ISignalExchangePart> writePart, std::unique_ptr<ISignalExchangePart> readPart)
        : _writePart(std::move(writePart)), _readPart(std::move(readPart)) {
    }

    ~SignalExchangeImpl() noexcept override = default;

    SignalExchangeImpl(const SignalExchangeImpl&) = delete;
    SignalExchangeImpl& operator=(const SignalExchangeImpl&) = delete;

    SignalExchangeImpl(SignalExchangeImpl&&) = delete;
    SignalExchangeImpl& operator=(SignalExchangeImpl&&) = delete;

    void ClearData() const override {
        _readPart->ClearData();
        _writePart->ClearData();
    }

    [[nodiscard]] Result Write(IoSignalId signalId, uint32_t length, const void* value) const override {
        return _writePart->Write(signalId, length, value);
    }

    [[nodiscard]] Result Read(IoSignalId signalId, uint32_t& length, void* value) const override {
        return _readPart->Read(signalId, length, value);
    }

    [[nodiscard]] Result Read(IoSignalId signalId, uint32_t& length, const void** value) const override {
        return _readPart->Read(signalId, length, value);
    }

    [[nodiscard]] Result Serialize(ChannelWriter& writer) const override {
        return _writePart->Serialize(writer);
    }

    [[nodiscard]] Result Deserialize(ChannelReader& reader, SimulationTime simulationTime, const Callbacks& callbacks) const override {
        return _readPart->Deserialize(reader, simulationTime, callbacks);
    }

private:
    std::unique_ptr<ISignalExchangePart> _writePart;
    std::unique_ptr<ISignalExchangePart> _readPart;
};

}  // namespace

[[nodiscard]] Result CreateSignalExchange(CoSimType coSimType,
                                          ConnectionKind connectionKind,
                                          std::string_view name,
                                          const std::vector<IoSignal>& incomingSignals,
                                          const std::vector<IoSignal>& outgoingSignals,
                                          IProtocol& protocol,
                                          std::unique_ptr<SignalExchange>& signalExchange) {
    const std::vector<IoSignal>* writeSignals = &outgoingSignals;
    const std::vector<IoSignal>* readSignals = &incomingSignals;
    if (coSimType == CoSimType::Server) {
        writeSignals = &incomingSignals;
        readSignals = &outgoingSignals;
    }

    std::unique_ptr<ISignalExchangePart> writePart;
    CheckResult(CreateSignalExchangePart(coSimType, connectionKind, name, *writeSignals, protocol, true, writePart));

    std::unique_ptr<ISignalExchangePart> readPart;
    CheckResult(CreateSignalExchangePart(coSimType, connectionKind, name, *readSignals, protocol, false, readPart));

    signalExchange = std::make_unique<SignalExchangeImpl>(std::move(writePart), std::move(readPart));
    signalExchange->ClearData();
    return CreateOk();
}

}  // namespace DsVeosCoSim
