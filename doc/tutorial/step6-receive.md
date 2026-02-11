# Step 6: Receiving Data

> [⬆️ Go to Tutorial](tutorial.md)

- [Step 6: Receiving Data](#step-6-receiving-data)
  - [1 Introduction](#1-introduction)
  - [2 Preconditions](#2-preconditions)
  - [3 How to set up the Co-Simulation](#3-how-to-set-up-the-co-simulation)
    - [3.1 Modify Client](#31-modify-client)
    - [3.2 Rebuild Client](#32-rebuild-client)
  - [4 What it does](#4-what-it-does)
  - [5 Running the Co-Simulation](#5-running-the-co-simulation)
    - [5.1 Enable bus logging](#51-enable-bus-logging)
    - [5.2 Load the OSA File](#52-load-the-osa-file)
    - [5.3 Run Client](#53-run-client)
    - [5.4 Start the Simulation](#54-start-the-simulation)
    - [5.5 Get Bus Log File](#55-get-bus-log-file)
  - [6 Client output and Bus Log Files](#6-client-output-and-bus-log-files)
  - [7 Cleanup](#7-cleanup)
  - [8 Next Step](#8-next-step)

## 1 Introduction

This example shows how to receive data from other co-simulation participants using receive functions instead of data callbacks.

## 2 Preconditions

You must have run [Step 5: Sending Data](step5-send.md).

## 3 How to set up the Co-Simulation

### 3.1 Modify Client

Update `main.cpp`:

```cpp
#include <iomanip>
#include <iostream>
#include <sstream>

#include "DsVeosCoSim/DsVeosCoSim.h"

uint32_t canControllersCount{};
const DsVeosCoSim_CanController* canControllers{};
uint32_t outgoingSignalsCount{};
const DsVeosCoSim_IoSignal* outgoingSignals{};

void OnLogCallback(DsVeosCoSim_Severity severity, const char* logMessage) {
    switch (severity) {
        case DsVeosCoSim_Severity_Error:
            std::cout << "ERROR " << logMessage << "\n";
            break;
        case DsVeosCoSim_Severity_Warning:
            std::cout << "WARN  " << logMessage << "\n";
            break;
        case DsVeosCoSim_Severity_Info:
            std::cout << "INFO  " << logMessage << "\n";
            break;
        case DsVeosCoSim_Severity_Trace:
            std::cout << "TRACE " << logMessage << "\n";
            break;
    }
}

std::string DataToString(const uint8_t* data, uint32_t dataLength, char separator = 0) {
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (uint32_t i = 0; i < dataLength; i++) {
        oss << std::setw(2) << static_cast<int>(data[i]);
        if ((i < dataLength - 1) && separator != 0) {
            oss << separator;
        }
    }

    return oss.str();
}

void OnEndStep(DsVeosCoSim_SimulationTime simulationTime, void* userData) {
    DsVeosCoSim_Handle handle = userData;
    // BEGIN MODIFICATION
    while (true) {
        DsVeosCoSim_CanMessage message{};
        DsVeosCoSim_Result result = DsVeosCoSim_ReceiveCanMessage(handle, &message);
        if (result == DsVeosCoSim_Result_Ok) {
            std::cout << "Received CAN message with ID " << message.id << " at " << DSVEOSCOSIM_SIMULATION_TIME_TO_SECONDS(simulationTime) << " s.\n";
        } else if (result == DsVeosCoSim_Result_Empty) {
            // No more messages
            break;
        } else {
            DsVeosCoSim_Disconnect(handle);
            return;
        }
    }
    // END MODIFICATION

    uint8_t data[1]{42};
    DsVeosCoSim_CanMessage canMessage{};
    canMessage.id = 12;
    canMessage.flags = DsVeosCoSim_CanMessageFlags_FlexibleDataRateFormat;
    canMessage.length = 1;
    canMessage.data = data;
    DsVeosCoSim_Result result = DsVeosCoSim_TransmitCanMessage(handle, &canMessage);
    if (result == DsVeosCoSim_Result_Full) {
        // No more space in the internal buffer
    } else if (result != DsVeosCoSim_Result_Ok) {
        DsVeosCoSim_Disconnect(handle);
        return;
    }
}

// Called when the simulation starts
void OnStarted(DsVeosCoSim_SimulationTime simulationTime, void* userData) {
    std::cout << "Simulation started at " << DSVEOSCOSIM_SIMULATION_TIME_TO_SECONDS(simulationTime) << " s.\n";
}

// Called when the simulation stops
void OnStopped(DsVeosCoSim_SimulationTime simulationTime, void* userData) {
    std::cout << "Simulation stopped at " << DSVEOSCOSIM_SIMULATION_TIME_TO_SECONDS(simulationTime) << " s.\n";
}

// Called when the simulation pauses
void OnPaused(DsVeosCoSim_SimulationTime simulationTime, void* userData) {
    std::cout << "Simulation paused at " << DSVEOSCOSIM_SIMULATION_TIME_TO_SECONDS(simulationTime) << " s.\n";
}

// Called when the simulation continues
void OnContinued(DsVeosCoSim_SimulationTime simulationTime, void* userData) {
    std::cout << "Simulation continued at " << DSVEOSCOSIM_SIMULATION_TIME_TO_SECONDS(simulationTime) << " s.\n";
}

// Will be called when a CAN message was received
void OnCanMessage(DsVeosCoSim_SimulationTime simulationTime,
                  const DsVeosCoSim_CanController* canController,
                  const DsVeosCoSim_CanMessage* message,
                  void* userData) {
    std::cout << "Received CAN message with ID " << message->id << " with data " << DataToString(message->data, message->length, '-') << " from CAN controller "
              << canController->name << " at " << DSVEOSCOSIM_SIMULATION_TIME_TO_SECONDS(simulationTime) << " s.\n";
}

int main() {
    DsVeosCoSim_SetLogCallback(OnLogCallback);

    DsVeosCoSim_Handle handle = DsVeosCoSim_Create();
    if (handle == nullptr) {
        return 1;
    }

    DsVeosCoSim_ConnectConfig connectConfig{};
    connectConfig.serverName = "CoSimExample";
    if (DsVeosCoSim_Connect(handle, connectConfig) != DsVeosCoSim_Result_Ok) {
        DsVeosCoSim_Destroy(handle);
        return 1;
    }

    // Get CAN controllers
    if (DsVeosCoSim_GetCanControllers(handle, &canControllersCount, &canControllers) != DsVeosCoSim_Result_Ok) {
        DsVeosCoSim_Disconnect(handle);
        DsVeosCoSim_Destroy(handle);
        return 1;
    }

    for (uint32_t i = 0; i < canControllersCount; i++) {
        std::cout << "Found CAN controller '" << canControllers[i].name << "'\n";
    }

    // Get outgoing signals
    if (DsVeosCoSim_GetOutgoingSignals(handle, &outgoingSignalsCount, &outgoingSignals) != DsVeosCoSim_Result_Ok) {
        DsVeosCoSim_Disconnect(handle);
        DsVeosCoSim_Destroy(handle);
        return 1;
    }

    for (uint32_t i = 0; i < outgoingSignalsCount; i++) {
        std::cout << "Found outgoing signal '" << outgoingSignals[i].name << "'\n";
    }

    DsVeosCoSim_Callbacks callbacks{};
    callbacks.simulationEndStepCallback = OnEndStep;
    callbacks.simulationStartedCallback = OnStarted;
    callbacks.simulationStoppedCallback = OnStopped;
    callbacks.simulationPausedCallback = OnPaused;
    callbacks.simulationContinuedCallback = OnContinued;
    // BEGIN MODIFICATION
    // callbacks.canMessageReceivedCallback = OnCanMessage;
    // END MODIFICATION
    callbacks.userData = handle;  // Pass the handle to every callback
    if (DsVeosCoSim_RunCallbackBasedCoSimulation(handle, callbacks) != DsVeosCoSim_Result_Disconnected) {
        DsVeosCoSim_Disconnect(handle);
        DsVeosCoSim_Destroy(handle);
    }

    DsVeosCoSim_Destroy(handle);
    return 0;
}
```

### 3.2 Rebuild Client

Run the following command in the directory `DsVeosCoSimDemo`:

```console
cmake --build ./build
```

## 4 What it does

When the `OnEndStep` function is called at the end of each simulation step, it calls the [DsVeosCoSim_ReceiveCanMessage](../api-reference/functions/DsVeosCoSim_ReceiveCanMessage.md) function repeatedly until this function returns [DsVeosCoSim_Result_Empty](../api-reference/enumerations/DsVeosCoSim_Result.md). For each received message, information on the message ID and the simulation time is printed.

## 5 Running the Co-Simulation

### 5.1 Enable bus logging

Run the following command:

```console
veos-sim config --enable-bus-log
```

### 5.2 Load the OSA File

Run the following command in the directory `DsVeosCoSimDemo`:

```console
veos-sim load ./DsVeosCoSimWithController.osa
```

### 5.3 Run Client

Run the following command in the directory `DsVeosCoSimDemo`:

```console
./build/Debug/DsVeosCoSimDemo
```

### 5.4 Start the Simulation

Run the following command:

```console
veos-sim start
```

### 5.5 Get Bus Log File

Run the following command in the directory `DsVeosCoSimDemo`:

```console
veos-sim show-log Controller.BusTransfer.pcapng > BusLog.pcapng
```

## 6 Client output and Bus Log Files

The client produces the same output as in [Step 4: Using Data Callback Functions to Get Information on CAN Bus Messages](step4-callback.md).

The bus log files correspond to the bus log files produced by [Step 5: Sending Data](step5-send.md).

## 7 Cleanup

Run the following command:

```console
veos-sim unload
```

## 8 Next Step

Refer to [Step 7: Running a Polling-Based Simulation](step7-poll.md).
