# Step 4: Using Data Callback Functions to Get Information on CAN Bus Messages

> [⬆️ Go to Tutorial](tutorial.md)

- [Step 4: Using Data Callback Functions to Get Information on CAN Bus Messages](#step-4-using-data-callback-functions-to-get-information-on-can-bus-messages)
  - [1 Introduction](#1-introduction)
  - [2 Preconditions](#2-preconditions)
  - [3 How to set up the Co-Simulation](#3-how-to-set-up-the-co-simulation)
    - [3.1 Modify Client](#31-modify-client)
    - [3.2 Rebuild Client](#32-rebuild-client)
    - [3.3 Modify OSA](#33-modify-osa)
      - [3.3.1 Copy V-ECU](#331-copy-v-ecu)
      - [3.3.2 Build V-ECU](#332-build-v-ecu)
      - [3.3.3 Import CoSim Server](#333-import-cosim-server)
      - [3.3.4 Connect CoSim Server to V-ECU](#334-connect-cosim-server-to-v-ecu)
  - [4 What it does](#4-what-it-does)
  - [5 Running the Co-Simulation](#5-running-the-co-simulation)
    - [5.1 Load the OSA File](#51-load-the-osa-file)
    - [5.2 Run Client](#52-run-client)
    - [5.3 Start the Simulation](#53-start-the-simulation)
  - [6 Client output](#6-client-output)
  - [7 Cleanup](#7-cleanup)
  - [8 Next Step](#8-next-step)

## 1 Introduction

This example demonstrates how to use data callbacks to get information on received CAN bus messages.

In addition to modifying the basic example client, you have to modify the `DsVeosCoSim.osa` by adding a V-ECU with
a CAN controller so that there are CAN bus messages for the CoSim server to receive.

## 2 Preconditions

You must have run [Step 3: Accessing the Data Interface](step3-data-interface.md).

You must have a VEOS installation on Windows.

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

// BEGIN ADDITION
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
// END ADDITION

void OnEndStep(DsVeosCoSim_SimulationTime simulationTime, void* userData) {
    // Disable output for each step to make the output more readable
    // std::cout << "Step callback received at " << DSVEOSCOSIM_SIMULATION_TIME_TO_SECONDS(simulationTime) << " s.\n";
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

// BEGIN ADDITION
// Will be called when a CAN message was received
void OnCanMessage(DsVeosCoSim_SimulationTime simulationTime,
                  const DsVeosCoSim_CanController* canController,
                  const DsVeosCoSim_CanMessage* message,
                  void* userData) {
    std::cout << "Received CAN message with ID " << message->id << " with data " << DataToString(message->data, message->length, '-') << " from CAN controller "
              << canController->name << " at " << DSVEOSCOSIM_SIMULATION_TIME_TO_SECONDS(simulationTime) << " s.\n";
}
// END ADDITION

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
    // BEGIN ADDITION
    callbacks.canMessageReceivedCallback = OnCanMessage;
    // END ADDITION
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

### 3.3 Modify OSA

#### 3.3.1 Copy V-ECU

Open the VEOS Player on Windows and copy the VEOS demos to your Documents folder via **File** - **Copy Demo Files**.

Copy the file `Controller.vecu` from `<My Documents>\dSPACE\VEOS\<VEOS Version>\2Ecu_CanBusSimulation` to the directory `DsVeosCoSimDemo`.

#### 3.3.2 Build V-ECU

Run the following command in the directory `DsVeosCoSimDemo`:

```console
veos-build classic-vecu ./Controller.vecu -o ./DsVeosCoSimWithController.osa
```

#### 3.3.3 Import CoSim Server

Run the following command in the directory `DsVeosCoSimDemo`:

```console
veos-model import -p ./DsVeosCoSim.osa ./DsVeosCoSimWithController.osa
```

#### 3.3.4 Connect CoSim Server to V-ECU

Run the following command in the directory `DsVeosCoSimDemo`:

```console
veos-model connect ./DsVeosCoSimWithController.osa --autoconnect-communication-controllers
```

## 4 What it does

Each time the CoSim server receives a CAN bus message, the `OnCanMessage` callback function prints the message ID,
the name of the CAN controller from which the message was received, and the corresponding simulation time.

## 5 Running the Co-Simulation

### 5.1 Load the OSA File

Run the following command in the directory `DsVeosCoSimDemo`:

```console
veos-sim load ./DsVeosCoSimWithController.osa
```

### 5.2 Run Client

Run the following command in the directory `DsVeosCoSimDemo`:

```console
./build/Debug/DsVeosCoSimDemo
```

### 5.3 Start the Simulation

Run the following command:

```console
veos-sim start
```

## 6 Client output

The following listing shows the output produced by the client:

```console
Received CAN message with ID 10 and data 00-00 from bus controller CanController at 0.002 s.
Received CAN message with ID 10 and data 00-00 from bus controller CanController at 0.022 s.
Received CAN message with ID 10 and data 00-00 from bus controller CanController at 0.042 s.
Received CAN message with ID 10 and data 00-00 from bus controller CanController at 0.062 s.
```

## 7 Cleanup

Run the following command:

```console
veos-sim unload
```

## 8 Next Step

Refer to [Step 5: Sending Data](step5-send.md).
