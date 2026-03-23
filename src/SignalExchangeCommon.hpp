// Copyright dSPACE SE & Co. KG. All rights reserved.

#pragma once

#include <cstddef>
#include <cstdint>
#include <unordered_map>
#include <vector>

#include "Channel.hpp"
#include "CoSimTypes.hpp"
#include "Logger.hpp"
#include "Result.hpp"

namespace DsVeosCoSim::SignalExchangeDetail {

struct SignalMetaData {
    IoSignal info{};
    size_t dataTypeSize{};
    size_t totalDataSize{};
    size_t signalIndex{};
};

using SignalMetaDataPtr = SignalMetaData*;

// The registry keeps lookup-by-id while also assigning a dense signal index for
// buffer-backed storage vectors.
class SignalRegistry final {
public:
    SignalRegistry() = default;
    ~SignalRegistry() noexcept = default;

    SignalRegistry(const SignalRegistry&) = delete;
    SignalRegistry& operator=(const SignalRegistry&) = delete;

    SignalRegistry(SignalRegistry&&) = delete;
    SignalRegistry& operator=(SignalRegistry&&) = delete;

    [[nodiscard]] Result Initialize(const std::vector<IoSignal>& ioSignals) {
        size_t nextSignalIndex = 0;
        for (const auto& ioSignal : ioSignals) {
            if (ioSignal.length == 0) {
                LogError("Invalid length 0 for IO signal '{}'.", ioSignal.name);
                return CreateError();
            }

            size_t dataTypeSize = GetDataTypeSize(ioSignal.dataType);
            if (dataTypeSize == 0) {
                LogError("Invalid data type for IO signal '{}'.", ioSignal.name);
                return CreateError();
            }

            auto search = _metaDataLookup.find(ioSignal.id);
            if (search != _metaDataLookup.end()) {
                LogError("Duplicated IO signal id {}.", ioSignal.id);
                return CreateError();
            }

            size_t totalDataSize = dataTypeSize * ioSignal.length;

            SignalMetaData metaData{};
            metaData.info = ioSignal;
            metaData.dataTypeSize = dataTypeSize;
            metaData.totalDataSize = totalDataSize;
            metaData.signalIndex = nextSignalIndex++;

            _metaDataLookup[ioSignal.id] = metaData;
        }

        return CreateOk();
    }

    [[nodiscard]] Result FindMetaData(IoSignalId signalId, SignalMetaDataPtr& metaData) {
        auto search = _metaDataLookup.find(signalId);
        if (search != _metaDataLookup.end()) {
            metaData = &search->second;
            return CreateOk();
        }

        LogError("IO signal id {} is unknown.", signalId);
        return CreateError();
    }

    [[nodiscard]] std::unordered_map<IoSignalId, SignalMetaData>& GetMetaDataLookup() {
        return _metaDataLookup;
    }

private:
    std::unordered_map<IoSignalId, SignalMetaData> _metaDataLookup;
};

class ISignalExchangePart {
public:
    ISignalExchangePart() = default;
    virtual ~ISignalExchangePart() noexcept = default;

    ISignalExchangePart(const ISignalExchangePart&) = delete;
    ISignalExchangePart& operator=(const ISignalExchangePart&) = delete;

    ISignalExchangePart(ISignalExchangePart&&) = delete;
    ISignalExchangePart& operator=(ISignalExchangePart&&) = delete;

    [[nodiscard]] virtual Result Initialize() = 0;
    virtual void ClearData() = 0;
    [[nodiscard]] virtual Result Write(IoSignalId signalId, uint32_t length, const void* value) = 0;
    [[nodiscard]] virtual Result Read(IoSignalId signalId, uint32_t& length, void* value) = 0;
    [[nodiscard]] virtual Result Read(IoSignalId signalId, uint32_t& length, const void** value) = 0;
    [[nodiscard]] virtual Result Serialize(ChannelWriter& writer) = 0;
    [[nodiscard]] virtual Result Deserialize(ChannelReader& reader, SimulationTime simulationTime, const Callbacks& callbacks) = 0;
};

}  // namespace DsVeosCoSim::SignalExchangeDetail
