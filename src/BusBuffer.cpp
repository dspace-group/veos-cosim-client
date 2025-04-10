// Copyright dSPACE GmbH. All rights reserved.

#include "BusBuffer.h"

#include <cstdint>
#include <cstring>
#include <functional>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <vector>

#include "Channel.h"
#include "CoSimHelper.h"
#include "DsVeosCoSim/CoSimTypes.h"
#include "Environment.h"
#include "RingBuffer.h"

#ifdef _WIN32
#include <atomic>

#include "OsUtilities.h"
#endif

namespace DsVeosCoSim {

namespace {

void Check(const CanMessageContainer& message) {
    if (message.length > CanMessageMaxLength) {
        throw std::runtime_error("CAN message data exceeds maximum length.");
    }

    if (!HasFlag(message.flags, CanMessageFlags::FlexibleDataRateFormat)) {
        if (message.length > 8) {
            throw std::runtime_error(
                "CAN message flags invalid. A DLC > 8 requires the flexible data rate format flag.");
        }

        if (HasFlag(message.flags, CanMessageFlags::BitRateSwitch)) {
            throw std::runtime_error(
                "CAN message flags invalid. A bit rate switch flag requires the flexible data rate format flag.");
        }
    }
}

[[nodiscard]] bool SerializeTo(const CanMessageContainer& message, ChannelWriter& writer) {
    CheckResultWithMessage(writer.Write(message.timestamp), "Could not write timestamp.");
    CheckResultWithMessage(writer.Write(message.controllerId), "Could not write controller id.");
    CheckResultWithMessage(writer.Write(message.id), "Could not write id.");
    CheckResultWithMessage(writer.Write(message.flags), "Could not write flags.");
    CheckResultWithMessage(writer.Write(message.length), "Could not write length.");
    CheckResultWithMessage(writer.Write(message.data.data(), message.length), "Could not write data.");
    return true;
}

[[nodiscard]] bool DeserializeFrom(CanMessageContainer& message, ChannelReader& reader) {
    CheckResultWithMessage(reader.Read(message.timestamp), "Could not read timestamp.");
    CheckResultWithMessage(reader.Read(message.controllerId), "Could not read controller id.");
    CheckResultWithMessage(reader.Read(message.id), "Could not read id.");
    CheckResultWithMessage(reader.Read(message.flags), "Could not read flags.");
    CheckResultWithMessage(reader.Read(message.length), "Could not read length");
    Check(message);
    CheckResultWithMessage(reader.Read(message.data.data(), message.length), "Could not read data.");
    return true;
}

void WriteTo(const CanMessageContainer& container, CanMessage& message) {
    message.timestamp = container.timestamp;
    message.controllerId = container.controllerId;
    message.id = container.id;
    message.flags = container.flags;
    message.length = container.length;
    message.data = container.data.data();
}

void Check(const EthMessageContainer& message) {
    if (message.length > EthMessageMaxLength) {
        throw std::runtime_error("Ethernet message data exceeds maximum length.");
    }
}

[[nodiscard]] bool SerializeTo(const EthMessageContainer& message, ChannelWriter& writer) {
    CheckResultWithMessage(writer.Write(message.timestamp), "Could not write timestamp.");
    CheckResultWithMessage(writer.Write(message.controllerId), "Could not write controller id.");
    CheckResultWithMessage(writer.Write(message.flags), "Could not write flags.");
    CheckResultWithMessage(writer.Write(message.length), "Could not write length.");
    CheckResultWithMessage(writer.Write(message.data.data(), message.length), "Could not write data.");
    return true;
}

[[nodiscard]] bool DeserializeFrom(EthMessageContainer& message, ChannelReader& reader) {
    CheckResultWithMessage(reader.Read(message.timestamp), "Could not read timestamp.");
    CheckResultWithMessage(reader.Read(message.controllerId), "Could not read controller id.");
    CheckResultWithMessage(reader.Read(message.flags), "Could not read flags.");
    CheckResultWithMessage(reader.Read(message.length), "Could not read length.");
    Check(message);
    CheckResultWithMessage(reader.Read(message.data.data(), message.length), "Could not read data.");
    return true;
}

void WriteTo(const EthMessageContainer& container, EthMessage& message) {
    message.timestamp = container.timestamp;
    message.controllerId = container.controllerId;
    message.flags = container.flags;
    message.length = container.length;
    message.data = container.data.data();
}

void Check(const LinMessageContainer& message) {
    if (message.length > LinMessageMaxLength) {
        throw std::runtime_error("LIN message data exceeds maximum length.");
    }
}

[[nodiscard]] bool SerializeTo(const LinMessageContainer& message, ChannelWriter& writer) {
    CheckResultWithMessage(writer.Write(message.timestamp), "Could not write timestamp.");
    CheckResultWithMessage(writer.Write(message.controllerId), "Could not write controller id.");
    CheckResultWithMessage(writer.Write(message.id), "Could not write id.");
    CheckResultWithMessage(writer.Write(message.flags), "Could not write flags.");
    CheckResultWithMessage(writer.Write(message.length), "Could not write length.");
    CheckResultWithMessage(writer.Write(message.data.data(), message.length), "Could not write data.");
    return true;
}

[[nodiscard]] bool DeserializeFrom(LinMessageContainer& message, ChannelReader& reader) {
    CheckResultWithMessage(reader.Read(message.timestamp), "Could not read timestamp.");
    CheckResultWithMessage(reader.Read(message.controllerId), "Could not read controller id.");
    CheckResultWithMessage(reader.Read(message.id), "Could not read id.");
    CheckResultWithMessage(reader.Read(message.flags), "Could not read flags.");
    CheckResultWithMessage(reader.Read(message.length), "Could not read length.");
    Check(message);
    CheckResultWithMessage(reader.Read(message.data.data(), message.length), "Could not read data.");
    return true;
}

void WriteTo(const LinMessageContainer& container, LinMessage& message) {
    message.timestamp = container.timestamp;
    message.controllerId = container.controllerId;
    message.id = container.id;
    message.flags = container.flags;
    message.length = container.length;
    message.data = container.data.data();
}

template <typename TMessageExtern, typename TControllerExtern>
class BusProtocolBufferBase {
protected:
    using Callback = std::function<void(SimulationTime, const TControllerExtern&, const TMessageExtern&)>;

