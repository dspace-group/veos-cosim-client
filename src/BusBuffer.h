// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <unordered_map>
#include <vector>

#include "CoSimTypes.h"
#include "Communication.h"
#include "Logger.h"

namespace DsVeosCoSim {

struct CanMessageShm {
    SimulationTime timestamp{};
    BusControllerId controllerId{};
    uint32_t id{};
    CanMessageFlags flags{};
    uint32_t length{};
    std::array<uint8_t, CanMessageMaxLength> data{};

    [[nodiscard]] Result SerializeTo(Channel& channel) const;
    [[nodiscard]] Result DeserializeFrom(Channel& channel);
    void WriteTo(DsVeosCoSim_CanMessage& message) const;
    [[nodiscard]] Result ReadFrom(const DsVeosCoSim_CanMessage& message);

private:
    [[nodiscard]] Result CheckMaxLength() const;
};

struct EthMessageShm {
    SimulationTime timestamp{};
    BusControllerId controllerId{};
    EthMessageFlags flags{};
    uint32_t length{};
    std::array<uint8_t, EthMessageMaxLength> data{};

    [[nodiscard]] Result SerializeTo(Channel& channel) const;
    [[nodiscard]] Result DeserializeFrom(Channel& channel);
    void WriteTo(DsVeosCoSim_EthMessage& message) const;
    [[nodiscard]] Result ReadFrom(const DsVeosCoSim_EthMessage& message);

private:
    [[nodiscard]] Result CheckMaxLength() const;
};

struct LinMessageShm {
    SimulationTime timestamp{};
    BusControllerId controllerId{};
    uint32_t id{};
    LinMessageFlags flags{};
    uint32_t length{};
    std::array<uint8_t, LinMessageMaxLength> data{};

    [[nodiscard]] Result SerializeTo(Channel& channel) const;
    [[nodiscard]] Result DeserializeFrom(Channel& channel);
    void WriteTo(DsVeosCoSim_LinMessage& message) const;
    [[nodiscard]] Result ReadFrom(const DsVeosCoSim_LinMessage& message);

private:
    [[nodiscard]] Result CheckMaxLength() const;
};

template <typename T>
class RingBuffer final {
public:
    void Resize(size_t size) {
        _capacity = size;
        _items.resize(size);
    }

    void Clear() {
        ClearData();
        _items.clear();
    }

    void ClearData() {
        _isEmpty = true;
        _isFull = false;
        _readIndex = 0;
        _writeIndex = 0;
    }

    size_t Size() const {
        return _size;
    }

    [[nodiscard]] bool IsFull() const {
        return _isFull;
    }

    [[nodiscard]] bool IsEmpty() const {
        return _isEmpty;
    }

    void PushBack(const T& element) {
        size_t currentWriteIndex = _writeIndex;
        _items[currentWriteIndex] = std::move(element);

        ++_writeIndex;
        if (_writeIndex == _capacity) {
            _writeIndex = 0;
        }

        _isEmpty = false;
        if (_writeIndex == _readIndex) {
            _isFull = true;
        }

        _size++;
    }

    T& PopFront() {
        T& item = _items[_readIndex];

        ++_readIndex;
        if (_readIndex == _capacity) {
            _readIndex = 0;
        }

        _isFull = false;
        if (_writeIndex == _readIndex) {
            _isEmpty = true;
        }

        _size--;

        return item;
    }

private:
    std::vector<T> _items;

    size_t _capacity{};
    size_t _readIndex{};
    size_t _writeIndex{};
    size_t _size{};

    bool _isFull = false;
    bool _isEmpty = true;
};

template <typename TMessage, typename TMessageShm, typename TController>
class BusProtocolBuffer {
    struct ControllerExtension {
        TController info{};
        uint32_t receiveCount{};
        uint32_t transmitCount{};
        bool receiveWarningSent = false;
        bool transmitWarningSent = false;

        void ClearData() {
            receiveCount = 0;
            transmitCount = 0;
            receiveWarningSent = false;
            transmitWarningSent = false;
        }
    };

public:
    BusProtocolBuffer() = default;
    ~BusProtocolBuffer() noexcept = default;

    BusProtocolBuffer(const BusProtocolBuffer&) = delete;
    BusProtocolBuffer& operator=(BusProtocolBuffer const&) = delete;

    BusProtocolBuffer(BusProtocolBuffer&&) = delete;
    BusProtocolBuffer& operator=(BusProtocolBuffer&&) = default;

    [[nodiscard]] Result Initialize(const std::vector<TController>& controllers) {
        _controllers.clear();
        _controllers.reserve(controllers.size());

        size_t totalQueueSize = 0;
        for (const auto& controller : controllers) {
            const auto search = _controllers.find(controller.id);
            if (search != _controllers.end()) {
                LogError("Duplicated controller id " + ToString(controller.id) + ".");
                return Result::Error;
            }

            ControllerExtension extension{};
            extension.info = controller;
            _controllers[controller.id] = extension;
            totalQueueSize += controller.queueSize;
        }

        _receiveBuffer.Clear();
        _receiveBuffer.Resize(totalQueueSize);
        _transmitBuffer.Clear();
        _transmitBuffer.Resize(totalQueueSize);
        return Result::Ok;
    }

    void ClearData() {
        for (auto& [controllerId, dataPerController] : _controllers) {
            dataPerController.ClearData();
        }

        _receiveBuffer.ClearData();
        _transmitBuffer.ClearData();
    }

