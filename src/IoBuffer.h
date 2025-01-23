// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "Channel.h"
#include "CoSimTypes.h"
#include "RingBuffer.h"

#ifdef _WIN32
#include "SharedMemory.h"
#endif

namespace DsVeosCoSim {

class IoPartBufferBase {
protected:
    struct MetaData {
        IoSignal info{};
        size_t dataTypeSize{};
        size_t totalDataSize{};
        size_t signalIndex{};
    };

public:
    IoPartBufferBase(CoSimType coSimType, const std::vector<IoSignal>& signals);
    virtual ~IoPartBufferBase() noexcept = default;

    IoPartBufferBase(const IoPartBufferBase&) = delete;
    IoPartBufferBase& operator=(const IoPartBufferBase&) = delete;

    IoPartBufferBase(IoPartBufferBase&&) = delete;
    IoPartBufferBase& operator=(IoPartBufferBase&&) = delete;

    void ClearData();

    void Write(IoSignalId signalId, uint32_t length, const void* value);
    void Read(IoSignalId signalId, uint32_t& length, void* value);
    void Read(IoSignalId signalId, uint32_t& length, const void** value);

    [[nodiscard]] bool Serialize(ChannelWriter& writer);
    [[nodiscard]] bool Deserialize(ChannelReader& reader, SimulationTime simulationTime, const Callbacks& callbacks);

protected:
    virtual void ClearDataInternal() = 0;

    virtual void WriteInternal(IoSignalId signalId, uint32_t length, const void* value) = 0;
    virtual void ReadInternal(IoSignalId signalId, uint32_t& length, void* value) = 0;
    virtual void ReadInternal(IoSignalId signalId, uint32_t& length, const void** value) = 0;

    [[nodiscard]] virtual bool SerializeInternal(ChannelWriter& writer) = 0;
    [[nodiscard]] virtual bool DeserializeInternal(ChannelReader& reader,
                                                   SimulationTime simulationTime,
                                                   const Callbacks& callbacks) = 0;

    [[nodiscard]] MetaData& FindMetaData(IoSignalId signalId);

    CoSimType _coSimType{};
    std::unordered_map<IoSignalId, MetaData> _metaDataLookup;
    RingBuffer<MetaData*> _changedSignalsQueue;

private:
    std::mutex _mutex;
};

class RemoteIoPartBuffer final : public IoPartBufferBase {
    struct Data {
        uint32_t currentLength{};
        bool isChanged{};
        std::vector<uint8_t> buffer;
    };

public:
    RemoteIoPartBuffer(CoSimType coSimType, const std::string& name, const std::vector<IoSignal>& signals);
    ~RemoteIoPartBuffer() noexcept override = default;

    RemoteIoPartBuffer(const RemoteIoPartBuffer&) = delete;
    RemoteIoPartBuffer& operator=(const RemoteIoPartBuffer&) = delete;

    RemoteIoPartBuffer(RemoteIoPartBuffer&&) = delete;
    RemoteIoPartBuffer& operator=(RemoteIoPartBuffer&&) = delete;

protected:
    void ClearDataInternal() override;

    void WriteInternal(IoSignalId signalId, uint32_t length, const void* value) override;
    void ReadInternal(IoSignalId signalId, uint32_t& length, void* value) override;
    void ReadInternal(IoSignalId signalId, uint32_t& length, const void** value) override;

    [[nodiscard]] bool SerializeInternal(ChannelWriter& writer) override;
    [[nodiscard]] bool DeserializeInternal(ChannelReader& reader,
                                           SimulationTime simulationTime,
                                           const Callbacks& callbacks) override;

private:
    std::vector<Data> _dataVector;
};

#ifdef _WIN32

class LocalIoPartBuffer final : public IoPartBufferBase {
    struct DataBuffer {
        uint32_t currentLength{};
        uint8_t data[1]{};
    };

    struct Data {
        size_t offsetOfDataBufferInShm{};
        size_t offsetOfBackupDataBufferInShm{};
        bool isChanged{};
    };

public:
    LocalIoPartBuffer(CoSimType coSimType, const std::string& name, const std::vector<IoSignal>& signals);
    ~LocalIoPartBuffer() noexcept override = default;

    LocalIoPartBuffer(const LocalIoPartBuffer&) = delete;
    LocalIoPartBuffer& operator=(const LocalIoPartBuffer&) = delete;

    LocalIoPartBuffer(LocalIoPartBuffer&&) = delete;
    LocalIoPartBuffer& operator=(LocalIoPartBuffer&&) = delete;

protected:
    void ClearDataInternal() override;

    void WriteInternal(IoSignalId signalId, uint32_t length, const void* value) override;
    void ReadInternal(IoSignalId signalId, uint32_t& length, void* value) override;
    void ReadInternal(IoSignalId signalId, uint32_t& length, const void** value) override;

    [[nodiscard]] bool SerializeInternal(ChannelWriter& writer) override;
    [[nodiscard]] bool DeserializeInternal(ChannelReader& reader,
                                           SimulationTime simulationTime,
                                           const Callbacks& callbacks) override;

private:
    [[nodiscard]] DataBuffer* GetDataBuffer(size_t offset) const;

    static void FlipBuffers(Data& data);

    std::vector<Data> _dataVector;
    SharedMemory _sharedMemory;
};

#endif

class IoBuffer {
public:
    IoBuffer(CoSimType coSimType,
             ConnectionKind connectionKind,
             const std::string& name,
             const std::vector<IoSignal>& incomingSignals,
             const std::vector<IoSignal>& outgoingSignals);
    ~IoBuffer() noexcept = default;

    IoBuffer(const IoBuffer&) = delete;
    IoBuffer& operator=(const IoBuffer&) = delete;

    IoBuffer(IoBuffer&&) = delete;
    IoBuffer& operator=(IoBuffer&&) = delete;

    void ClearData() const;

    void Write(IoSignalId signalId, uint32_t length, const void* value) const;
    void Read(IoSignalId signalId, uint32_t& length, void* value) const;
    void Read(IoSignalId signalId, uint32_t& length, const void** value) const;

    [[nodiscard]] bool Serialize(ChannelWriter& writer) const;
    [[nodiscard]] bool Deserialize(ChannelReader& reader,
                                   SimulationTime simulationTime,
                                   const Callbacks& callbacks) const;

private:
    std::unique_ptr<IoPartBufferBase> _writeBuffer;
    std::unique_ptr<IoPartBufferBase> _readBuffer;
};

}  // namespace DsVeosCoSim
