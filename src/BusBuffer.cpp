// Copyright dSPACE SE & Co. KG. All rights reserved.

#include "BusBuffer.hpp"

#include <cstdint>
#include <cstring>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

#include <fmt/format.h>

#include "Channel.hpp"
#include "CoSimTypes.hpp"
#include "Environment.hpp"
#include "Protocol.hpp"
#include "ProtocolLogger.hpp"
#include "RingBuffer.hpp"

#ifdef _WIN32

#include <atomic>

#include "OsUtilities.hpp"
#include "RingBufferView.hpp"

#endif

namespace DsVeosCoSim {

namespace {

struct CanBus {
    using Message = CanMessage;
    using MessageContainer = CanMessageContainer;
    using Controller = CanController;
#ifdef _WIN32
    static constexpr const char* ShmNamePart = ".Can.";
#endif
    static constexpr const char* DisplayName = "CAN";
    static constexpr uint32_t MessageMaxLength = CanMessageMaxLength;
};

struct EthBus {
    using Message = EthMessage;
    using MessageContainer = EthMessageContainer;
    using Controller = EthController;
#ifdef _WIN32
    static constexpr const char* ShmNamePart = ".Eth.";
#endif
    static constexpr const char* DisplayName = "ETH";
    static constexpr uint32_t MessageMaxLength = EthMessageMaxLength;
};

struct LinBus {
    using Message = LinMessage;
    using MessageContainer = LinMessageContainer;
    using Controller = LinController;
#ifdef _WIN32
    static constexpr const char* ShmNamePart = ".Lin.";
#endif
    static constexpr const char* DisplayName = "LIN";
    static constexpr uint32_t MessageMaxLength = LinMessageMaxLength;
};

struct FrBus {
    using Message = FrMessage;
    using MessageContainer = FrMessageContainer;
    using Controller = FrController;
#ifdef _WIN32
    static constexpr const char* ShmNamePart = ".Fr.";
#endif
    static constexpr const char* DisplayName = "FlexRay";
    static constexpr uint32_t MessageMaxLength = FrMessageMaxLength;
};

template <typename TBus>
class Bus final {
public:
    using TMessage = typename TBus::Message;
    using TMessageContainer = typename TBus::MessageContainer;
    using TController = typename TBus::Controller;
    using TMessageCallback = std::function<void(SimulationTime, const TController&, const TMessage&)>;
    using TMessageContainerCallback = std::function<void(SimulationTime, const TController&, const TMessageContainer&)>;

    struct ControllerExtension {
        TController info{};
        size_t controllerIndex{};
        bool receiveWarningSent{};
        bool transmitWarningSent{};

        void ClearData() {
            receiveWarningSent = false;
            transmitWarningSent = false;
        }
    };

    using ControllerExtensionPtr = ControllerExtension*;

    class Data final {
    public:
        Data() = default;
        ~Data() noexcept = default;

        Data(const Data&) = delete;
        Data& operator=(const Data&) = delete;

        Data(Data&&) = delete;
        Data& operator=(Data&&) = delete;

        [[nodiscard]] Result Initialize(const std::vector<TController>& controllers) {
            size_t nextControllerIndex = 0;
            for (const auto& controller : controllers) {
                auto search = _controllers.find(controller.id);
                if (search != _controllers.end()) {
                    LogError("Duplicated controller id {}.", controller.id);
                    return CreateError();
                }

                ControllerExtension extension{};
                extension.info = controller;
                extension.controllerIndex = nextControllerIndex++;
                _controllers[controller.id] = extension;
                _totalQueueItemsCountPerBuffer += controller.queueSize;
            }

            return CreateOk();
        }

        void ClearData() {
            for (auto& [controllerId, dataPerController] : _controllers) {
                dataPerController.ClearData();
            }
        }

        [[nodiscard]] Result FindController(BusControllerId controllerId, ControllerExtensionPtr& extensionPtr) {
            auto search = _controllers.find(controllerId);
            if (search != _controllers.end()) {
                extensionPtr = &search->second;
                return CreateOk();
            }

            LogError("Controller id {} is unknown.", controllerId);
            return CreateError();
        }

