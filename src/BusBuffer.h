// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <unordered_map>
#include <vector>

#include "CoSimTypes.h"
#include "Communication.h"

namespace DsVeosCoSim {

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

    void Push(const T& element) {
        _items[_writeIndex] = element;

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

    T& Pop() {
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

class BusBuffer {
    template<typename T>
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
    ~BusBuffer() = default;

    BusBuffer(const BusBuffer&) = delete;
    BusBuffer& operator=(BusBuffer const&) = delete;

    BusBuffer(BusBuffer&&) = default;
    BusBuffer& operator=(BusBuffer&&) = default;

    [[nodiscard]] Result Initialize(const std::vector<CanController>& canControllers,
                                    const std::vector<EthController>& ethControllers,
                                    const std::vector<LinController>& linControllers);

    void ClearData();

    [[nodiscard]] Result Receive(CanMessage& message);
    [[nodiscard]] Result Receive(EthMessage& message);
    [[nodiscard]] Result Receive(LinMessage& message);

    [[nodiscard]] Result Transmit(const CanMessage& message);
    [[nodiscard]] Result Transmit(const EthMessage& message);
    [[nodiscard]] Result Transmit(const LinMessage& message);

    [[nodiscard]] Result Deserialize(Channel& channel, SimulationTime simulationTime, const Callbacks& callbacks);
    [[nodiscard]] Result Serialize(Channel& channel);

private:
    [[nodiscard]] Result Initialize(const std::vector<CanController>& controllers);
    [[nodiscard]] Result Initialize(const std::vector<EthController>& controllers);
    [[nodiscard]] Result Initialize(const std::vector<LinController>& controllers);

    [[nodiscard]] Result AddMessageToReceiveBuffer(ControllerExtension<CanController>& controller, const CanMessage& message);
    [[nodiscard]] Result AddMessageToReceiveBuffer(ControllerExtension<EthController>& controller, const EthMessage& message);
    [[nodiscard]] Result AddMessageToReceiveBuffer(ControllerExtension<LinController>& controller, const LinMessage& message);

    [[nodiscard]] Result FindController(BusControllerId controllerId, ControllerExtension<CanController>** controller);
    [[nodiscard]] Result FindController(BusControllerId controllerId, ControllerExtension<EthController>** controller);
    [[nodiscard]] Result FindController(BusControllerId controllerId, ControllerExtension<LinController>** controller);

    [[nodiscard]] Result DeserializeCanMessages(Channel& channel, SimulationTime simulationTime, const Callbacks& callbacks);
    [[nodiscard]] Result DeserializeEthMessages(Channel& channel, SimulationTime simulationTime, const Callbacks& callbacks);
    [[nodiscard]] Result DeserializeLinMessages(Channel& channel, SimulationTime simulationTime, const Callbacks& callbacks);

    [[nodiscard]] Result SerializeCanMessages(Channel& channel);
    [[nodiscard]] Result SerializeEthMessages(Channel& channel);
    [[nodiscard]] Result SerializeLinMessages(Channel& channel);

    std::unordered_map<BusControllerId, ControllerExtension<CanController>> _canControllers;
    std::unordered_map<BusControllerId, ControllerExtension<EthController>> _ethControllers;
    std::unordered_map<BusControllerId, ControllerExtension<LinController>> _linControllers;

    RingBuffer<CanMessage> _canReceiveBuffer;
    RingBuffer<EthMessage> _ethReceiveBuffer;
    RingBuffer<LinMessage> _linReceiveBuffer;

    RingBuffer<CanMessage> _canTransmitBuffer;
    RingBuffer<EthMessage> _ethTransmitBuffer;
    RingBuffer<LinMessage> _linTransmitBuffer;
};

}  // namespace DsVeosCoSim