    struct ControllerExtension {
        TControllerExtern info{};
        bool warningSent{};
        size_t controllerIndex{};

        void ClearData() {
            warningSent = false;
        }
    };

public:
    BusProtocolBufferBase() noexcept = default;
    virtual ~BusProtocolBufferBase() noexcept = default;

    BusProtocolBufferBase(const BusProtocolBufferBase&) = delete;
    BusProtocolBufferBase& operator=(const BusProtocolBufferBase&) = delete;

    BusProtocolBufferBase(BusProtocolBufferBase&&) = delete;
    BusProtocolBufferBase& operator=(BusProtocolBufferBase&&) = delete;

    void Initialize(const CoSimType coSimType,
                    const std::string& name,
                    const std::vector<TControllerExtern>& controllers) {
        _coSimType = coSimType;

        size_t totalQueueItemsCountPerBuffer = 0;
        size_t nextControllerIndex = 0;
        for (const auto& controller : controllers) {
            const auto search = _controllers.find(controller.id);
            if (search != _controllers.end()) {
                std::string message = "Duplicated controller id ";
                message.append(ToString(controller.id));
                message.append(".");
                throw std::runtime_error(message);
            }

            ControllerExtension extension{};
            extension.info = controller;
            extension.controllerIndex = nextControllerIndex++;
            _controllers[controller.id] = extension;
            totalQueueItemsCountPerBuffer += controller.queueSize;
        }

        InitializeInternal(name, totalQueueItemsCountPerBuffer);
    }

    void ClearData() {
        if (_coSimType == CoSimType::Client) {
            std::lock_guard lock(_mutex);
            ClearDataInternal();
            return;
        }

        ClearDataInternal();
    }

    [[nodiscard]] bool Transmit(const TMessageExtern& messageExtern) {
        if (_coSimType == CoSimType::Client) {
            std::lock_guard lock(_mutex);
            return TransmitInternal(messageExtern);
        }

        return TransmitInternal(messageExtern);
    }

    [[nodiscard]] bool Receive(TMessageExtern& messageExtern) {
        if (_coSimType == CoSimType::Client) {
            std::lock_guard lock(_mutex);
            return ReceiveInternal(messageExtern);
        }

        return ReceiveInternal(messageExtern);
    }