        [[nodiscard]] std::unordered_map<BusControllerId, ControllerExtension>& GetAllControllers() {
            return _controllers;
        }

        [[nodiscard]] size_t GetTotalQueueItemsCountPerBuffer() const {
            return _totalQueueItemsCountPerBuffer;
        }

    private:
        size_t _totalQueueItemsCountPerBuffer{};
        std::unordered_map<BusControllerId, ControllerExtension> _controllers;
    };

    class IPart {
    public:
        IPart() = default;
        virtual ~IPart() noexcept = default;

        IPart(const IPart&) = delete;
        IPart& operator=(const IPart&) = delete;

        IPart(IPart&&) = delete;
        IPart& operator=(IPart&&) = delete;

        [[nodiscard]] virtual Result Initialize(const std::vector<TController>& controllers) = 0;
        virtual void ClearData() = 0;
        [[nodiscard]] virtual Result Transmit(const TMessage& message) = 0;
        [[nodiscard]] virtual Result Transmit(const TMessageContainer& messageContainer) = 0;
        [[nodiscard]] virtual Result Receive(TMessage& message) = 0;
        [[nodiscard]] virtual Result Receive(TMessageContainer& messageContainer) = 0;
        [[nodiscard]] virtual Result Serialize(ChannelWriter& writer) = 0;
        [[nodiscard]] virtual Result Deserialize(ChannelReader& reader,
                                                 SimulationTime simulationTime,
                                                 const TMessageCallback& messageCallback,
                                                 const TMessageContainerCallback& messageContainerCallback) = 0;
    };

    class LockedPartImpl final : public IPart {
    public:
        explicit LockedPartImpl(std::unique_ptr<IPart> proxiedPart) : _proxiedPart(std::move(proxiedPart)) {
        }

        ~LockedPartImpl() noexcept override = default;

        LockedPartImpl(const LockedPartImpl&) = delete;
        LockedPartImpl& operator=(const LockedPartImpl&) = delete;

        LockedPartImpl(LockedPartImpl&&) = delete;
        LockedPartImpl& operator=(LockedPartImpl&&) = delete;

        [[nodiscard]] Result Initialize(const std::vector<TController>& controllers) override {
            std::lock_guard lock(_mutex);
            return _proxiedPart->Initialize(controllers);
        }

        void ClearData() override {
            std::lock_guard lock(_mutex);
            _proxiedPart->ClearData();
        }

        [[nodiscard]] Result Transmit(const TMessage& message) override {
            std::lock_guard lock(_mutex);
            return _proxiedPart->Transmit(message);
        }

        [[nodiscard]] Result Transmit(const TMessageContainer& messageContainer) override {
            std::lock_guard lock(_mutex);
            return _proxiedPart->Transmit(messageContainer);
        }

        [[nodiscard]] Result Receive(TMessage& message) override {
            std::lock_guard lock(_mutex);
            return _proxiedPart->Receive(message);
        }

        [[nodiscard]] Result Receive(TMessageContainer& messageContainer) override {
            std::lock_guard lock(_mutex);
            return _proxiedPart->Receive(messageContainer);
        }

        [[nodiscard]] Result Serialize(ChannelWriter& writer) override {
            std::lock_guard lock(_mutex);
            return _proxiedPart->Serialize(writer);
        }

        [[nodiscard]] Result Deserialize(ChannelReader& reader,
                                         SimulationTime simulationTime,
                                         const TMessageCallback& messageCallback,
                                         const TMessageContainerCallback& messageContainerCallback) override {
            std::lock_guard lock(_mutex);
            return _proxiedPart->Deserialize(reader, simulationTime, messageCallback, messageContainerCallback);
        }

    private:
        std::unique_ptr<IPart> _proxiedPart;
        std::mutex _mutex;
    };

    class RemotePartImpl final : public IPart {
    public:
        explicit RemotePartImpl(IProtocol& protocol) : _protocol(protocol) {};
        ~RemotePartImpl() noexcept override = default;

        RemotePartImpl(const RemotePartImpl&) = delete;
        RemotePartImpl& operator=(const RemotePartImpl&) = delete;