    [[nodiscard]] Result Receive(TMessage& message) {
        if (_receiveBuffer.IsEmpty()) {
            return Result::Empty;
        }

        TMessageShm& messageShm = _receiveBuffer.PopFront();
        messageShm.WriteTo(message);

        ControllerExtension* extension{};
        CheckResult(FindController(message.controllerId, &extension));
        extension->receiveCount--;
        return Result::Ok;
    }

    [[nodiscard]] Result Transmit(const TMessage& message) {
        TMessageShm messageShm{};
        CheckResult(messageShm.ReadFrom(message));

        ControllerExtension* extension{};
        CheckResult(FindController(message.controllerId, &extension));

        if (extension->transmitCount == extension->info.queueSize) {
            if (!extension->transmitWarningSent) {
                LogWarning("Queue for controller '" + std::string(extension->info.name) + "' is full. Messages are dropped.");
                extension->transmitWarningSent = true;
            }

            return Result::Full;
        }

        _transmitBuffer.PushBack(messageShm);
        extension->transmitCount++;
        return Result::Ok;
    }

    [[nodiscard]] Result Deserialize(
        Channel& channel,
        SimulationTime simulationTime,
        const std::function<void(SimulationTime simulationTime, const TController& controller, const TMessage& message)>& callback) {
        uint32_t totalCanCount = 0;
        CheckResult(channel.Read(totalCanCount));

        for (uint32_t i = 0; i < totalCanCount; i++) {
            TMessageShm messageShm{};
            CheckResult(messageShm.DeserializeFrom(channel));

            ControllerExtension* extension{};
            CheckResult(FindController(messageShm.controllerId, &extension));

            if (callback) {
                TMessage message{};
                messageShm.WriteTo(message);
                callback(simulationTime, extension->info, message);
                continue;
            }

            if (extension->receiveCount == extension->info.queueSize) {
                if (!extension->receiveWarningSent) {
                    LogWarning("Receive buffer for controller '" + extension->info.name + "' is full.");
                    extension->receiveWarningSent = true;
                }

                continue;
            }

            extension->receiveCount++;
            _receiveBuffer.PushBack(messageShm);
        }

        return Result::Ok;
    }

    [[nodiscard]] Result Serialize(Channel& channel) {
        const auto count = static_cast<uint32_t>(_transmitBuffer.Size());
        CheckResult(channel.Write(count));
        for (uint32_t i = 0; i < count; i++) {
            CheckResult(_transmitBuffer.PopFront().SerializeTo(channel));
        }

        for (auto& [id, controller] : _controllers) {
            controller.transmitCount = 0;
        }

        return Result::Ok;
    }

private:
    [[nodiscard]] Result FindController(BusControllerId controllerId, ControllerExtension** extension) {
        const auto search = _controllers.find(controllerId);
        if (search != _controllers.end()) {
            *extension = &search->second;
            return Result::Ok;
        }

        LogError("controller id " + ToString(controllerId) + " is unknown.");
        return Result::InvalidArgument;
    }

    std::unordered_map<BusControllerId, ControllerExtension> _controllers;
    RingBuffer<TMessageShm> _receiveBuffer;
    RingBuffer<TMessageShm> _transmitBuffer;
};

class BusBuffer {
    template <typename T>
    struct ControllerExtension {
        T info{};
        uint32_t receiveCount{};
        uint32_t transmitCount{};
        bool receiveWarningSent = false;
        bool transmitWarningSent = false;

        void ClearData() {
            receiveCount = 0;
            transmitCount = 0;
            receiveWarningSent = false;
            transmitWarningSent = false;
        }
    };

public:
    BusBuffer() = default;
    ~BusBuffer() noexcept = default;

    BusBuffer(const BusBuffer&) = delete;
    BusBuffer& operator=(BusBuffer const&) = delete;

    BusBuffer(BusBuffer&&) = delete;
    BusBuffer& operator=(BusBuffer&&) = delete;

    [[nodiscard]] Result Initialize(const std::vector<CanController>& canControllers,
                                    const std::vector<EthController>& ethControllers,
                                    const std::vector<LinController>& linControllers);

    void ClearData();

    [[nodiscard]] Result Receive(DsVeosCoSim_CanMessage& message);
    [[nodiscard]] Result Receive(DsVeosCoSim_EthMessage& message);
    [[nodiscard]] Result Receive(DsVeosCoSim_LinMessage& message);

    [[nodiscard]] Result Transmit(const DsVeosCoSim_CanMessage& message);
    [[nodiscard]] Result Transmit(const DsVeosCoSim_EthMessage& message);
    [[nodiscard]] Result Transmit(const DsVeosCoSim_LinMessage& message);

    [[nodiscard]] Result Deserialize(Channel& channel, SimulationTime simulationTime, const Callbacks& callbacks);
    [[nodiscard]] Result Serialize(Channel& channel);

private:
    BusProtocolBuffer<DsVeosCoSim_CanMessage, CanMessageShm, CanController> _canBuffer;
    BusProtocolBuffer<DsVeosCoSim_EthMessage, EthMessageShm, EthController> _ethBuffer;
    BusProtocolBuffer<DsVeosCoSim_LinMessage, LinMessageShm, LinController> _linBuffer;
};

}  // namespace DsVeosCoSim
