# Step 3: Accessing the Data Interface

> [⬆️ Go to Tutorial](tutorial.md)

- [Step 3: Accessing the Data Interface](#step-3-accessing-the-data-interface)
  - [1 Introduction](#1-introduction)
  - [2 Preconditions](#2-preconditions)
  - [3 How to set up the Co-Simulation](#3-how-to-set-up-the-co-simulation)
    - [3.1 Modify Client](#31-modify-client)
    - [3.2 Rebuild Client](#32-rebuild-client)
  - [4 What it does](#4-what-it-does)
  - [5 Running the Co-Simulation](#5-running-the-co-simulation)
    - [5.1 Load the OSA File](#51-load-the-osa-file)
    - [5.2 Run Client](#52-run-client)
  - [6 Client output](#6-client-output)
  - [7 Cleanup](#7-cleanup)
  - [8 Next Step](#8-next-step)

## 1 Introduction

This example shows how to access the VEOS CoSim data interface to get information on available bus controllers and I/O signals.

## 2 Preconditions

You must have created the basic CoSim client described in [Step 2: Implementing Simulation State Change Callbacks](step2-state-change.md).

## 3 How to set up the Co-Simulation

### 3.1 Modify Client

Update `main.cpp`:

```cpp
#include <iomanip>
#include <iostream>
#include <sstream>

#include "DsVeosCoSim/DsVeosCoSim.h"

// BEGIN ADDITION
uint32_t canControllersCount{};
const DsVeosCoSim_CanController* canControllers{};
uint32_t outgoingSignalsCount{};
const DsVeosCoSim_IoSignal* outgoingSignals{};
// END ADDITION

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

    // BEGIN ADDITION
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
    // END ADDITION

    DsVeosCoSim_Callbacks callbacks{};
    callbacks.simulationEndStepCallback = OnEndStep;
    callbacks.simulationStartedCallback = OnStarted;
    callbacks.simulationStoppedCallback = OnStopped;
    callbacks.simulationPausedCallback = OnPaused;
    callbacks.simulationContinuedCallback = OnContinued;
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

- First, the client collects information on the available CAN controllers via the [DsVeosCoSim_GetCanControllers](../api-reference/functions/DsVeosCoSim_GetCanControllers.md) function.

- It then prints the name of each CAN controller.

- Then it does the same for the outgoing I/O signals.

## 5 Running the Co-Simulation

### 5.1 Load the OSA File

Run the following command in the directory `DsVeosCoSimDemo`:

```console
veos-sim load ./DsVeosCoSim.osa
```

### 5.2 Run Client

Run the following command in the directory `DsVeosCoSimDemo`:

```console
./build/Debug/DsVeosCoSimDemo
```

## 6 Client output

The following listing shows the output produced by the client:

```console
Found CAN controller 'CanController'
Found outgoing signal '/Port1/Signal1'
```

## 7 Cleanup

Run the following command:

```console
veos-sim unload
```

## 8 Next Step

Refer to [Step 4: Using Data Callback Functions to Get Information on CAN Bus Messages](step4-callback.md).