        RemotePartImpl(RemotePartImpl&&) = delete;
        RemotePartImpl& operator=(RemotePartImpl&&) = delete;

        [[nodiscard]] Result Initialize(const std::vector<TController>& controllers) override {
            CheckResult(_data.Initialize(controllers));
            size_t totalQueueItemsCountPerBuffer = _data.GetTotalQueueItemsCountPerBuffer();
            _messageCountPerController.resize(_data.GetAllControllers().size());
            _messageBuffer = RingBuffer<TMessageContainer>(totalQueueItemsCountPerBuffer);
            return CreateOk();
        }

        void ClearData() override {
            _data.ClearData();

            for (auto& messageCount : _messageCountPerController) {
                messageCount = 0;
            }

            _messageBuffer.Clear();
        }

        [[nodiscard]] Result Transmit(const TMessage& message) override {
            ControllerExtensionPtr extensionPtr{};
            CheckResult(_data.FindController(message.controllerId, extensionPtr));
            CheckResult(CheckForSpace(*extensionPtr));

            TMessageContainer messageContainer{};
            message.WriteTo(messageContainer);
            if (!_messageBuffer.TryPushBack(std::move(messageContainer))) {
                LogError("Message buffer is full.");
                return CreateError();
            }

            ++_messageCountPerController[extensionPtr->controllerIndex];
            return CreateOk();
        }

        [[nodiscard]] Result Transmit(const TMessageContainer& messageContainer) override {
            ControllerExtensionPtr extensionPtr{};
            CheckResult(_data.FindController(messageContainer.controllerId, extensionPtr));
            CheckResult(CheckForSpace(*extensionPtr));

            if (!_messageBuffer.TryPushBack(messageContainer)) {
                LogError("Message buffer is full.");
                return CreateError();
            }

            ++_messageCountPerController[extensionPtr->controllerIndex];
            return CreateOk();
        }

        // Using the same TryPopFront function as in Receive(TMessageContainer& messageContainer)
        // would move the data to a local messageContainer. After leaving this function, message.data
        // would be a dangling pointer.
        // Solution:
        // - message.data points to the data inside of the buffer
        // - _messageBuffer.RemoveFront() only advances the read index. The data is still there until it is overwritten eventually.
        // - -> The CoSim client is responsible of copying the data after receiving the message
        [[nodiscard]] Result Receive(TMessage& message) override {
            TMessageContainer* messageContainer = _messageBuffer.TryPeekFront();
            if (messageContainer == nullptr) {
                return CreateEmpty();
            }

            messageContainer->WriteTo(message);

            // As mentioned above, this does not remove the message container
            _messageBuffer.RemoveFront();

            ControllerExtensionPtr extensionPtr{};
            CheckResult(_data.FindController(messageContainer->controllerId, extensionPtr));
            --_messageCountPerController[extensionPtr->controllerIndex];
            return CreateOk();
        }

        [[nodiscard]] Result Receive(TMessageContainer& messageContainer) override {
            if (!_messageBuffer.TryPopFront(messageContainer)) {
                return CreateEmpty();
            }

            ControllerExtensionPtr extensionPtr{};
            CheckResult(_data.FindController(messageContainer.controllerId, extensionPtr));
            --_messageCountPerController[extensionPtr->controllerIndex];
            return CreateOk();
        }

        [[nodiscard]] Result Serialize(ChannelWriter& writer) override {
            size_t count = _messageBuffer.Size();
            CheckResultWithMessage(_protocol.WriteSize(writer, count), "Could not write count of messages.");

            TMessageContainer messageContainer{};
            while (_messageBuffer.TryPopFront(messageContainer)) {
                if (IsProtocolTracingEnabled()) {
                    LogProtocolDataTrace(format_as(messageContainer));
                }

                CheckResultWithMessage(_protocol.WriteMessage(writer, messageContainer), "Could not serialize message.");
            }

            for (auto& [controllerId, extension] : _data.GetAllControllers()) {
                _messageCountPerController[extension.controllerIndex] = 0;
            }

            return CreateOk();
        }