    [[nodiscard]] bool Serialize(ChannelWriter& writer) {
        if (_coSimType == CoSimType::Client) {
            std::lock_guard lock(_mutex);
            return SerializeInternal(writer);
        }

        return SerializeInternal(writer);
    }

    [[nodiscard]] bool Deserialize(ChannelReader& reader,
                                   const SimulationTime simulationTime,
                                   const Callback& callback) {
        if (_coSimType == CoSimType::Client) {
            std::lock_guard lock(_mutex);
            return DeserializeInternal(reader, simulationTime, callback);
        }

        return DeserializeInternal(reader, simulationTime, callback);
    }

protected:
    virtual void InitializeInternal(const std::string& name, size_t totalQueueItemsCountPerBuffer) = 0;

    virtual void ClearDataInternal() = 0;

    [[nodiscard]] virtual bool TransmitInternal(const TMessageExtern& messageExtern) = 0;
    [[nodiscard]] virtual bool ReceiveInternal(TMessageExtern& messageExtern) = 0;

    [[nodiscard]] virtual bool SerializeInternal(ChannelWriter& writer) = 0;
    [[nodiscard]] virtual bool DeserializeInternal(ChannelReader& reader,
                                                   SimulationTime simulationTime,
                                                   const Callback& callback) = 0;

    [[nodiscard]] ControllerExtension& FindController(BusControllerId controllerId) {
        const auto search = _controllers.find(controllerId);
        if (search != _controllers.end()) {
            return search->second;
        }

        std::string message = "Controller id ";
        message.append(ToString(controllerId));
        message.append(" is unknown.");
        throw std::runtime_error(message);
    }

    std::unordered_map<BusControllerId, ControllerExtension> _controllers;

private:
    CoSimType _coSimType{};
    std::mutex _mutex;
};

template <typename TMessage, typename TMessageExtern, typename TControllerExtern>
class RemoteBusProtocolBuffer final : public BusProtocolBufferBase<TMessageExtern, TControllerExtern> {
    using Base = BusProtocolBufferBase<TMessageExtern, TControllerExtern>;
    using Extension = typename Base::ControllerExtension;

public:
    RemoteBusProtocolBuffer() noexcept = default;
    ~RemoteBusProtocolBuffer() noexcept override = default;

    RemoteBusProtocolBuffer(const RemoteBusProtocolBuffer&) = delete;
    RemoteBusProtocolBuffer& operator=(const RemoteBusProtocolBuffer&) = delete;

    RemoteBusProtocolBuffer(RemoteBusProtocolBuffer&&) = delete;
    RemoteBusProtocolBuffer& operator=(RemoteBusProtocolBuffer&&) = delete;

protected:
    void InitializeInternal([[maybe_unused]] const std::string& name, size_t totalQueueItemsCountPerBuffer) override {
        _messageCountPerController.resize(this->_controllers.size());
        _messageBuffer = RingBuffer<TMessage>(totalQueueItemsCountPerBuffer);
    }

    void ClearDataInternal() override {
        for (auto& [controllerId, dataPerController] : Base::_controllers) {
            dataPerController.ClearData();
        }

        for (auto& messageCount : _messageCountPerController) {
            messageCount = 0;
        }

        _messageBuffer.Clear();
    }

    [[nodiscard]] bool TransmitInternal(const TMessageExtern& messageExtern) override {
        Extension& extension = Base::FindController(messageExtern.controllerId);

        if (_messageCountPerController[extension.controllerIndex] == extension.info.queueSize) {
            if (!extension.warningSent) {
                std::string message = "Transmit buffer for controller '";
                message.append(extension.info.name);
                message.append("' is full. Messages are dropped.");
                LogWarning(message);
                extension.warningSent = true;
            }

            return false;
        }

        auto message = Convert(messageExtern);
        Check(message);

        _messageBuffer.PushBack(std::move(message));
        ++_messageCountPerController[extension.controllerIndex];
        return true;
    }

    [[nodiscard]] bool ReceiveInternal(TMessageExtern& messageExtern) override {
        if (_messageBuffer.IsEmpty()) {
            return false;
        }

        TMessage& message = _messageBuffer.PopFront();
        WriteTo(message, messageExtern);

        Extension& extension = Base::FindController(message.controllerId);
        --_messageCountPerController[extension.controllerIndex];
        return true;
    }

