// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "Channel.h"
#include "CoSimHelper.h"
#include "CoSimTypes.h"
#include "Environment.h"
#include "RingBuffer.h"

#ifdef _WIN32
#include <atomic>

#include "SharedMemory.h"
#include "ShmRingBuffer.h"
#endif

namespace DsVeosCoSim {

struct CanMessageContainer {
    SimulationTime timestamp{};
    BusControllerId controllerId{};
    uint32_t controllerIndex{};
    BusMessageId id{};
    CanMessageFlags flags{};
    uint32_t length{};
    std::array<uint8_t, CanMessageMaxLength> data{};

    [[nodiscard]] bool SerializeTo(ChannelWriter& writer) const;
    [[nodiscard]] bool DeserializeFrom(ChannelReader& reader);
    void WriteTo(CanMessage& message) const;
    void ReadFrom(const CanMessage& message);

    [[nodiscard]] explicit operator CanMessage() const;

    [[nodiscard]] std::string ToString() const;

private:
    void CheckMaxLength() const;
    void CheckFlags() const;
};

struct EthMessageContainer {
    SimulationTime timestamp{};
    BusControllerId controllerId{};
    uint32_t controllerIndex{};
    EthMessageFlags flags{};
    uint32_t length{};
    std::array<uint8_t, EthMessageMaxLength> data{};

    [[nodiscard]] bool SerializeTo(ChannelWriter& writer) const;
    [[nodiscard]] bool DeserializeFrom(ChannelReader& reader);
    void WriteTo(EthMessage& message) const;
    void ReadFrom(const EthMessage& message);

    [[nodiscard]] explicit operator EthMessage() const;

    [[nodiscard]] std::string ToString() const;

private:
    void CheckMaxLength() const;
};

struct LinMessageContainer {
    SimulationTime timestamp{};
    BusControllerId controllerId{};
    uint32_t controllerIndex{};
    BusMessageId id{};
    LinMessageFlags flags{};
    uint32_t length{};
    std::array<uint8_t, LinMessageMaxLength> data{};

    [[nodiscard]] bool SerializeTo(ChannelWriter& writer) const;
    [[nodiscard]] bool DeserializeFrom(ChannelReader& reader);
    void WriteTo(LinMessage& message) const;
    void ReadFrom(const LinMessage& message);

    [[nodiscard]] explicit operator LinMessage() const;

    [[nodiscard]] std::string ToString() const;

private:
    void CheckMaxLength() const;
};

template <typename TMessage, typename TMessageExtern, typename TControllerExtern>
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
                throw CoSimException("Duplicated controller id " + ToString(controller.id) + ".");
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

        throw CoSimException("Controller id " + ToString(controllerId) + " is unknown.");
    }

    std::unordered_map<BusControllerId, ControllerExtension> _controllers;

private:
    CoSimType _coSimType{};
    std::mutex _mutex;
};