        [[nodiscard]] Result Deserialize(ChannelReader& reader,
                                         SimulationTime simulationTime,
                                         const TMessageCallback& messageCallback,
                                         const TMessageContainerCallback& messageContainerCallback) override {
            size_t totalCount{};
            CheckResultWithMessage(_protocol.ReadSize(reader, totalCount), "Could not read count of messages.");

            for (size_t i = 0; i < totalCount; i++) {
                TMessageContainer messageContainer{};
                CheckResultWithMessage(_protocol.ReadMessage(reader, messageContainer), "Could not deserialize message.");

                if (IsProtocolTracingEnabled()) {
                    LogProtocolDataTrace(format_as(messageContainer));
                }

                ControllerExtensionPtr extensionPtr{};
                CheckResult(_data.FindController(messageContainer.controllerId, extensionPtr));

                if (messageContainerCallback) {
                    messageContainerCallback(simulationTime, extensionPtr->info, messageContainer);
                    continue;
                }

                if (messageCallback) {
                    TMessage message{};
                    messageContainer.WriteTo(message);
                    messageCallback(simulationTime, extensionPtr->info, message);
                    continue;
                }

                if (_messageCountPerController[extensionPtr->controllerIndex] == extensionPtr->info.queueSize) {
                    if (!extensionPtr->receiveWarningSent) {
                        LogWarning("Receive buffer for controller '{}' is full. Messages are dropped.", extensionPtr->info.name);
                        extensionPtr->receiveWarningSent = true;
                    }

                    continue;
                }

                ++_messageCountPerController[extensionPtr->controllerIndex];
                if (!_messageBuffer.TryPushBack(std::move(messageContainer))) {
                    LogError("Message buffer is full.");
                    return CreateError();
                }
            }

            return CreateOk();
        }

    private:
        [[nodiscard]] Result CheckForSpace(ControllerExtension& extension) {
            if (_messageCountPerController[extension.controllerIndex] == extension.info.queueSize) {
                if (!extension.transmitWarningSent) {
                    LogWarning("Transmit buffer for controller '{}' is full. Messages are dropped.", extension.info.name);
                    extension.transmitWarningSent = true;
                }

                return CreateFull();
            }

            return CreateOk();
        }

        IProtocol& _protocol;
        Data _data;
        std::vector<uint32_t> _messageCountPerController;
        RingBuffer<TMessageContainer> _messageBuffer;
    };

#ifdef _WIN32

    class LocalPartImpl final : public IPart {
    public:
        LocalPartImpl(IProtocol& protocol, std::string name) : _protocol(protocol), _name(std::move(name)) {};
        ~LocalPartImpl() noexcept override = default;

        LocalPartImpl(const LocalPartImpl&) = delete;
        LocalPartImpl& operator=(const LocalPartImpl&) = delete;

        LocalPartImpl(LocalPartImpl&&) = delete;
        LocalPartImpl& operator=(LocalPartImpl&&) = delete;

        [[nodiscard]] Result Initialize(const std::vector<TController>& controllers) override {
            // The memory layout looks like this:
            // [ list of message count per controller ]
            // [ message buffer ]

            CheckResult(_data.Initialize(controllers));

            size_t totalQueueItemsCountPerBuffer = _data.GetTotalQueueItemsCountPerBuffer();
            size_t sizeOfMessageCountPerController = _data.GetAllControllers().size() * sizeof(std::atomic<uint32_t>);
            size_t sizeOfRingBuffer = sizeof(RingBufferView<TMessageContainer>) + (totalQueueItemsCountPerBuffer * sizeof(TMessageContainer));

            size_t sizeOfSharedMemory = 0;
            sizeOfSharedMemory += sizeOfMessageCountPerController;
            sizeOfSharedMemory += sizeOfRingBuffer;

            CheckResult(SharedMemory::CreateOrOpen(_name, sizeOfSharedMemory, _sharedMemory));

            auto* pointerToMessageCountPerController = _sharedMemory.GetData();
            auto* pointerToMessageBuffer = pointerToMessageCountPerController + sizeOfMessageCountPerController;

            _messageCountPerController = reinterpret_cast<std::atomic<uint32_t>*>(pointerToMessageCountPerController);
            _messageBuffer = reinterpret_cast<RingBufferView<TMessageContainer>*>(pointerToMessageBuffer);

            _messageBuffer->Initialize(static_cast<uint32_t>(totalQueueItemsCountPerBuffer));

            ClearData();
            return CreateOk();
        }

