// Copyright dSPACE GmbH. All rights reserved.

#include "BusBuffer.h"

#include <cstdint>
#include <cstring>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "Channel.h"
#include "CoSimHelper.h"
#include "DsVeosCoSim/CoSimTypes.h"
#include "Environment.h"
#include "Protocol.h"
#include "RingBuffer.h"

#ifdef _WIN32
#include <atomic>

#include "OsUtilities.h"
#endif

namespace DsVeosCoSim {

namespace {

template <typename TMessage, typename TMessageContainer, typename TController>
class BusProtocolBufferBase {
protected:
    using MessageCallback = std::function<void(SimulationTime, const TController&, const TMessage&)>;
    using MessageContainerCallback = std::function<void(SimulationTime, const TController&, const TMessageContainer&)>;

    struct ControllerExtension {
        TController info{};
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
                                    const std::string& name,
                                    const std::vector<TController>& controllers) {
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

    [[nodiscard]] Result Transmit(const TMessage& message) {
        if (_coSimType == CoSimType::Client) {
            std::lock_guard lock(_mutex);
            return TransmitInternal(message);
        }

        return TransmitInternal(message);
    }

    [[nodiscard]] Result Transmit(const TMessageContainer& messageContainer) {
        if (_coSimType == CoSimType::Client) {
            std::lock_guard lock(_mutex);
            return TransmitInternal(messageContainer);
        }

        return TransmitInternal(messageContainer);
    }

    [[nodiscard]] Result Receive(TMessage& message) {
        if (_coSimType == CoSimType::Client) {
            std::lock_guard lock(_mutex);
            return ReceiveInternal(message);
        }

        return ReceiveInternal(message);
    }

    [[nodiscard]] Result Receive(TMessageContainer& messageContainer) {
        if (_coSimType == CoSimType::Client) {
            std::lock_guard lock(_mutex);
            return ReceiveInternal(messageContainer);
        }

        return ReceiveInternal(messageContainer);
    }

    [[nodiscard]] Result Serialize(ChannelWriter& writer) {
        if (_coSimType == CoSimType::Client) {
            std::lock_guard lock(_mutex);
            return SerializeInternal(writer);
        }

        return SerializeInternal(writer);
    }

    [[nodiscard]] Result Deserialize(ChannelReader& reader,
                                     SimulationTime simulationTime,
                                     const MessageCallback& messageCallback,
                                     const MessageContainerCallback& messageContainerCallback) {
        if (_coSimType == CoSimType::Client) {
            std::lock_guard lock(_mutex);
            return DeserializeInternal(reader, simulationTime, messageCallback, messageContainerCallback);
        }

        return DeserializeInternal(reader, simulationTime, messageCallback, messageContainerCallback);
    }

protected:
    [[nodiscard]] virtual Result InitializeInternal(const std::string& name, size_t totalQueueItemsCountPerBuffer) = 0;

    virtual void ClearDataInternal() = 0;

    [[nodiscard]] virtual Result TransmitInternal(const TMessage& message) = 0;
    [[nodiscard]] virtual Result TransmitInternal(const TMessageContainer& messageContainer) = 0;

    [[nodiscard]] virtual Result ReceiveInternal(TMessage& message) = 0;
    [[nodiscard]] virtual Result ReceiveInternal(TMessageContainer& messageContainer) = 0;

    [[nodiscard]] virtual Result SerializeInternal(ChannelWriter& writer) = 0;
    [[nodiscard]] virtual Result DeserializeInternal(ChannelReader& reader,
                                                     SimulationTime simulationTime,
                                                     const MessageCallback& messageCallback,
                                                     const MessageContainerCallback& messageContainerCallback) = 0;

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

template <typename TMessage, typename TMessageContainer, typename TController>
class RemoteBusProtocolBuffer final : public BusProtocolBufferBase<TMessage, TMessageContainer, TController> {
    using Base = BusProtocolBufferBase<TMessage, TMessageContainer, TController>;
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
    [[nodiscard]] Result InitializeInternal([[maybe_unused]] const std::string& name,
                                            size_t totalQueueItemsCountPerBuffer) override {
        _messageCountPerController.resize(this->_controllers.size());
        _messageBuffer = RingBuffer<TMessageContainer>(totalQueueItemsCountPerBuffer);
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

    [[nodiscard]] Result CheckForSpace(ExtensionPtr extension) {
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

        return Result::Ok;
    }

    [[nodiscard]] Result TransmitInternal(const TMessage& message) override {
        CheckResult(message.Check());

        ExtensionPtr extension{};
        CheckResult(Base::FindController(message.controllerId, extension));
        CheckResult(CheckForSpace(extension));

        TMessageContainer& messageContainer = _messageBuffer.EmplaceBack();
        message.WriteTo(messageContainer);

        ++_messageCountPerController[extension->controllerIndex];
        return Result::Ok;
    }

    [[nodiscard]] Result TransmitInternal(const TMessageContainer& messageContainer) override {
        CheckResult(messageContainer.Check());

        ExtensionPtr extension{};
        CheckResult(Base::FindController(messageContainer.controllerId, extension));
        CheckResult(CheckForSpace(extension));

        _messageBuffer.PushBack(messageContainer);
        ++_messageCountPerController[extension->controllerIndex];
        return Result::Ok;
    }

    [[nodiscard]] Result ReceiveInternal(TMessage& message) override {
        if (_messageBuffer.IsEmpty()) {
            return Result::Empty;
        }

        TMessageContainer& messageContainer = _messageBuffer.PopFront();
        messageContainer.WriteTo(message);

        ExtensionPtr extension{};
        CheckResult(Base::FindController(messageContainer.controllerId, extension));
        --_messageCountPerController[extension->controllerIndex];
        return Result::Ok;
    }

    [[nodiscard]] Result ReceiveInternal(TMessageContainer& messageContainer) override {
        if (_messageBuffer.IsEmpty()) {
            return Result::Empty;
        }

        messageContainer = _messageBuffer.PopFront();

        ExtensionPtr extension{};
        CheckResult(Base::FindController(messageContainer.controllerId, extension));
        --_messageCountPerController[extension->controllerIndex];
        return Result::Ok;
    }

    [[nodiscard]] Result SerializeInternal(ChannelWriter& writer) override {
        size_t count = _messageBuffer.Size();
        CheckResultWithMessage(Protocol::WriteSize(writer, count), "Could not write count of messages.");

        for (size_t i = 0; i < count; i++) {
            TMessageContainer& messageContainer = _messageBuffer.PopFront();

            if (IsProtocolTracingEnabled()) {
                LogProtocolDataTrace(messageContainer.ToString());
            }

            CheckResultWithMessage(Protocol::WriteMessage(writer, messageContainer), "Could not serialize message.");
        }

        for (auto& [id, extension] : Base::_controllers) {
            _messageCountPerController[extension.controllerIndex] = 0;
        }

        return Result::Ok;
    }

    [[nodiscard]] Result DeserializeInternal(
        ChannelReader& reader,
        SimulationTime simulationTime,
        const typename Base::MessageCallback& messageCallback,
        const typename Base::MessageContainerCallback& messageContainerCallback) override {
        size_t totalCount{};
        CheckResultWithMessage(Protocol::ReadSize(reader, totalCount), "Could not read count of messages.");

        for (size_t i = 0; i < totalCount; i++) {
            TMessageContainer messageContainer{};
            CheckResultWithMessage(Protocol::ReadMessage(reader, messageContainer), "Could not deserialize message.");

            if (IsProtocolTracingEnabled()) {
                LogProtocolDataTrace(messageContainer.ToString());
            }

            ExtensionPtr extension{};
            CheckResult(Base::FindController(messageContainer.controllerId, extension));

            if (messageContainerCallback) {
                messageContainerCallback(simulationTime, extension->info, messageContainer);
                continue;
            }

            if (messageCallback) {
                TMessage message{};
                messageContainer.WriteTo(message);
                messageCallback(simulationTime, extension->info, message);
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
            _messageBuffer.PushBack(std::move(messageContainer));
        }

        return Result::Ok;
    }

private:
    std::vector<uint32_t> _messageCountPerController;

    RingBuffer<TMessageContainer> _messageBuffer;
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

    void PushBack(const T& item) {
        if (IsFull()) {
            throw std::runtime_error("SHM ring buffer is full.");
        }

        uint32_t currentWriteIndex = _writeIndex;
        _items[currentWriteIndex] = item;
        _writeIndex = (_writeIndex + 1) % _capacity;
        ++_size;
    }

    void PushBack(T&& item) {
        if (IsFull()) {
            throw std::runtime_error("SHM ring buffer is full.");
        }

        uint32_t currentWriteIndex = _writeIndex;
        _items[currentWriteIndex] = std::move(item);
        _writeIndex = (_writeIndex + 1) % _capacity;
        ++_size;
    }

    [[nodiscard]] T& EmplaceBack() {
        if (IsFull()) {
            throw std::runtime_error("SHM Ring buffer is full.");
        }

        size_t currentWriteIndex = _writeIndex;
        T& item = _items[currentWriteIndex];
        _writeIndex = (_writeIndex + 1) % _capacity;
        ++_size;
        return item;
    }

    [[nodiscard]] T& PopFront() {
        if (IsEmpty()) {
            throw std::runtime_error("SHM ring buffer is empty.");
        }

        --_size;
        T& item = _items[_readIndex];
        _readIndex = (_readIndex + 1) % _capacity;
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

template <typename TMessage, typename TMessageContainer, typename TController>
class LocalBusProtocolBuffer final : public BusProtocolBufferBase<TMessage, TMessageContainer, TController> {
    using Base = BusProtocolBufferBase<TMessage, TMessageContainer, TController>;
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
    [[nodiscard]] Result InitializeInternal(const std::string& name, size_t totalQueueItemsCountPerBuffer) override {
        // The memory layout looks like this:
        // [ list of message count per controller ]
        // [ message buffer ]

        size_t sizeOfMessageCountPerController = Base::_controllers.size() * sizeof(std::atomic<uint32_t>);
        size_t sizeOfRingBuffer =
            sizeof(ShmRingBuffer<TMessageContainer>) + (totalQueueItemsCountPerBuffer * sizeof(TMessageContainer));

        size_t sizeOfSharedMemory = 0;
        sizeOfSharedMemory += sizeOfMessageCountPerController;
        sizeOfSharedMemory += sizeOfRingBuffer;

        CheckResult(SharedMemory::CreateOrOpen(name, sizeOfSharedMemory, _sharedMemory));

        auto* pointerToMessageCountPerController = static_cast<uint8_t*>(_sharedMemory.GetData());
        auto* pointerToMessageBuffer = pointerToMessageCountPerController + sizeOfMessageCountPerController;

        _messageCountPerController = reinterpret_cast<std::atomic<uint32_t>*>(pointerToMessageCountPerController);
        _messageBuffer = reinterpret_cast<ShmRingBuffer<TMessageContainer>*>(pointerToMessageBuffer);

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
            _messageCountPerController[i].store(0, std::memory_order_release);
        }

        if (_messageBuffer) {
            _messageBuffer->Clear();
        }
    }

    [[nodiscard]] Result CheckForSpace(std::atomic<uint32_t>& messageCount, ExtensionPtr extension) {
        if (messageCount.load(std::memory_order_acquire) == extension->info.queueSize) {
            if (!extension->warningSent) {
                std::string warningMessage = "Transmit buffer for controller '";
                warningMessage.append(extension->info.name);
                warningMessage.append("' is full. Messages are dropped.");
                LogWarning(warningMessage);
                extension->warningSent = true;
            }

            return Result::Full;
        }

        return Result::Ok;
    }

    [[nodiscard]] Result TransmitInternal(const TMessage& message) override {
        CheckResult(message.Check());

        ExtensionPtr extension{};
        CheckResult(Base::FindController(message.controllerId, extension));
        std::atomic<uint32_t>& messageCount = _messageCountPerController[extension->controllerIndex];
        CheckResult(CheckForSpace(messageCount, extension));

        TMessageContainer& messageContainer = _messageBuffer->EmplaceBack();

        message.WriteTo(messageContainer);
        messageCount.fetch_add(1);
        return Result::Ok;
    }

    [[nodiscard]] Result TransmitInternal(const TMessageContainer& messageContainer) override {
        CheckResult(messageContainer.Check());

        ExtensionPtr extension{};
        CheckResult(Base::FindController(messageContainer.controllerId, extension));
        std::atomic<uint32_t>& messageCount = _messageCountPerController[extension->controllerIndex];
        CheckResult(CheckForSpace(messageCount, extension));

        _messageBuffer->PushBack(messageContainer);
        messageCount.fetch_add(1);
        return Result::Ok;
    }

    [[nodiscard]] Result ReceiveInternal(TMessage& message) override {
        if (_totalReceiveCount == 0) {
            return Result::Empty;
        }

        TMessageContainer& messageContainer = _messageBuffer->PopFront();
        messageContainer.WriteTo(message);

        ExtensionPtr extension{};
        CheckResult(Base::FindController(message.controllerId, extension));
        std::atomic<uint32_t>& receiveCount = _messageCountPerController[extension->controllerIndex];
        receiveCount.fetch_sub(1);
        _totalReceiveCount--;
        return Result::Ok;
    }

    [[nodiscard]] Result ReceiveInternal(TMessageContainer& messageContainer) override {
        if (_totalReceiveCount == 0) {
            return Result::Empty;
        }

        messageContainer = _messageBuffer->PopFront();

        ExtensionPtr extension{};
        CheckResult(Base::FindController(messageContainer.controllerId, extension));
        std::atomic<uint32_t>& receiveCount = _messageCountPerController[extension->controllerIndex];
        receiveCount.fetch_sub(1);
        _totalReceiveCount--;
        return Result::Ok;
    }

    [[nodiscard]] Result SerializeInternal(ChannelWriter& writer) override {
        CheckResultWithMessage(Protocol::WriteSize(writer, _messageBuffer->Size()), "Could not write transmit count.");
        return Result::Ok;
    }

    [[nodiscard]] Result DeserializeInternal(
        ChannelReader& reader,
        SimulationTime simulationTime,
        const typename Base::MessageCallback& messageCallback,
        const typename Base::MessageContainerCallback& messageContainerCallback) override {
        size_t receiveCount{};
        CheckResultWithMessage(Protocol::ReadSize(reader, receiveCount), "Could not read receive count.");
        _totalReceiveCount += receiveCount;

        if (!messageCallback && !messageContainerCallback) {
            return Result::Ok;
        }

        while (_totalReceiveCount > 0) {
            TMessageContainer& messageContainer = _messageBuffer->PopFront();

            if (IsProtocolTracingEnabled()) {
                LogProtocolDataTrace(messageContainer.ToString());
            }

            ExtensionPtr extension{};
            CheckResult(Base::FindController(messageContainer.controllerId, extension));
            std::atomic<uint32_t>& receiveCountPerController = _messageCountPerController[extension->controllerIndex];
            receiveCountPerController.fetch_sub(1);
            _totalReceiveCount--;

            if (messageContainerCallback) {
                messageContainerCallback(simulationTime, extension->info, messageContainer);
                continue;
            }

            if (messageCallback) {
                TMessage message{};
                messageContainer.WriteTo(message);
                messageCallback(simulationTime, extension->info, message);
                continue;
            }
        }

        return Result::Ok;
    }

private:
    size_t _totalReceiveCount{};
    std::atomic<uint32_t>* _messageCountPerController{};
    ShmRingBuffer<TMessageContainer>* _messageBuffer{};

    SharedMemory _sharedMemory;
};

#endif

#ifdef _WIN32
using LocalCanBuffer = LocalBusProtocolBuffer<CanMessage, CanMessageContainer, CanController>;
using LocalEthBuffer = LocalBusProtocolBuffer<EthMessage, EthMessageContainer, EthController>;
using LocalLinBuffer = LocalBusProtocolBuffer<LinMessage, LinMessageContainer, LinController>;
#endif

using RemoteCanBuffer = RemoteBusProtocolBuffer<CanMessage, CanMessageContainer, CanController>;
using RemoteEthBuffer = RemoteBusProtocolBuffer<EthMessage, EthMessageContainer, EthController>;
using RemoteLinBuffer = RemoteBusProtocolBuffer<LinMessage, LinMessageContainer, LinController>;

class BusBufferImpl final : public BusBuffer {
    using CanBufferBase = BusProtocolBufferBase<CanMessage, CanMessageContainer, CanController>;
    using EthBufferBase = BusProtocolBufferBase<EthMessage, EthMessageContainer, EthController>;
    using LinBufferBase = BusProtocolBufferBase<LinMessage, LinMessageContainer, LinController>;

public:
    BusBufferImpl() = default;
    [[nodiscard]] Result Initialize(CoSimType coSimType,
                                    [[maybe_unused]] ConnectionKind connectionKind,
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

        const char* suffixForTransmit = coSimType == CoSimType::Client ? "Transmit" : "Receive";
        const char* suffixForReceive = coSimType == CoSimType::Client ? "Receive" : "Transmit";

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

    [[nodiscard]] Result Transmit(const CanMessageContainer& message) const override {
        return _canTransmitBuffer->Transmit(message);
    }

    [[nodiscard]] Result Transmit(const EthMessageContainer& message) const override {
        return _ethTransmitBuffer->Transmit(message);
    }

    [[nodiscard]] Result Transmit(const LinMessageContainer& message) const override {
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

    [[nodiscard]] Result Receive(CanMessageContainer& message) const override {
        return _canReceiveBuffer->Receive(message);
    }

    [[nodiscard]] Result Receive(EthMessageContainer& message) const override {
        return _ethReceiveBuffer->Receive(message);
    }

    [[nodiscard]] Result Receive(LinMessageContainer& message) const override {
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
        CheckResultWithMessage(_canReceiveBuffer->Deserialize(reader,
                                                              simulationTime,
                                                              callbacks.canMessageReceivedCallback,
                                                              callbacks.canMessageContainerReceivedCallback),
                               "Could not receive CAN messages.");
        CheckResultWithMessage(_ethReceiveBuffer->Deserialize(reader,
                                                              simulationTime,
                                                              callbacks.ethMessageReceivedCallback,
                                                              callbacks.ethMessageContainerReceivedCallback),
                               "Could not receive ETH messages.");
        CheckResultWithMessage(_linReceiveBuffer->Deserialize(reader,
                                                              simulationTime,
                                                              callbacks.linMessageReceivedCallback,
                                                              callbacks.linMessageContainerReceivedCallback),
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
                                     const std::string& name,
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
