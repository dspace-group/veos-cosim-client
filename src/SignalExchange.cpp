// Copyright dSPACE SE & Co. KG. All rights reserved.

#include "SignalExchange.hpp"

#include <algorithm>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <fmt/format.h>

#include "Channel.hpp"
#include "CoSimTypes.hpp"
#include "SignalExchangeCommon.hpp"
#include "SignalExchangeLocalWin.hpp"
#include "SignalExchangeLocked.hpp"
#include "SignalExchangeRemote.hpp"
#include "Protocol.hpp"
#include "Result.hpp"

namespace DsVeosCoSim {

namespace {

using SignalExchangeDetail::ISignalExchangePart;
#ifdef _WIN32
using SignalExchangeDetail::LocalSignalExchangePart;
#endif
using SignalExchangeDetail::LockedSignalExchangePart;
using SignalExchangeDetail::RemoteSignalExchangePart;

class SignalExchangeImpl final : public SignalExchange {
public:
    SignalExchangeImpl() = default;
    ~SignalExchangeImpl() noexcept override = default;

    SignalExchangeImpl(const SignalExchangeImpl&) = delete;
    SignalExchangeImpl& operator=(const SignalExchangeImpl&) = delete;

    SignalExchangeImpl(SignalExchangeImpl&&) = delete;
    SignalExchangeImpl& operator=(SignalExchangeImpl&&) = delete;

    [[nodiscard]] Result Initialize(CoSimType coSimType,
                                    [[maybe_unused]] ConnectionKind connectionKind,
                                    [[maybe_unused]] const std::string& name,
                                    const std::vector<IoSignal>& incomingSignals,
                                    const std::vector<IoSignal>& outgoingSignals,
                                    IProtocol& protocol) {
        const std::vector<IoSignal>* writeSignals = &outgoingSignals;
        const std::vector<IoSignal>* readSignals = &incomingSignals;
        if (coSimType == CoSimType::Server) {
            writeSignals = &incomingSignals;
            readSignals = &outgoingSignals;
        }

#ifdef _WIN32
        if (connectionKind == ConnectionKind::Local) {
            std::string outgoingName = fmt::format("{}.Outgoing", name);
            std::string incomingName = fmt::format("{}.Incoming", name);
            if (coSimType == CoSimType::Server) {
                std::swap(incomingName, outgoingName);
            }

            _readPart = std::make_unique<LocalSignalExchangePart>(protocol, *readSignals, incomingName);
            _writePart = std::make_unique<LocalSignalExchangePart>(protocol, *writeSignals, outgoingName);
        } else {
#endif
            _readPart = std::make_unique<RemoteSignalExchangePart>(protocol, *readSignals);
            _writePart = std::make_unique<RemoteSignalExchangePart>(protocol, *writeSignals);
#ifdef _WIN32
        }
#endif

        if (coSimType == CoSimType::Client) {
            _readPart = std::make_unique<LockedSignalExchangePart>(std::move(_readPart));
            _writePart = std::make_unique<LockedSignalExchangePart>(std::move(_writePart));
        }

        CheckResult(_readPart->Initialize());
        CheckResult(_writePart->Initialize());

        ClearData();
        return CreateOk();
    }

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
                                          const std::string& name,
                                          const std::vector<IoSignal>& incomingSignals,
                                          const std::vector<IoSignal>& outgoingSignals,
                                          IProtocol& protocol,
                                          std::unique_ptr<SignalExchange>& signalExchange) {
    auto tmpSignalExchange = std::make_unique<SignalExchangeImpl>();
    CheckResult(tmpSignalExchange->Initialize(coSimType, connectionKind, name, incomingSignals, outgoingSignals, protocol));
    signalExchange = std::move(tmpSignalExchange);
    return CreateOk();
}

}  // namespace DsVeosCoSim