        void ClearData() override {
            _data.ClearData();

            _totalReceiveCount = 0;
            _totalTransmitCount = 0;

            for (size_t i = 0; i < _data.GetAllControllers().size(); i++) {
                _messageCountPerController[i].store(0, std::memory_order_release);
            }

            if (_messageBuffer) {
                _messageBuffer->Clear();
            }
        }

        [[nodiscard]] Result Transmit(const TMessage& message) override {
            ControllerExtensionPtr extensionPtr{};
            CheckResult(_data.FindController(message.controllerId, extensionPtr));
            std::atomic<uint32_t>& messageCount = _messageCountPerController[extensionPtr->controllerIndex];
            CheckResult(CheckForSpace(messageCount, *extensionPtr));

            TMessageContainer messageContainer{};
            message.WriteTo(messageContainer);
            _messageBuffer->PushBack(messageContainer);

            messageCount.fetch_add(1);
            _totalTransmitCount++;
            return CreateOk();
        }

        [[nodiscard]] Result Transmit(const TMessageContainer& messageContainer) override {
            ControllerExtensionPtr extensionPtr{};
            CheckResult(_data.FindController(messageContainer.controllerId, extensionPtr));
            std::atomic<uint32_t>& messageCount = _messageCountPerController[extensionPtr->controllerIndex];
            CheckResult(CheckForSpace(messageCount, *extensionPtr));

            _messageBuffer->PushBack(messageContainer);

            messageCount.fetch_add(1);
            _totalTransmitCount++;
            return CreateOk();
        }

        [[nodiscard]] Result Receive(TMessage& message) override {
            if (_totalReceiveCount == 0) {
                return CreateEmpty();
            }

            TMessageContainer& messageContainer = _messageBuffer->PopFront();
            messageContainer.WriteTo(message);

            ControllerExtensionPtr extensionPtr{};
            CheckResult(_data.FindController(message.controllerId, extensionPtr));
            std::atomic<uint32_t>& receiveCount = _messageCountPerController[extensionPtr->controllerIndex];
            receiveCount.fetch_sub(1);
            _totalReceiveCount--;
            return CreateOk();
        }

        [[nodiscard]] Result Receive(TMessageContainer& messageContainer) override {
            if (_totalReceiveCount == 0) {
                return CreateEmpty();
            }

            messageContainer = _messageBuffer->PopFront();

            ControllerExtensionPtr extensionPtr{};
            CheckResult(_data.FindController(messageContainer.controllerId, extensionPtr));
            std::atomic<uint32_t>& receiveCount = _messageCountPerController[extensionPtr->controllerIndex];
            receiveCount.fetch_sub(1);
            _totalReceiveCount--;
            return CreateOk();
        }

        [[nodiscard]] Result Serialize(ChannelWriter& writer) override {
            CheckResultWithMessage(_protocol.WriteSize(writer, _totalTransmitCount), "Could not write transmit count.");
            _totalTransmitCount = 0;
            return CreateOk();
        }

        [[nodiscard]] Result Deserialize(ChannelReader& reader,
                                         SimulationTime simulationTime,
                                         const TMessageCallback& messageCallback,
                                         const TMessageContainerCallback& messageContainerCallback) override {
            size_t receiveCount{};
            CheckResultWithMessage(_protocol.ReadSize(reader, receiveCount), "Could not read receive count.");
            _totalReceiveCount += receiveCount;

            if (!messageCallback && !messageContainerCallback) {
                return CreateOk();
            }

            while (_totalReceiveCount > 0) {
                TMessageContainer& messageContainer = _messageBuffer->PopFront();

                if (IsProtocolTracingEnabled()) {
                    LogProtocolDataTrace(format_as(messageContainer));
                }

                ControllerExtensionPtr extensionPtr{};
                CheckResult(_data.FindController(messageContainer.controllerId, extensionPtr));
                std::atomic<uint32_t>& receiveCountPerController = _messageCountPerController[extensionPtr->controllerIndex];
                receiveCountPerController.fetch_sub(1);
                _totalReceiveCount--;

                if (messageContainerCallback) {
                    messageContainerCallback(simulationTime, extensionPtr->info, messageContainer);
                    continue;
                }

                if (messageCallback) {
                    TMessage message{};
                    messageContainer.WriteTo(message);
                    messageCallback(simulationTime, extensionPtr->info, message);
                }
            }

            return CreateOk();
        }

