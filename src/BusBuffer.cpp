// Copyright dSPACE GmbH. All rights reserved.

#include "BusBuffer.h"

#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
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

[[nodiscard]] Result Check(const CanMessageContainer& message) {
    if (message.length > CanMessageMaxLength) {
        LogError("CAN message data exceeds maximum length.");
        return Result::Error;
    }

    if (!HasFlag(message.flags, CanMessageFlags::FlexibleDataRateFormat)) {
        if (message.length > 8) {
            LogError("CAN message flags are invalid. A DLC > 8 requires the flexible data rate format flag.");
            return Result::Error;
        }

        if (HasFlag(message.flags, CanMessageFlags::BitRateSwitch)) {
            LogError(
                "CAN message flags are invalid. A bit rate switch flag requires the flexible data rate format flag.");
            return Result::Error;
        }
    }

    return Result::Ok;
}

[[nodiscard]] Result SerializeTo(const CanMessageContainer& message, ChannelWriter& writer) {
    CheckResultWithMessage(writer.Write(message.timestamp), "Could not write timestamp.");
    CheckResultWithMessage(writer.Write(message.controllerId), "Could not write controller id.");
    CheckResultWithMessage(writer.Write(message.id), "Could not write id.");
    CheckResultWithMessage(writer.Write(message.flags), "Could not write flags.");
    CheckResultWithMessage(writer.Write(message.length), "Could not write length.");
    CheckResultWithMessage(writer.Write(message.data.data(), message.length), "Could not write data.");
    return Result::Ok;
}

[[nodiscard]] Result DeserializeFrom(CanMessageContainer& message, ChannelReader& reader) {
    CheckResultWithMessage(reader.Read(message.timestamp), "Could not read timestamp.");
    CheckResultWithMessage(reader.Read(message.controllerId), "Could not read controller id.");
    CheckResultWithMessage(reader.Read(message.id), "Could not read id.");
    CheckResultWithMessage(reader.Read(message.flags), "Could not read flags.");
    CheckResultWithMessage(reader.Read(message.length), "Could not read length");
    CheckResult(Check(message));
    CheckResultWithMessage(reader.Read(message.data.data(), message.length), "Could not read data.");
    return Result::Ok;
}

void WriteTo(const CanMessageContainer& container, CanMessage& message) {
    message.timestamp = container.timestamp;
    message.controllerId = container.controllerId;
    message.id = container.id;
    message.flags = container.flags;
    message.length = container.length;
    message.data = container.data.data();
}

[[nodiscard]] Result Check(const EthMessageContainer& message) {
    if (message.length > EthMessageMaxLength) {
        LogError("Ethernet message data exceeds maximum length.");
        return Result::Error;
    }

    return Result::Ok;
}

[[nodiscard]] Result SerializeTo(const EthMessageContainer& message, ChannelWriter& writer) {
    CheckResultWithMessage(writer.Write(message.timestamp), "Could not write timestamp.");
    CheckResultWithMessage(writer.Write(message.controllerId), "Could not write controller id.");
    CheckResultWithMessage(writer.Write(message.flags), "Could not write flags.");
    CheckResultWithMessage(writer.Write(message.length), "Could not write length.");
    CheckResultWithMessage(writer.Write(message.data.data(), message.length), "Could not write data.");
    return Result::Ok;
}

[[nodiscard]] Result DeserializeFrom(EthMessageContainer& message, ChannelReader& reader) {
    CheckResultWithMessage(reader.Read(message.timestamp), "Could not read timestamp.");
    CheckResultWithMessage(reader.Read(message.controllerId), "Could not read controller id.");
    CheckResultWithMessage(reader.Read(message.flags), "Could not read flags.");
    CheckResultWithMessage(reader.Read(message.length), "Could not read length.");
    CheckResult(Check(message));
    CheckResultWithMessage(reader.Read(message.data.data(), message.length), "Could not read data.");
    return Result::Ok;
}

void WriteTo(const EthMessageContainer& container, EthMessage& message) {
    message.timestamp = container.timestamp;
    message.controllerId = container.controllerId;
    message.flags = container.flags;
    message.length = container.length;
    message.data = container.data.data();
}