    [[nodiscard]] bool SerializeInternal(ChannelWriter& writer) override {
        const auto count = static_cast<uint32_t>(_messageBuffer.Size());
        CheckResultWithMessage(writer.Write(count), "Could not write count of messages.");

        for (uint32_t i = 0; i < count; i++) {
            TMessage& message = _messageBuffer.PopFront();

            if (IsProtocolTracingEnabled()) {
                LogProtocolDataTrace(ToString(message));
            }

            CheckResultWithMessage(SerializeTo(message, writer), "Could not serialize message.");
        }

        for (auto& [id, extension] : Base::_controllers) {
            _messageCountPerController[extension.controllerIndex] = 0;
        }

        return true;
    }

    [[nodiscard]] bool DeserializeInternal(ChannelReader& reader,
                                           SimulationTime simulationTime,
                                           const typename Base::Callback& callback) override {
        uint32_t totalCount{};
        CheckResultWithMessage(reader.Read(totalCount), "Could not read count of messages.");

        for (uint32_t i = 0; i < totalCount; i++) {
            TMessage message{};
            CheckResultWithMessage(DeserializeFrom(message, reader), "Could not deserialize message.");

            if (IsProtocolTracingEnabled()) {
                LogProtocolDataTrace(ToString(message));
            }

            Extension& extension = Base::FindController(message.controllerId);

            if (callback) {
                callback(simulationTime, extension.info, Convert(message));
                continue;
            }

            if (_messageCountPerController[extension.controllerIndex] == extension.info.queueSize) {
                if (!extension.warningSent) {
                    std::string message = "Receive buffer for controller '";
                    message.append(extension.info.name);
                    message.append("' is full. Messages are dropped.");
                    LogWarning(message);
                    extension.warningSent = true;
                }

                continue;
            }

            ++_messageCountPerController[extension.controllerIndex];
            _messageBuffer.PushBack(std::move(message));
        }

        return true;
    }

private:
    std::vector<uint32_t> _messageCountPerController;

    RingBuffer<TMessage> _messageBuffer;
};

#ifdef _WIN32

template <typename T>
class ShmRingBuffer final {
public:
    ShmRingBuffer() noexcept = default;
    ~ShmRingBuffer() noexcept = default;

    ShmRingBuffer(const ShmRingBuffer&) = delete;
    ShmRingBuffer& operator=(const ShmRingBuffer&) = delete;

    ShmRingBuffer(ShmRingBuffer&& other) = delete;
    ShmRingBuffer& operator=(ShmRingBuffer&& other) = delete;

    void Initialize(const uint32_t capacity) noexcept {
        _capacity = capacity;
    }

    void Clear() noexcept {
        _readIndex = 0;
        _writeIndex = 0;
        _size = 0;
    }

    [[nodiscard]] uint32_t Size() const noexcept {
        return _size;
    }

    [[nodiscard]] bool IsEmpty() const noexcept {
        return _size == 0;
    }

    [[nodiscard]] bool IsFull() const noexcept {
        return _size == _capacity;
    }

    void PushBack(T&& element) {
        if (IsFull()) {
            throw std::runtime_error("SHM ring buffer is full.");
        }

        const uint32_t currentWriteIndex = _writeIndex;
        _items[currentWriteIndex] = std::move(element);

        ++_writeIndex;
        if (_writeIndex == _capacity) {
            _writeIndex = 0;
        }

        ++_size;
    }

    [[nodiscard]] T& PopFront() {
        if (IsEmpty()) {
            throw std::runtime_error("SHM ring buffer is empty.");
        }

        --_size;

        T& item = _items[_readIndex];

        ++_readIndex;
        if (_readIndex == _capacity) {
            _readIndex = 0;
        }

        return item;
    }

private:
    uint32_t _capacity{};           // Read by reader and writer
    std::atomic<uint32_t> _size{};  // Read and written by reader and writer
    uint32_t _readIndex{};          // Read and written by reader
    uint32_t _writeIndex{};         // Read and written by writerF

    // Zero sized array would be correct here, since the items are inside a shared memory. But that leads to
    // warnings, so we add set the size to 1
    T _items[1]{};
};

template <typename TMessage, typename TMessageExtern, typename TControllerExtern>
class LocalBusProtocolBuffer final : public BusProtocolBufferBase<TMessageExtern, TControllerExtern> {
    using Base = BusProtocolBufferBase<TMessageExtern, TControllerExtern>;
    using Extension = typename Base::ControllerExtension;

public:
    LocalBusProtocolBuffer() noexcept = default;
    ~LocalBusProtocolBuffer() noexcept override = default;

