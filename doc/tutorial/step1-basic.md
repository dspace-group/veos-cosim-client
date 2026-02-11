# Step 1: Setting Up a Basic Co-Simulation

> [⬆️ Go to Tutorial](tutorial.md)

- [Step 1: Setting Up a Basic Co-Simulation](#step-1-setting-up-a-basic-co-simulation)
  - [1 Introduction](#1-introduction)
  - [2 Precondition](#2-precondition)
  - [3 How to set up the client](#3-how-to-set-up-the-client)
    - [3.1 Modify Client](#31-modify-client)
    - [3.2 Rebuild the CoSim Client](#32-rebuild-the-cosim-client)
  - [4 What it does](#4-what-it-does)
    - [4.1 Logging](#41-logging)
    - [4.2 Create a Handle](#42-create-a-handle)
    - [4.3 Connect](#43-connect)
    - [4.4 Run Callback-based Co-Simulation](#44-run-callback-based-co-simulation)
  - [5 Running the Co-Simulation](#5-running-the-co-simulation)
    - [5.1 Load the OSA File](#51-load-the-osa-file)
    - [5.2 Run Client](#52-run-client)
    - [5.3 Start the Simulation](#53-start-the-simulation)
  - [6 Client output](#6-client-output)
  - [7 Cleanup](#7-cleanup)
  - [8 Next Step](#8-next-step)

## 1 Introduction

This step shows how to create a basic CoSim client which connects to the CoSim server in VEOS and reports on its progress via log callbacks.

## 2 Precondition

You must have made the preparations described in [How to Prepare the CoSim Demo](prepare.md).

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

// Called after every interval specified in the StepSize parameter of the Example.json file
void OnEndStep(DsVeosCoSim_SimulationTime simulationTime, void* userData) {
    std::cout << "Step callback received at " << DSVEOSCOSIM_SIMULATION_TIME_TO_SECONDS(simulationTime) << " s.\n";
}

int main() {
    // Step A: Enable logging
    DsVeosCoSim_SetLogCallback(OnLogCallback);

    // Step B: Create a handle for the client
    DsVeosCoSim_Handle handle = DsVeosCoSim_Create();
    if (handle == nullptr) {
        return 1;
    }

    // Step C: Connect to the Example server that is running in VEOS
    DsVeosCoSim_ConnectConfig connectConfig{};
    connectConfig.serverName = "CoSimExample";
    if (DsVeosCoSim_Connect(handle, connectConfig) != DsVeosCoSim_Result_Ok) {
        DsVeosCoSim_Destroy(handle);
        return 1;
    }

    // Step D: Run a callback-based co-simulation
    DsVeosCoSim_Callbacks callbacks{};
    callbacks.simulationEndStepCallback = OnEndStep;
    callbacks.userData = handle;  // Pass the handle to every callback
    if (DsVeosCoSim_RunCallbackBasedCoSimulation(handle, callbacks) != DsVeosCoSim_Result_Disconnected) {
        DsVeosCoSim_Disconnect(handle);
        DsVeosCoSim_Destroy(handle);
    }

    DsVeosCoSim_Destroy(handle);
    return 0; 
}
```

### 3.2 Rebuild the CoSim Client

Run the following command in the directory `DsVeosCoSimDemo`:

```console
cmake --build ./build
```

## 4 What it does

### 4.1 Logging

The `OnLogCallback` function prints log messages corresponding to the severity level. It is registered by the
[DsVeosCoSim_SetLogCallback](../api-reference/functions/DsVeosCoSim_SetLogCallback.md) function in `Step A`.

### 4.2 Create a Handle

In `Step B`, a handle for the client is created which is used by all the other functions.

Only one handle can be connected to one CoSim server in the VEOS simulator. However, a client can create multiple
handles and connect them to other CoSim servers on the same or other VEOS simulators.

### 4.3 Connect

In `Step C`, a connection to the Example server running in VEOS is established via the [DsVeosCoSim_Connect](../api-reference/functions/DsVeosCoSim_Connect.md) function.

> [!Tip]
> - The client can also connect to a server that is running on a different computer. In this case, you have to add
>     `connectConfig.remoteIpAddress = "<IP address>";`.
>
> - The Example server uses a dynamic TCP port and a port mapper. However, if you specified a static TCP port,
>   you have to provide this to the client via `DsVeosCoSim_ConnectConfig.remotePort`.
>   For more information, refer to [Connecting to a CoSim server](../basics/basics-clients.md#connecting-to-a-cosim-server).

### 4.4 Run Callback-based Co-Simulation

In `Step D` a callback-based co-simulation is started. The [DsVeosCoSim_RunCallbackBasedCoSimulation](../api-reference/functions/DsVeosCoSim_RunCallbackBasedCoSimulation.md) function blocks
until either [DsVeosCoSim_Disconnect](../api-reference/functions/DsVeosCoSim_Disconnect.md) function is called by a callback function or the simulation is unloaded in VEOS. Therefore, the
expected return value is [DsVeosCoSim_Result_Disconnected](../api-reference/enumerations/DsVeosCoSim_Result.md) instead of [DsVeosCoSim_Result_Ok](../api-reference/enumerations/DsVeosCoSim_Result.md).

In this example, the `simulationEndStepCallback` is called at the end of each simulation step and prints the corresponding simulation time in seconds as
specified in the `OnEndStep` function definition.

> [!Tip]
> You can also run polling-based co-simulations. Refer to [Step 7: Running a Polling-Based Simulation](step7-poll.md) for an example.

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

```console
veos-sim start
```

## 6 Client output

The following listing shows the output produced by the client:

```console
INFO  Obtaining TCP port of dSPACE VEOS CoSim server 'CoSimExample' at 127.0.0.1 ...
INFO  Connecting to dSPACE VEOS CoSim server 'CoSimExample' at 127.0.0.1:50212 ...
INFO  Connected to dSPACE VEOS CoSim server 'CoSimExample' at 127.0.0.1:50212.
Step callback received at 0 s.
Step callback received at 0.001 s.
Step callback received at 0.002 s.
Step callback received at 0.003 s.
...
```

## 7 Cleanup

Run the following command:

```console
veos-sim unload
```

## 8 Next Step

Refer to [Step 2: Implementing Simulation State Change Callbacks](step2-state-change.md).
