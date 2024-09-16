// Copyright dSPACE GmbH. All rights reserved.

#include "ClientServerTestHelper.h"

#include "BusBuffer.h"
#include "Generator.h"
#include "Logger.h"

using namespace DsVeosCoSim;

namespace {

bool g_sendIoData;
bool g_sendCanMessages;
bool g_sendEthMessages;
bool g_sendLinMessages;

void PrintStatus(bool value, std::string_view what) {
    LogInfo("{} sending {}.", value ? "Enabled" : "Disabled", what);
}

}  // namespace

void SwitchSendingIoSignals() {
    g_sendIoData = !g_sendIoData;
    PrintStatus(g_sendIoData, "IO data");
}

void SwitchSendingCanMessages() {
    g_sendCanMessages = !g_sendCanMessages;
    PrintStatus(g_sendCanMessages, "CAN messages");
}

void SwitchSendingEthMessages() {
    g_sendEthMessages = !g_sendEthMessages;
    PrintStatus(g_sendEthMessages, "ETH messages");
}

void SwitchSendingLinMessages() {
    g_sendLinMessages = !g_sendLinMessages;
    PrintStatus(g_sendLinMessages, "LIN messages");
}

bool IsSendingIoSignalsEnabled() {
    return g_sendIoData;
}

bool IsSendingCanMessagesEnabled() {
    return g_sendCanMessages;
}

bool IsSendingEthMessagesEnabled() {
    return g_sendEthMessages;
}

bool IsSendingLinMessagesEnabled() {
    return g_sendLinMessages;
}

bool SendSomeData(DsVeosCoSim::SimulationTime simulationTime, const RunTimeInfo& runTimeInfo) {
    static int64_t lastHalfSecond = -1;
    static int64_t counter = 0;
    const int64_t currentHalfSecond = simulationTime / 500000000;
    if (currentHalfSecond == lastHalfSecond) {
        return true;
    }

    lastHalfSecond = currentHalfSecond;
    counter++;

    if (IsSendingIoSignalsEnabled() && ((counter % 4) == 0)) {
        for (const IoSignal& signal : runTimeInfo.outgoingSignals) {
            std::vector<uint8_t> data = GenerateIoData(signal);
            runTimeInfo.write(signal.id, signal.length, data.data());
        }
    }

    if (IsSendingCanMessagesEnabled() && ((counter % 4) == 1)) {
        for (const CanController& controller : runTimeInfo.canControllers) {
            CanMessage message{};
            FillWithRandom(message, controller.id);
            CheckResult(runTimeInfo.transmitCan(message));
        }
    }

    if (IsSendingEthMessagesEnabled() && ((counter % 4) == 2)) {
        for (const EthController& controller : runTimeInfo.ethControllers) {
            EthMessage message{};
            FillWithRandom(message, controller.id);
            CheckResult(runTimeInfo.transmitEth(message));
        }
    }

    if (IsSendingLinMessagesEnabled() && ((counter % 4) == 3)) {
        for (const LinController& controller : runTimeInfo.linControllers) {
            LinMessage message{};
            FillWithRandom(message, controller.id);
            CheckResult(runTimeInfo.transmitLin(message));
        }
    }

    return true;
}