    LocalBusProtocolBuffer(const LocalBusProtocolBuffer&) = delete;
    LocalBusProtocolBuffer& operator=(const LocalBusProtocolBuffer&) = delete;

    LocalBusProtocolBuffer(LocalBusProtocolBuffer&&) = delete;
    LocalBusProtocolBuffer& operator=(LocalBusProtocolBuffer&&) = delete;

protected:
    void InitializeInternal(const std::string& name, const size_t totalQueueItemsCountPerBuffer) override {
        // The memory layout looks like this:
        // [ list of message count per controller ]
        // [ message buffer ]

        const size_t sizeOfMessageCountPerController = Base::_controllers.size() * sizeof(std::atomic<uint32_t>);
        const size_t sizeOfRingBuffer =
            sizeof(ShmRingBuffer<TMessage>) + (totalQueueItemsCountPerBuffer * sizeof(TMessage));

        size_t sizeOfSharedMemory = 0;
        sizeOfSharedMemory += sizeOfMessageCountPerController;
        sizeOfSharedMemory += sizeOfRingBuffer;

        _sharedMemory = CreateOrOpenSharedMemory(name, sizeOfSharedMemory);

        auto* pointerToMessageCountPerController = static_cast<uint8_t*>(_sharedMemory->data());
        auto* pointerToMessageBuffer = pointerToMessageCountPerController + sizeOfMessageCountPerController;

        _messageCountPerController = reinterpret_cast<std::atomic<uint32_t>*>(pointerToMessageCountPerController);
        _messageBuffer = reinterpret_cast<ShmRingBuffer<TMessage>*>(pointerToMessageBuffer);

        _messageBuffer->Initialize(static_cast<uint32_t>(totalQueueItemsCountPerBuffer));

        ClearDataInternal();
    }

    void ClearDataInternal() override {
        for (auto& [controllerId, dataPerController] : Base::_controllers) {
            dataPerController.ClearData();
        }

        _totalReceiveCount = 0;

        for (size_t i = 0; i < Base::_controllers.size(); i++) {
            _messageCountPerController[i].store(0);
        }

        if (_messageBuffer) {
            _messageBuffer->Clear();
        }
    }

    [[nodiscard]] bool TransmitInternal(const TMessageExtern& messageExtern) override {
        Extension& extension = Base::FindController(messageExtern.controllerId);
        std::atomic<uint32_t>& messageCount = _messageCountPerController[extension.controllerIndex];

        if (messageCount.load() == extension.info.queueSize) {
            if (!extension.warningSent) {
                std::string message = "Transmit buffer for controller '";
                message.append(extension.info.name);
                message.append("' is full. Messages are dropped.");
                LogWarning(message);
                extension.warningSent = true;
            }

            return false;
        }

        auto message = Convert(messageExtern);
        Check(message);

        _messageBuffer->PushBack(std::move(message));
        messageCount.fetch_add(1);
        return true;
    }

    [[nodiscard]] bool ReceiveInternal(TMessageExtern& messageExtern) override {
        if (_totalReceiveCount == 0) {
            return false;
        }

        TMessage& message = _messageBuffer->PopFront();
        WriteTo(message, messageExtern);

        Extension& extension = Base::FindController(messageExtern.controllerId);
        std::atomic<uint32_t>& receiveCount = _messageCountPerController[extension.controllerIndex];
        receiveCount.fetch_sub(1);
        _totalReceiveCount--;

        return true;
    }

    [[nodiscard]] bool SerializeInternal(ChannelWriter& writer) override {
        CheckResultWithMessage(writer.Write<uint32_t>(_messageBuffer->Size()), "Could not write transmit count.");
        return true;
    }