    private:
        [[nodiscard]] static Result CheckForSpace(const std::atomic<uint32_t>& messageCount, ControllerExtension& extension) {
            if (messageCount.load(std::memory_order_acquire) == extension.info.queueSize) {
                if (!extension.transmitWarningSent) {
                    LogWarning("Transmit buffer for controller '{}' is full. Messages are dropped.", extension.info.name);
                    extension.transmitWarningSent = true;
                }

                return CreateFull();
            }

            return CreateOk();
        }

        IProtocol& _protocol;
        std::string _name;
        Data _data;
        size_t _totalReceiveCount{};
        size_t _totalTransmitCount{};
        std::atomic<uint32_t>* _messageCountPerController{};
        RingBufferView<TMessageContainer>* _messageBuffer{};
        SharedMemory _sharedMemory;
    };
#endif

    class SpecificBus final {
    public:
        SpecificBus() = default;
        ~SpecificBus() noexcept = default;

        SpecificBus(const SpecificBus&) = delete;
        SpecificBus& operator=(const SpecificBus&) = delete;

        SpecificBus(SpecificBus&&) = delete;
        SpecificBus& operator=(SpecificBus&&) = delete;

        [[nodiscard]] Result Initialize(CoSimType coSimType,
                                        [[maybe_unused]] ConnectionKind connectionKind,
                                        [[maybe_unused]] const std::string& name,
                                        const std::vector<TController>& controllers,
                                        IProtocol& protocol) {
#ifdef _WIN32
            if (connectionKind == ConnectionKind::Local) {
                const char* suffixForTransmit = coSimType == CoSimType::Client ? "Transmit" : "Receive";
                const char* suffixForReceive = coSimType == CoSimType::Client ? "Receive" : "Transmit";

                std::string transmitBufferName = fmt::format("{}{}{}", name, TBus::ShmNamePart, suffixForTransmit);
                std::string receiveBufferName = fmt::format("{}{}{}", name, TBus::ShmNamePart, suffixForReceive);

                _transmitBuffer = std::make_unique<LocalPartImpl>(protocol, transmitBufferName);
                _receiveBuffer = std::make_unique<LocalPartImpl>(protocol, receiveBufferName);
            } else {
#endif
                _transmitBuffer = std::make_unique<RemotePartImpl>(protocol);
                _receiveBuffer = std::make_unique<RemotePartImpl>(protocol);
#ifdef _WIN32
            }
#endif
            if (coSimType == CoSimType::Client) {
                _transmitBuffer = std::make_unique<LockedPartImpl>(std::move(_transmitBuffer));
                _receiveBuffer = std::make_unique<LockedPartImpl>(std::move(_receiveBuffer));
            }

            CheckResult(_transmitBuffer->Initialize(controllers));
            CheckResult(_receiveBuffer->Initialize(controllers));

            return CreateOk();
        }

        void ClearData() const {
            _transmitBuffer->ClearData();
            _receiveBuffer->ClearData();
        }

        [[nodiscard]] Result Transmit(const TMessage& message) const {
            CheckResult(CheckMessageLength(message.length));

            return _transmitBuffer->Transmit(message);
        }

        [[nodiscard]] Result Transmit(const TMessageContainer& messageContainer) const {
            CheckResult(CheckMessageLength(messageContainer.length));

            return _transmitBuffer->Transmit(messageContainer);
        }

