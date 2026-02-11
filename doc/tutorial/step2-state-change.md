# Step 2: Implementing Simulation State Change Callbacks

> [⬆️ Go to Tutorial](tutorial.md)

- [Step 2: Implementing Simulation State Change Callbacks](#step-2-implementing-simulation-state-change-callbacks)
  - [1 Introduction](#1-introduction)
  - [2 Preconditions](#2-preconditions)
  - [3 How to set up the client](#3-how-to-set-up-the-client)
    - [3.1 Modify Client](#31-modify-client)
    - [3.2 Rebuild Client](#32-rebuild-client)
  - [4 What it does](#4-what-it-does)
  - [5 Running the Co-Simulation](#5-running-the-co-simulation)
    - [5.1 Load the OSA File](#51-load-the-osa-file)
    - [5.2 Run Client](#52-run-client)
    - [5.3 Start the Simulation](#53-start-the-simulation)
    - [5.4 Perform manual State Changes](#54-perform-manual-state-changes)
  - [6 Client output](#6-client-output)
  - [7 Cleanup](#7-cleanup)
  - [8 Next Step](#8-next-step)


## 1 Introduction

This example shows how to implement simulation state change callbacks in a CoSim client.

## 2 Preconditions

You must have created the basic CoSim client described in [Step 1: Setting Up a Basic Co-Simulation](step1-basic.md).

## 3 How to set up the client

### 3.1 Modify Client

Update `main.cpp`:

```cpp
#include <iomanip>
#include <iostream>
#include <sstream>

#include "DsVeosCoSim/DsVeosCoSim.h"

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
    // BEGIN MODIFICATION
    // Disable output for each step to make the output more readable
    // std::cout << "Step callback received at " << DSVEOSCOSIM_SIMULATION_TIME_TO_SECONDS(simulationTime) << " s.\n";
    // END MODIFICATION
}

// BEGIN ADDITION
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

    DsVeosCoSim_Callbacks callbacks{};
    callbacks.simulationEndStepCallback = OnEndStep;

    // BEGIN ADDITION
    // Register callbacks
    callbacks.simulationStartedCallback = OnStarted;
    callbacks.simulationStoppedCallback = OnStopped;
    callbacks.simulationPausedCallback = OnPaused;
    callbacks.simulationContinuedCallback = OnContinued;
    // END ADDITION

    callbacks.userData = handle; // Pass the handle to every callback
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

The additional callback functions are called each time the simulation state changes and print corresponding messages.

Also, the output in the step function is deactivated to make the output more readable.

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

### 5.3 Start the Simulation

Run the following command:

```console
veos-sim start
```

### 5.4 Perform manual State Changes

Run the following commands:

```console
veos-sim pause
```

```console
veos-sim step
```

```console
veos-sim stop
```

## 6 Client output

The following listing shows the client output for the simulation state changes:

```console
INFO  Obtaining TCP port of dSPACE VEOS CoSim server 'CoSimExample' at 127.0.0.1 ...
INFO  Connecting to dSPACE VEOS CoSim server 'CoSimExample' at 127.0.0.1:61385 ...
INFO  Connected to dSPACE VEOS CoSim server 'CoSimExample' at 127.0.0.1:61385.
Simulation started at 0 s.
Simulation paused at 3.51 s.
Simulation continued at 3.51 s.
Simulation paused at 3.511 s.
Simulation stopped at 3.511 s.
```

## 7 Cleanup

Run the following command:

```console
veos-sim unload
```

## 8 Next Step

Refer to [Step 3: Accessing the Data Interface](step3-data-interface.md).
