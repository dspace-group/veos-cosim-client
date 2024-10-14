// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <functional>
#include <vector>

#include "CoSimTypes.h"

struct RunTimeInfo {
    std::vector<DsVeosCoSim::CanController> canControllers;
    std::vector<DsVeosCoSim::EthController> ethControllers;
    std::vector<DsVeosCoSim::LinController> linControllers;
    std::vector<DsVeosCoSim::IoSignal> incomingSignals;
    std::vector<DsVeosCoSim::IoSignal> outgoingSignals;
    std::function<void(DsVeosCoSim_IoSignalId signalId, uint32_t length, const void* value)> write;
    std::function<bool(const DsVeosCoSim_CanMessage& message)> transmitCan;
    std::function<bool(const DsVeosCoSim_EthMessage& message)> transmitEth;
    std::function<bool(const DsVeosCoSim_LinMessage& message)> transmitLin;
};

void SwitchSendingIoSignals();
void SwitchSendingCanMessages();
void SwitchSendingEthMessages();
void SwitchSendingLinMessages();

[[nodiscard]] bool IsSendingIoSignalsEnabled();
[[nodiscard]] bool IsSendingCanMessagesEnabled();
[[nodiscard]] bool IsSendingEthMessagesEnabled();
[[nodiscard]] bool IsSendingLinMessagesEnabled();

[[nodiscard]] bool SendSomeData(DsVeosCoSim_SimulationTime simulationTime, const RunTimeInfo& runTimeInfo);