        [[nodiscard]] Result Receive(TMessage& message) const {
            return _receiveBuffer->Receive(message);
        }

        [[nodiscard]] Result Receive(TMessageContainer& messageContainer) const {
            return _receiveBuffer->Receive(messageContainer);
        }

        [[nodiscard]] Result Serialize(ChannelWriter& writer) const {
            return _transmitBuffer->Serialize(writer);
        }

        [[nodiscard]] Result Deserialize(ChannelReader& reader,
                                         SimulationTime simulationTime,
                                         const TMessageCallback& messageCallback,
                                         const TMessageContainerCallback& messageContainerCallback) const {
            return _receiveBuffer->Deserialize(reader, simulationTime, messageCallback, messageContainerCallback);
        }

    private:
        [[nodiscard]] static Result CheckMessageLength(uint32_t length) {
            if (length > TBus::MessageMaxLength) {
                LogError("{} message data exceeds maximum length.", TBus::DisplayName);
                return CreateInvalidArgument();
            }

            return CreateOk();
        }

        std::unique_ptr<IPart> _transmitBuffer;
        std::unique_ptr<IPart> _receiveBuffer;
    };
};

using CanBuffer = Bus<CanBus>::SpecificBus;
using EthBuffer = Bus<EthBus>::SpecificBus;
using LinBuffer = Bus<LinBus>::SpecificBus;
using FrBuffer = Bus<FrBus>::SpecificBus;

class BusBufferImpl final : public BusBuffer {
public:
    BusBufferImpl() = default;

    [[nodiscard]] Result Initialize(CoSimType coSimType,
                                    ConnectionKind connectionKind,
                                    const std::string& name,
                                    const std::vector<CanController>& canControllers,
                                    const std::vector<EthController>& ethControllers,
                                    const std::vector<LinController>& linControllers,
                                    const std::vector<FrController>& frControllers,
                                    IProtocol& protocol) {
        _doFlexrayOperations = protocol.DoFlexRayOperations();

        _canBuffer = std::make_unique<CanBuffer>();
        _ethBuffer = std::make_unique<EthBuffer>();
        _linBuffer = std::make_unique<LinBuffer>();
        _frBuffer = std::make_unique<FrBuffer>();

        CheckResult(_canBuffer->Initialize(coSimType, connectionKind, name, canControllers, protocol));
        CheckResult(_ethBuffer->Initialize(coSimType, connectionKind, name, ethControllers, protocol));
        CheckResult(_linBuffer->Initialize(coSimType, connectionKind, name, linControllers, protocol));
        CheckResult(_frBuffer->Initialize(coSimType, connectionKind, name, frControllers, protocol));

        return CreateOk();
    }

    ~BusBufferImpl() override = default;

    BusBufferImpl(const BusBufferImpl&) = delete;
    BusBufferImpl& operator=(const BusBufferImpl&) = delete;

    BusBufferImpl(BusBufferImpl&&) = delete;
    BusBufferImpl& operator=(BusBufferImpl&&) = delete;

    void ClearData() const override {
        _canBuffer->ClearData();
        _ethBuffer->ClearData();
        _linBuffer->ClearData();
        _frBuffer->ClearData();
    }

    [[nodiscard]] Result Transmit(const CanMessage& message) const override {
        return _canBuffer->Transmit(message);
    }

    [[nodiscard]] Result Transmit(const EthMessage& message) const override {
        return _ethBuffer->Transmit(message);
    }

    [[nodiscard]] Result Transmit(const LinMessage& message) const override {
        return _linBuffer->Transmit(message);
    }

    [[nodiscard]] Result Transmit(const FrMessage& message) const override {
        return _frBuffer->Transmit(message);
    }

    [[nodiscard]] Result Transmit(const CanMessageContainer& messageContainer) const override {
        return _canBuffer->Transmit(messageContainer);
    }

    [[nodiscard]] Result Transmit(const EthMessageContainer& messageContainer) const override {
        return _ethBuffer->Transmit(messageContainer);
    }

    [[nodiscard]] Result Transmit(const LinMessageContainer& messageContainer) const override {
        return _linBuffer->Transmit(messageContainer);
    }