    [[nodiscard]] bool DeserializeInternal(ChannelReader& reader,
                                           SimulationTime simulationTime,
                                           const typename Base::Callback& callback) override {
        uint32_t receiveCount{};
        CheckResultWithMessage(reader.Read(receiveCount), "Could not read receive count.");
        _totalReceiveCount += receiveCount;

        if (!callback) {
            return true;
        }

        while (_totalReceiveCount > 0) {
            TMessage& message = _messageBuffer->PopFront();

            if (IsProtocolTracingEnabled()) {
                LogProtocolDataTrace(ToString(message));
            }

            Extension& extension = Base::FindController(message.controllerId);
            std::atomic<uint32_t>& receiveCountPerController = _messageCountPerController[extension.controllerIndex];
            receiveCountPerController.fetch_sub(1);
            _totalReceiveCount--;

            callback(simulationTime, extension.info, Convert(message));
        }

        return true;
    }

private:
    uint32_t _totalReceiveCount{};
    std::atomic<uint32_t>* _messageCountPerController{};
    ShmRingBuffer<TMessage>* _messageBuffer{};

    std::unique_ptr<SharedMemory> _sharedMemory;
};

#endif

#ifdef _WIN32
using LocalCanBuffer = LocalBusProtocolBuffer<CanMessageContainer, CanMessage, CanController>;
using LocalEthBuffer = LocalBusProtocolBuffer<EthMessageContainer, EthMessage, EthController>;
using LocalLinBuffer = LocalBusProtocolBuffer<LinMessageContainer, LinMessage, LinController>;
#endif

using RemoteCanBuffer = RemoteBusProtocolBuffer<CanMessageContainer, CanMessage, CanController>;
using RemoteEthBuffer = RemoteBusProtocolBuffer<EthMessageContainer, EthMessage, EthController>;
using RemoteLinBuffer = RemoteBusProtocolBuffer<LinMessageContainer, LinMessage, LinController>;

class BusBufferImpl final : public BusBuffer {
    using CanBufferBase = BusProtocolBufferBase<CanMessage, CanController>;
    using EthBufferBase = BusProtocolBufferBase<EthMessage, EthController>;
    using LinBufferBase = BusProtocolBufferBase<LinMessage, LinController>;

public:
    BusBufferImpl(const CoSimType coSimType,
                  [[maybe_unused]] const ConnectionKind connectionKind,
                  const std::string& name,
                  const std::vector<CanController>& canControllers,
                  const std::vector<EthController>& ethControllers,
                  const std::vector<LinController>& linControllers) {
#ifdef _WIN32
        if (connectionKind == ConnectionKind::Local) {
            _canTransmitBuffer = std::make_unique<LocalCanBuffer>();
            _ethTransmitBuffer = std::make_unique<LocalEthBuffer>();
            _linTransmitBuffer = std::make_unique<LocalLinBuffer>();

            _canReceiveBuffer = std::make_unique<LocalCanBuffer>();
            _ethReceiveBuffer = std::make_unique<LocalEthBuffer>();
            _linReceiveBuffer = std::make_unique<LocalLinBuffer>();
        } else {
#endif
            _canTransmitBuffer = std::make_unique<RemoteCanBuffer>();
            _ethTransmitBuffer = std::make_unique<RemoteEthBuffer>();
            _linTransmitBuffer = std::make_unique<RemoteLinBuffer>();

            _canReceiveBuffer = std::make_unique<RemoteCanBuffer>();
            _ethReceiveBuffer = std::make_unique<RemoteEthBuffer>();
            _linReceiveBuffer = std::make_unique<RemoteLinBuffer>();
#ifdef _WIN32
        }
#endif

        const std::string suffixForTransmit = coSimType == CoSimType::Client ? "Transmit" : "Receive";
        const std::string suffixForReceive = coSimType == CoSimType::Client ? "Receive" : "Transmit";

        std::string canTransmitBufferName = name;
        canTransmitBufferName.append(".Can.");
        canTransmitBufferName.append(suffixForTransmit);
        std::string ethTransmitBufferName = name;
        ethTransmitBufferName.append(".Eth.");
        ethTransmitBufferName.append(suffixForTransmit);
        std::string linTransmitBufferName = name;
        linTransmitBufferName.append(".Lin.");
        linTransmitBufferName.append(suffixForTransmit);

        std::string canReceiveBufferName = name;
        canReceiveBufferName.append(".Can.");
        canReceiveBufferName.append(suffixForReceive);
        std::string ethReceiveBufferName = name;
        ethReceiveBufferName.append(".Eth.");
        ethReceiveBufferName.append(suffixForReceive);
        std::string linReceiveBufferName = name;
        linReceiveBufferName.append(".Lin.");
        linReceiveBufferName.append(suffixForReceive);

        _canTransmitBuffer->Initialize(coSimType, canTransmitBufferName, canControllers);
        _ethTransmitBuffer->Initialize(coSimType, ethTransmitBufferName, ethControllers);
        _linTransmitBuffer->Initialize(coSimType, linTransmitBufferName, linControllers);
        _canReceiveBuffer->Initialize(coSimType, canReceiveBufferName, canControllers);
        _ethReceiveBuffer->Initialize(coSimType, ethReceiveBufferName, ethControllers);
        _linReceiveBuffer->Initialize(coSimType, linReceiveBufferName, linControllers);
    }