[[nodiscard]] Result Check(const LinMessageContainer& message) {
    if (message.length > LinMessageMaxLength) {
        LogError("LIN message data exceeds maximum length.");
        return Result::Error;
    }

    return Result::Ok;
}

[[nodiscard]] Result SerializeTo(const LinMessageContainer& message, ChannelWriter& writer) {
    CheckResultWithMessage(writer.Write(message.timestamp), "Could not write timestamp.");
    CheckResultWithMessage(writer.Write(message.controllerId), "Could not write controller id.");
    CheckResultWithMessage(writer.Write(message.id), "Could not write id.");
    CheckResultWithMessage(writer.Write(message.flags), "Could not write flags.");
    CheckResultWithMessage(writer.Write(message.length), "Could not write length.");
    CheckResultWithMessage(writer.Write(message.data.data(), message.length), "Could not write data.");
    return Result::Ok;
}

[[nodiscard]] Result DeserializeFrom(LinMessageContainer& message, ChannelReader& reader) {
    CheckResultWithMessage(reader.Read(message.timestamp), "Could not read timestamp.");
    CheckResultWithMessage(reader.Read(message.controllerId), "Could not read controller id.");
    CheckResultWithMessage(reader.Read(message.id), "Could not read id.");
    CheckResultWithMessage(reader.Read(message.flags), "Could not read flags.");
    CheckResultWithMessage(reader.Read(message.length), "Could not read length.");
    CheckResult(Check(message));
    CheckResultWithMessage(reader.Read(message.data.data(), message.length), "Could not read data.");
    return Result::Ok;
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
    BusProtocolBufferBase() = default;
    virtual ~BusProtocolBufferBase() = default;

    BusProtocolBufferBase(const BusProtocolBufferBase&) = delete;
    BusProtocolBufferBase& operator=(const BusProtocolBufferBase&) = delete;

    BusProtocolBufferBase(BusProtocolBufferBase&&) = delete;
    BusProtocolBufferBase& operator=(BusProtocolBufferBase&&) = delete;

    [[nodiscard]] Result Initialize(CoSimType coSimType,
                                    std::string_view name,
                                    const std::vector<TControllerExtern>& controllers) {
        _coSimType = coSimType;

        size_t totalQueueItemsCountPerBuffer = 0;
        size_t nextControllerIndex = 0;
        for (const auto& controller : controllers) {
            auto search = _controllers.find(controller.id);
            if (search != _controllers.end()) {
                std::string message = "Duplicated controller id ";
                message.append(ToString(controller.id));
                message.append(".");
                LogError(message);
                return Result::Error;
            }

            ControllerExtension extension{};
            extension.info = controller;
            extension.controllerIndex = nextControllerIndex++;
            _controllers[controller.id] = extension;
            totalQueueItemsCountPerBuffer += controller.queueSize;
        }

        return InitializeInternal(name, totalQueueItemsCountPerBuffer);
    }

    void ClearData() {
        if (_coSimType == CoSimType::Client) {
            std::lock_guard lock(_mutex);
            ClearDataInternal();
            return;
        }

        ClearDataInternal();
    }

    [[nodiscard]] Result Transmit(const TMessageExtern& messageExtern) {
        if (_coSimType == CoSimType::Client) {
            std::lock_guard lock(_mutex);
            return TransmitInternal(messageExtern);
        }

        return TransmitInternal(messageExtern);
    }

    [[nodiscard]] Result Receive(TMessageExtern& messageExtern) {
        if (_coSimType == CoSimType::Client) {
            std::lock_guard lock(_mutex);
            return ReceiveInternal(messageExtern);
        }

        return ReceiveInternal(messageExtern);
    }

    [[nodiscard]] Result Serialize(ChannelWriter& writer) {
        if (_coSimType == CoSimType::Client) {
            std::lock_guard lock(_mutex);
            return SerializeInternal(writer);
        }

        return SerializeInternal(writer);
    }

    [[nodiscard]] Result Deserialize(ChannelReader& reader, SimulationTime simulationTime, const Callback& callback) {
        if (_coSimType == CoSimType::Client) {
            std::lock_guard lock(_mutex);
            return DeserializeInternal(reader, simulationTime, callback);
        }

        return DeserializeInternal(reader, simulationTime, callback);
    }

protected:
    [[nodiscard]] virtual Result InitializeInternal(std::string_view name, size_t totalQueueItemsCountPerBuffer) = 0;

    virtual void ClearDataInternal() = 0;

    [[nodiscard]] virtual Result TransmitInternal(const TMessageExtern& messageExtern) = 0;
    [[nodiscard]] virtual Result ReceiveInternal(TMessageExtern& messageExtern) = 0;

    [[nodiscard]] virtual Result SerializeInternal(ChannelWriter& writer) = 0;
    [[nodiscard]] virtual Result DeserializeInternal(ChannelReader& reader,
                                                     SimulationTime simulationTime,
                                                     const Callback& callback) = 0;

    [[nodiscard]] Result FindController(BusControllerId controllerId, ControllerExtension*& extension) {
        auto search = _controllers.find(controllerId);
        if (search != _controllers.end()) {
            extension = &search->second;
            return Result::Ok;
        }

        std::string message = "Controller id ";
        message.append(ToString(controllerId));
        message.append(" is unknown.");
        LogError(message);
        return Result::Error;
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

    using ExtensionPtr = Extension*;

public:
    RemoteBusProtocolBuffer() = default;
    ~RemoteBusProtocolBuffer() override = default;

    RemoteBusProtocolBuffer(const RemoteBusProtocolBuffer&) = delete;
    RemoteBusProtocolBuffer& operator=(const RemoteBusProtocolBuffer&) = delete;

    RemoteBusProtocolBuffer(RemoteBusProtocolBuffer&&) = delete;
    RemoteBusProtocolBuffer& operator=(RemoteBusProtocolBuffer&&) = delete;

protected:
    [[nodiscard]] Result InitializeInternal([[maybe_unused]] std::string_view name,
                                            size_t totalQueueItemsCountPerBuffer) override {
        _messageCountPerController.resize(this->_controllers.size());
        _messageBuffer = RingBuffer<TMessage>(totalQueueItemsCountPerBuffer);
        return Result::Ok;
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

    [[nodiscard]] Result TransmitInternal(const TMessageExtern& messageExtern) override {
        ExtensionPtr extension{};
        CheckResult(Base::FindController(messageExtern.controllerId, extension));

        if (_messageCountPerController[extension->controllerIndex] == extension->info.queueSize) {
            if (!extension->warningSent) {
                std::string warningMessage = "Transmit buffer for controller '";
                warningMessage.append(extension->info.name);
                warningMessage.append("' is full. Messages are dropped.");
                LogWarning(warningMessage);
                extension->warningSent = true;
            }

            return Result::Full;
        }

        auto message = Convert(messageExtern);
        CheckResult(Check(message));

        CheckResult(_messageBuffer.PushBack(std::move(message)));
        ++_messageCountPerController[extension->controllerIndex];
        return Result::Ok;
    }

    [[nodiscard]] Result ReceiveInternal(TMessageExtern& messageExtern) override {
        if (_messageBuffer.IsEmpty()) {
            return Result::Empty;
        }

        const TMessage* message{};
        CheckResult(_messageBuffer.PopFront(message));
        WriteTo(*message, messageExtern);

        ExtensionPtr extension{};
        CheckResult(Base::FindController(message->controllerId, extension));
        --_messageCountPerController[extension->controllerIndex];
        return Result::Ok;
    }

    [[nodiscard]] Result SerializeInternal(ChannelWriter& writer) override {
        auto count = static_cast<uint32_t>(_messageBuffer.Size());
        CheckResultWithMessage(writer.Write(count), "Could not write count of messages.");

        for (uint32_t i = 0; i < count; i++) {
            const TMessage* message{};
            CheckResult(_messageBuffer.PopFront(message));

            if (IsProtocolTracingEnabled()) {
                LogProtocolDataTrace(ToString(*message));
            }

            CheckResultWithMessage(SerializeTo(*message, writer), "Could not serialize message.");
        }

        for (auto& [id, extension] : Base::_controllers) {
            _messageCountPerController[extension.controllerIndex] = 0;
        }

        return Result::Ok;
    }

    [[nodiscard]] Result DeserializeInternal(ChannelReader& reader,
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

            ExtensionPtr extension{};
            CheckResult(Base::FindController(message.controllerId, extension));

            if (callback) {
                callback(simulationTime, extension->info, Convert(message));
                continue;
            }

            if (_messageCountPerController[extension->controllerIndex] == extension->info.queueSize) {
                if (!extension->warningSent) {
                    std::string warningMessage = "Receive buffer for controller '";
                    warningMessage.append(extension->info.name);
                    warningMessage.append("' is full. Messages are dropped.");
                    LogWarning(warningMessage);
                    extension->warningSent = true;
                }

                continue;
            }

            ++_messageCountPerController[extension->controllerIndex];
            CheckResult(_messageBuffer.PushBack(std::move(message)));
        }

        return Result::Ok;
    }

private:
    std::vector<uint32_t> _messageCountPerController;

    RingBuffer<TMessage> _messageBuffer;
};

#ifdef _WIN32

template <typename T>
class ShmRingBuffer final {
public:
    ShmRingBuffer() = default;
    ~ShmRingBuffer() = default;

    ShmRingBuffer(const ShmRingBuffer&) = delete;
    ShmRingBuffer& operator=(const ShmRingBuffer&) = delete;

    ShmRingBuffer(ShmRingBuffer&& other) = delete;
    ShmRingBuffer& operator=(ShmRingBuffer&& other) = delete;

    void Initialize(uint32_t capacity) {
        _capacity = capacity;
    }

    void Clear() {
        _readIndex = 0;
        _writeIndex = 0;
        _size = 0;
    }

    [[nodiscard]] uint32_t Size() const {
        return _size;
    }

    [[nodiscard]] bool IsEmpty() const {
        return _size == 0;
    }

    [[nodiscard]] bool IsFull() const {
        return _size == _capacity;
    }

    [[nodiscard]] Result PushBack(T&& item) {
        if (IsFull()) {
            LogError("SHM ring buffer is full.");
            return Result::Ok;
        }

        uint32_t currentWriteIndex = _writeIndex;
        _items[currentWriteIndex] = std::move(item);

        ++_writeIndex;
        if (_writeIndex == _capacity) {
            _writeIndex = 0;
        }

        ++_size;
        return Result::Ok;
    }

    [[nodiscard]] Result PopFront(const T*& item) {
        if (IsEmpty()) {
            LogError("SHM ring buffer is empty.");
            return Result::Ok;
        }

        --_size;

        item = &_items[_readIndex];

        ++_readIndex;
        if (_readIndex == _capacity) {
            _readIndex = 0;
        }

        return Result::Ok;
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

    using ExtensionPtr = Extension*;

public:
    LocalBusProtocolBuffer() = default;
    ~LocalBusProtocolBuffer() override = default;

    LocalBusProtocolBuffer(const LocalBusProtocolBuffer&) = delete;
    LocalBusProtocolBuffer& operator=(const LocalBusProtocolBuffer&) = delete;

    LocalBusProtocolBuffer(LocalBusProtocolBuffer&&) = delete;
    LocalBusProtocolBuffer& operator=(LocalBusProtocolBuffer&&) = delete;

protected:
    [[nodiscard]] Result InitializeInternal(std::string_view name, size_t totalQueueItemsCountPerBuffer) override {
        // The memory layout looks like this:
        // [ list of message count per controller ]
        // [ message buffer ]

        size_t sizeOfMessageCountPerController = Base::_controllers.size() * sizeof(std::atomic<uint32_t>);
        size_t sizeOfRingBuffer = sizeof(ShmRingBuffer<TMessage>) + (totalQueueItemsCountPerBuffer * sizeof(TMessage));

        size_t sizeOfSharedMemory = 0;
        sizeOfSharedMemory += sizeOfMessageCountPerController;
        sizeOfSharedMemory += sizeOfRingBuffer;

        CheckResult(SharedMemory::CreateOrOpen(name, sizeOfSharedMemory, _sharedMemory));

        auto* pointerToMessageCountPerController = static_cast<uint8_t*>(_sharedMemory.GetData());
        auto* pointerToMessageBuffer = pointerToMessageCountPerController + sizeOfMessageCountPerController;

        _messageCountPerController = reinterpret_cast<std::atomic<uint32_t>*>(pointerToMessageCountPerController);
        _messageBuffer = reinterpret_cast<ShmRingBuffer<TMessage>*>(pointerToMessageBuffer);

        _messageBuffer->Initialize(static_cast<uint32_t>(totalQueueItemsCountPerBuffer));

        ClearDataInternal();
        return Result::Ok;
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

    [[nodiscard]] Result TransmitInternal(const TMessageExtern& messageExtern) override {
        ExtensionPtr extension{};
        CheckResult(Base::FindController(messageExtern.controllerId, extension));
        std::atomic<uint32_t>& messageCount = _messageCountPerController[extension->controllerIndex];

        if (messageCount.load() == extension->info.queueSize) {
            if (!extension->warningSent) {
                std::string message = "Transmit buffer for controller '";
                message.append(extension->info.name);
                message.append("' is full. Messages are dropped.");
                LogWarning(message);
                extension->warningSent = true;
            }

            return Result::Full;
        }

        auto message = Convert(messageExtern);
        CheckResult(Check(message));

        CheckResult(_messageBuffer->PushBack(std::move(message)));
        messageCount.fetch_add(1);
        return Result::Ok;
    }

    [[nodiscard]] Result ReceiveInternal(TMessageExtern& messageExtern) override {
        if (_totalReceiveCount == 0) {
            return Result::Empty;
        }

        const TMessage* message{};
        CheckResult(_messageBuffer->PopFront(message));
        WriteTo(*message, messageExtern);

        ExtensionPtr extension{};
        CheckResult(Base::FindController(messageExtern.controllerId, extension));
        std::atomic<uint32_t>& receiveCount = _messageCountPerController[extension->controllerIndex];
        receiveCount.fetch_sub(1);
        _totalReceiveCount--;
        return Result::Ok;
    }

    [[nodiscard]] Result SerializeInternal(ChannelWriter& writer) override {
        CheckResultWithMessage(writer.Write<uint32_t>(_messageBuffer->Size()), "Could not write transmit count.");
        return Result::Ok;
    }

    [[nodiscard]] Result DeserializeInternal(ChannelReader& reader,
                                             SimulationTime simulationTime,
                                             const typename Base::Callback& callback) override {
        uint32_t receiveCount{};
        CheckResultWithMessage(reader.Read(receiveCount), "Could not read receive count.");
        _totalReceiveCount += receiveCount;

        if (!callback) {
            return Result::Ok;
        }

        while (_totalReceiveCount > 0) {
            const TMessage* message{};
            CheckResult(_messageBuffer->PopFront(message));

            if (IsProtocolTracingEnabled()) {
                LogProtocolDataTrace(ToString(*message));
            }

            ExtensionPtr extension{};
            CheckResult(Base::FindController(message->controllerId, extension));
            std::atomic<uint32_t>& receiveCountPerController = _messageCountPerController[extension->controllerIndex];
            receiveCountPerController.fetch_sub(1);
            _totalReceiveCount--;

            callback(simulationTime, extension->info, Convert(*message));
        }

        return Result::Ok;
    }

private:
    uint32_t _totalReceiveCount{};
    std::atomic<uint32_t>* _messageCountPerController{};
    ShmRingBuffer<TMessage>* _messageBuffer{};

    SharedMemory _sharedMemory;
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
    BusBufferImpl() = default;
    [[nodiscard]] Result Initialize(CoSimType coSimType,
                                    [[maybe_unused]] ConnectionKind connectionKind,
                                    std::string_view name,
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

        std::string_view suffixForTransmit = coSimType == CoSimType::Client ? "Transmit" : "Receive";
        std::string_view suffixForReceive = coSimType == CoSimType::Client ? "Receive" : "Transmit";

        std::string canTransmitBufferName(name);
        canTransmitBufferName.append(".Can.");
        canTransmitBufferName.append(suffixForTransmit);
        std::string ethTransmitBufferName(name);
        ethTransmitBufferName.append(".Eth.");
        ethTransmitBufferName.append(suffixForTransmit);
        std::string linTransmitBufferName(name);
        linTransmitBufferName.append(".Lin.");
        linTransmitBufferName.append(suffixForTransmit);

        std::string canReceiveBufferName(name);
        canReceiveBufferName.append(".Can.");
        canReceiveBufferName.append(suffixForReceive);
        std::string ethReceiveBufferName(name);
        ethReceiveBufferName.append(".Eth.");
        ethReceiveBufferName.append(suffixForReceive);
        std::string linReceiveBufferName(name);
        linReceiveBufferName.append(".Lin.");
        linReceiveBufferName.append(suffixForReceive);

        CheckResult(_canTransmitBuffer->Initialize(coSimType, canTransmitBufferName, canControllers));
        CheckResult(_ethTransmitBuffer->Initialize(coSimType, ethTransmitBufferName, ethControllers));
        CheckResult(_linTransmitBuffer->Initialize(coSimType, linTransmitBufferName, linControllers));
        CheckResult(_canReceiveBuffer->Initialize(coSimType, canReceiveBufferName, canControllers));
        CheckResult(_ethReceiveBuffer->Initialize(coSimType, ethReceiveBufferName, ethControllers));
        CheckResult(_linReceiveBuffer->Initialize(coSimType, linReceiveBufferName, linControllers));
        return Result::Ok;
    }

    ~BusBufferImpl() override = default;

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

    [[nodiscard]] Result Transmit(const CanMessage& message) const override {
        return _canTransmitBuffer->Transmit(message);
    }

    [[nodiscard]] Result Transmit(const EthMessage& message) const override {
        return _ethTransmitBuffer->Transmit(message);
    }

    [[nodiscard]] Result Transmit(const LinMessage& message) const override {
        return _linTransmitBuffer->Transmit(message);
    }

    [[nodiscard]] Result Receive(CanMessage& message) const override {
        return _canReceiveBuffer->Receive(message);
    }

    [[nodiscard]] Result Receive(EthMessage& message) const override {
        return _ethReceiveBuffer->Receive(message);
    }

    [[nodiscard]] Result Receive(LinMessage& message) const override {
        return _linReceiveBuffer->Receive(message);
    }

    [[nodiscard]] Result Serialize(ChannelWriter& writer) const override {
        CheckResultWithMessage(_canTransmitBuffer->Serialize(writer), "Could not transmit CAN messages.");
        CheckResultWithMessage(_ethTransmitBuffer->Serialize(writer), "Could not transmit ETH messages.");
        CheckResultWithMessage(_linTransmitBuffer->Serialize(writer), "Could not transmit LIN messages.");
        return Result::Ok;
    }

    [[nodiscard]] Result Deserialize(ChannelReader& reader,
                                     SimulationTime simulationTime,
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
        return Result::Ok;
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

[[nodiscard]] Result CreateBusBuffer(CoSimType coSimType,
                                     ConnectionKind connectionKind,
                                     std::string_view name,
                                     const std::vector<CanController>& canControllers,
                                     const std::vector<EthController>& ethControllers,
                                     const std::vector<LinController>& linControllers,
                                     std::unique_ptr<BusBuffer>& busBuffer) {
    busBuffer = std::make_unique<BusBufferImpl>();
    CheckResult(dynamic_cast<BusBufferImpl&>(*busBuffer)
                    .Initialize(coSimType, connectionKind, name, canControllers, ethControllers, linControllers));
    return Result::Ok;
}

}  // namespace DsVeosCoSim