    [[nodiscard]] Result Transmit(const FrMessageContainer& messageContainer) const override {
        return _frBuffer->Transmit(messageContainer);
    }

    [[nodiscard]] Result Receive(CanMessage& message) const override {
        return _canBuffer->Receive(message);
    }

    [[nodiscard]] Result Receive(EthMessage& message) const override {
        return _ethBuffer->Receive(message);
    }

    [[nodiscard]] Result Receive(LinMessage& message) const override {
        return _linBuffer->Receive(message);
    }

    [[nodiscard]] Result Receive(FrMessage& message) const override {
        return _frBuffer->Receive(message);
    }

    [[nodiscard]] Result Receive(CanMessageContainer& messageContainer) const override {
        return _canBuffer->Receive(messageContainer);
    }

    [[nodiscard]] Result Receive(EthMessageContainer& messageContainer) const override {
        return _ethBuffer->Receive(messageContainer);
    }

    [[nodiscard]] Result Receive(LinMessageContainer& messageContainer) const override {
        return _linBuffer->Receive(messageContainer);
    }

    [[nodiscard]] Result Receive(FrMessageContainer& messageContainer) const override {
        return _frBuffer->Receive(messageContainer);
    }

    [[nodiscard]] Result Serialize(ChannelWriter& writer) const override {
        CheckResultWithMessage(_canBuffer->Serialize(writer), "Could not transmit CAN messages.");
        CheckResultWithMessage(_ethBuffer->Serialize(writer), "Could not transmit Ethernet messages.");
        CheckResultWithMessage(_linBuffer->Serialize(writer), "Could not transmit LIN messages.");

        if (_doFlexrayOperations) {
            CheckResultWithMessage(_frBuffer->Serialize(writer), "Could not transmit FlexRay messages.");
        }

        return CreateOk();
    }

    [[nodiscard]] Result Deserialize(ChannelReader& reader, SimulationTime simulationTime, const Callbacks& callbacks) const override {
        CheckResultWithMessage(
            _canBuffer->Deserialize(reader, simulationTime, callbacks.canMessageReceivedCallback, callbacks.canMessageContainerReceivedCallback),
            "Could not receive CAN messages.");
        CheckResultWithMessage(
            _ethBuffer->Deserialize(reader, simulationTime, callbacks.ethMessageReceivedCallback, callbacks.ethMessageContainerReceivedCallback),
            "Could not receive Ethernet messages.");
        CheckResultWithMessage(
            _linBuffer->Deserialize(reader, simulationTime, callbacks.linMessageReceivedCallback, callbacks.linMessageContainerReceivedCallback),
            "Could not receive LIN messages.");

        if (_doFlexrayOperations) {
            CheckResultWithMessage(
                _frBuffer->Deserialize(reader, simulationTime, callbacks.frMessageReceivedCallback, callbacks.frMessageContainerReceivedCallback),
                "Could not receive FlexRay messages.");
        }

        return CreateOk();
    }

private:
    std::unique_ptr<CanBuffer> _canBuffer;
    std::unique_ptr<EthBuffer> _ethBuffer;
    std::unique_ptr<LinBuffer> _linBuffer;
    std::unique_ptr<FrBuffer> _frBuffer;

    bool _doFlexrayOperations{};
};

}  // namespace

[[nodiscard]] Result CreateBusBuffer(CoSimType coSimType,
                                     ConnectionKind connectionKind,
                                     const std::string& name,
                                     const std::vector<CanController>& canControllers,
                                     const std::vector<EthController>& ethControllers,
                                     const std::vector<LinController>& linControllers,
                                     const std::vector<FrController>& frControllers,
                                     IProtocol& protocol,
                                     std::unique_ptr<BusBuffer>& busBuffer) {
    auto busBufferTmp = std::make_unique<BusBufferImpl>();
    CheckResult(busBufferTmp->Initialize(coSimType, connectionKind, name, canControllers, ethControllers, linControllers, frControllers, protocol));
    busBuffer = std::move(busBufferTmp);
    return CreateOk();
}

}  // namespace DsVeosCoSim