    ~BusBufferImpl() noexcept override = default;

    BusBufferImpl(const BusBufferImpl&) = delete;
    BusBufferImpl& operator=(const BusBufferImpl&) = delete;

    BusBufferImpl(BusBufferImpl&&) = delete;
    BusBufferImpl& operator=(BusBufferImpl&&) = delete;

    void ClearData() const override {
        _canTransmitBuffer->ClearData();
        _ethTransmitBuffer->ClearData();
        _linTransmitBuffer->ClearData();

        _canReceiveBuffer->ClearData();
        _ethReceiveBuffer->ClearData();
        _linReceiveBuffer->ClearData();
    }

    [[nodiscard]] bool Transmit(const CanMessage& message) const override {
        return _canTransmitBuffer->Transmit(message);
    }

    [[nodiscard]] bool Transmit(const EthMessage& message) const override {
        return _ethTransmitBuffer->Transmit(message);
    }

    [[nodiscard]] bool Transmit(const LinMessage& message) const override {
        return _linTransmitBuffer->Transmit(message);
    }

    [[nodiscard]] bool Receive(CanMessage& message) const override {
        return _canReceiveBuffer->Receive(message);
    }

    [[nodiscard]] bool Receive(EthMessage& message) const override {
        return _ethReceiveBuffer->Receive(message);
    }

    [[nodiscard]] bool Receive(LinMessage& message) const override {
        return _linReceiveBuffer->Receive(message);
    }

    [[nodiscard]] bool Serialize(ChannelWriter& writer) const override {
        CheckResultWithMessage(_canTransmitBuffer->Serialize(writer), "Could not transmit CAN messages.");
        CheckResultWithMessage(_ethTransmitBuffer->Serialize(writer), "Could not transmit ETH messages.");
        CheckResultWithMessage(_linTransmitBuffer->Serialize(writer), "Could not transmit LIN messages.");
        return true;
    }

    [[nodiscard]] bool Deserialize(ChannelReader& reader,
                                   const SimulationTime simulationTime,
                                   const Callbacks& callbacks) const override {
        CheckResultWithMessage(
            _canReceiveBuffer->Deserialize(reader, simulationTime, callbacks.canMessageReceivedCallback),
            "Could not receive CAN messages.");
        CheckResultWithMessage(
            _ethReceiveBuffer->Deserialize(reader, simulationTime, callbacks.ethMessageReceivedCallback),
            "Could not receive ETH messages.");
        CheckResultWithMessage(
            _linReceiveBuffer->Deserialize(reader, simulationTime, callbacks.linMessageReceivedCallback),
            "Could not receive LIN messages.");
        return true;
    }

private:
    std::unique_ptr<CanBufferBase> _canTransmitBuffer;
    std::unique_ptr<EthBufferBase> _ethTransmitBuffer;
    std::unique_ptr<LinBufferBase> _linTransmitBuffer;

    std::unique_ptr<CanBufferBase> _canReceiveBuffer;
    std::unique_ptr<EthBufferBase> _ethReceiveBuffer;
    std::unique_ptr<LinBufferBase> _linReceiveBuffer;
};

}  // namespace

[[nodiscard]] std::unique_ptr<BusBuffer> CreateBusBuffer(CoSimType coSimType,
                                                         ConnectionKind connectionKind,
                                                         const std::string& name,
                                                         const std::vector<CanController>& canControllers,
                                                         const std::vector<EthController>& ethControllers,
                                                         const std::vector<LinController>& linControllers) {
    return std::make_unique<BusBufferImpl>(coSimType,
                                           connectionKind,
                                           name,
                                           canControllers,
                                           ethControllers,
                                           linControllers);
}

}  // namespace DsVeosCoSim