template <typename TMessage, typename TMessageExtern, typename TControllerExtern>
class RemoteBusProtocolBuffer final : public BusProtocolBufferBase<TMessage, TMessageExtern, TControllerExtern> {
    using Base = BusProtocolBufferBase<TMessage, TMessageExtern, TControllerExtern>;
    using Extension = typename Base::ControllerExtension;

public:
    RemoteBusProtocolBuffer() = default;
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
                LogWarning("Queue for controller '" + std::string(extension.info.name) +
                           "' is full. Messages are dropped.");
                extension.warningSent = true;
            }

            return false;
        }

        TMessage message{};
        message.ReadFrom(messageExtern);

        _messageBuffer.PushBack(std::move(message));
        ++_messageCountPerController[extension.controllerIndex];
        return true;
    }

    [[nodiscard]] bool ReceiveInternal(TMessageExtern& messageExtern) override {
        if (_messageBuffer.IsEmpty()) {
            return false;
        }

        TMessage& message = _messageBuffer.PopFront();
        message.WriteTo(messageExtern);

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
                LogProtocolDataTrace(message.ToString());
            }

            CheckResultWithMessage(message.SerializeTo(writer), "Could not serialize message.");
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
            CheckResultWithMessage(message.DeserializeFrom(reader), "Could not deserialize message.");

            if (IsProtocolTracingEnabled()) {
                LogProtocolDataTrace(message.ToString());
            }

            Extension& extension = Base::FindController(message.controllerId);

            if (callback) {
                callback(simulationTime, extension.info, static_cast<TMessageExtern>(message));
                continue;
            }

            if (_messageCountPerController[extension.controllerIndex] == extension.info.queueSize) {
                if (!extension.warningSent) {
                    LogWarning("Receive buffer for controller '" + std::string(extension.info.name) + "' is full.");
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

template <typename TMessage, typename TMessageExtern, typename TControllerExtern>
class LocalBusProtocolBuffer final : public BusProtocolBufferBase<TMessage, TMessageExtern, TControllerExtern> {
    using Base = BusProtocolBufferBase<TMessage, TMessageExtern, TControllerExtern>;
    using Extension = typename Base::ControllerExtension;

public:
    LocalBusProtocolBuffer() = default;
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

        _sharedMemory = SharedMemory::CreateOrOpen(name, sizeOfSharedMemory);

        auto* pointerToMessageCountPerController = static_cast<uint8_t*>(_sharedMemory.data());
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
                LogWarning("Queue for controller '" + std::string(extension.info.name) +
                           "' is full. Messages are dropped.");
                extension.warningSent = true;
            }

            return false;
        }

        TMessage message{};
        message.ReadFrom(messageExtern);

        _messageBuffer->PushBack(std::move(message));
        messageCount.fetch_add(1);
        return true;
    }

    [[nodiscard]] bool ReceiveInternal(TMessageExtern& messageExtern) override {
        if (_totalReceiveCount == 0) {
            return false;
        }

        TMessage& message = _messageBuffer->PopFront();
        message.WriteTo(messageExtern);

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
                LogProtocolDataTrace(message.ToString());
            }

            Extension& extension = Base::FindController(message.controllerId);
            std::atomic<uint32_t>& receiveCountPerController = _messageCountPerController[extension.controllerIndex];
            receiveCountPerController.fetch_sub(1);
            _totalReceiveCount--;

            callback(simulationTime, extension.info, static_cast<TMessageExtern>(message));
        }

        return true;
    }

private:
    uint32_t _totalReceiveCount{};
    std::atomic<uint32_t>* _messageCountPerController{};
    ShmRingBuffer<TMessage>* _messageBuffer{};

    SharedMemory _sharedMemory;
};

#endif

class BusBuffer {
    using CanBufferBase = BusProtocolBufferBase<CanMessageContainer, CanMessage, CanController>;
    using EthBufferBase = BusProtocolBufferBase<EthMessageContainer, EthMessage, EthController>;
    using LinBufferBase = BusProtocolBufferBase<LinMessageContainer, LinMessage, LinController>;

public:
    BusBuffer(CoSimType coSimType,
              ConnectionKind connectionKind,
              const std::string& name,
              const std::vector<CanController>& canControllers,
              const std::vector<EthController>& ethControllers,
              const std::vector<LinController>& linControllers);
    BusBuffer(CoSimType coSimType,
              ConnectionKind connectionKind,
              const std::string& name,
              const std::vector<CanController>& canControllers);
    BusBuffer(CoSimType coSimType,
              ConnectionKind connectionKind,
              const std::string& name,
              const std::vector<EthController>& ethControllers);
    BusBuffer(CoSimType coSimType,
              ConnectionKind connectionKind,
              const std::string& name,
              const std::vector<LinController>& linControllers);
    ~BusBuffer() noexcept = default;

    BusBuffer(const BusBuffer&) = delete;
    BusBuffer& operator=(const BusBuffer&) = delete;

    BusBuffer(BusBuffer&&) = delete;
    BusBuffer& operator=(BusBuffer&&) = delete;

    void ClearData() const;

    [[nodiscard]] bool Transmit(const CanMessage& message) const;
    [[nodiscard]] bool Transmit(const EthMessage& message) const;
    [[nodiscard]] bool Transmit(const LinMessage& message) const;

    [[nodiscard]] bool Receive(CanMessage& message) const;
    [[nodiscard]] bool Receive(EthMessage& message) const;
    [[nodiscard]] bool Receive(LinMessage& message) const;

    [[nodiscard]] bool Serialize(ChannelWriter& writer) const;
    [[nodiscard]] bool Deserialize(ChannelReader& reader,
                                   SimulationTime simulationTime,
                                   const Callbacks& callbacks) const;

private:
    std::unique_ptr<CanBufferBase> _canTransmitBuffer;
    std::unique_ptr<EthBufferBase> _ethTransmitBuffer;
    std::unique_ptr<LinBufferBase> _linTransmitBuffer;

    std::unique_ptr<CanBufferBase> _canReceiveBuffer;
    std::unique_ptr<EthBufferBase> _ethReceiveBuffer;
    std::unique_ptr<LinBufferBase> _linReceiveBuffer;
};

}  // namespace DsVeosCoSim
