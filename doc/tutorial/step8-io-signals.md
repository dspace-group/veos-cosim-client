# Step 8: Handling I/O Signals

> [⬆️ Go to Tutorial](tutorial.md)

- [Step 8: Handling I/O Signals](#step-8-handling-io-signals)
  - [1 Introduction](#1-introduction)
  - [2 Preconditions](#2-preconditions)
  - [3 How to modify the polling-based client](#3-how-to-modify-the-polling-based-client)
    - [3.1 Modify Client](#31-modify-client)
    - [3.2 Rebuild Client](#32-rebuild-client)
    - [3.3 Creating a second CoSim Client](#33-creating-a-second-cosim-client)
    - [3.4 Update Cmake file](#34-update-cmake-file)
    - [3.5 Build the second CoSim Client](#35-build-the-second-cosim-client)
    - [3.6 Create the second CoSim Server](#36-create-the-second-cosim-server)
    - [3.7 Create the second OSA file](#37-create-the-second-osa-file)
    - [3.8 Import the first CoSim server](#38-import-the-first-cosim-server)
    - [3.9 Connect Signals](#39-connect-signals)
  - [4 What it does](#4-what-it-does)
  - [5 Running the Co-Simulation](#5-running-the-co-simulation)
    - [5.1 Load the OSA File](#51-load-the-osa-file)
    - [5.2 Run first Client](#52-run-first-client)
    - [5.3 Run second Client](#53-run-second-client)
    - [5.4 Start the Simulation](#54-start-the-simulation)
  - [6 Client output](#6-client-output)
  - [7 Cleanup](#7-cleanup)
  - [8 Next Step](#8-next-step)

## 1 Introduction

This example illustrates how to handle I/O signals in a co-simulation using a modification of
[Step 7: Running a Polling-Based Simulation](step7-poll.md) and an additional CoSim server-client pair.

The polling-based co-simulation is modified to change the value of the outgoing signal at specified times when
the simulation is advanced by a step.
The second client reads the signal from the first one and reports on each signal value change.

## 2 Preconditions

You must have run [Step 7: Running a Polling-Based Simulation](step7-signals.md).

## 3 How to modify the polling-based client

### 3.1 Modify Client

Update `main.cpp`:

```cpp
#include <iomanip>
#include <iostream>
#include <sstream>

#include "DsVeosCoSim/DsVeosCoSim.h"

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
    callbacks.canMessageReceivedCallback = OnCanMessage;
    callbacks.userData = handle;  // Pass the handle to every callback
    if (DsVeosCoSim_StartPollingBasedCoSimulation(handle, callbacks) != DsVeosCoSim_Result_Ok) {
        DsVeosCoSim_Disconnect(handle);
        DsVeosCoSim_Destroy(handle);
        return 1;
    }

    while (true) {
        DsVeosCoSim_SimulationTime simulationTime{};
        DsVeosCoSim_Command command{};
        if (DsVeosCoSim_PollCommand(handle, &simulationTime, &command) != DsVeosCoSim_Result_Ok) {
            DsVeosCoSim_Disconnect(handle);
            DsVeosCoSim_Destroy(handle);
            return 1;
        }

        switch (command) {
            case DsVeosCoSim_Command_Step: {
                std::cout << "Simulation stepped at " << DSVEOSCOSIM_SIMULATION_TIME_TO_SECONDS(simulationTime) << " s.\n";
                if ((simulationTime % 10000000) == 0) {  // Only every 10 milliseconds
                    double value = DSVEOSCOSIM_SIMULATION_TIME_TO_SECONDS(simulationTime);
                    if (DsVeosCoSim_WriteOutgoingSignal(handle, outgoingSignals[0].id, 1, &value) != DsVeosCoSim_Result_Ok) {
                        DsVeosCoSim_Disconnect(handle);
                        DsVeosCoSim_Destroy(handle);
                        return 1;
                    }
                }
                break;
            }
            case DsVeosCoSim_Command_Start:
                std::cout << "Simulation started at " << DSVEOSCOSIM_SIMULATION_TIME_TO_SECONDS(simulationTime) << " s.\n";
                break;
            case DsVeosCoSim_Command_Stop:
                std::cout << "Simulation stopped at " << DSVEOSCOSIM_SIMULATION_TIME_TO_SECONDS(simulationTime) << " s.\n";
                break;
            case DsVeosCoSim_Command_Pause:
                std::cout << "Simulation paused at " << DSVEOSCOSIM_SIMULATION_TIME_TO_SECONDS(simulationTime) << " s.\n";
                break;
            case DsVeosCoSim_Command_Continue:
                std::cout << "Simulation continued at " << DSVEOSCOSIM_SIMULATION_TIME_TO_SECONDS(simulationTime) << " s.\n";
                break;
        }

        if (DsVeosCoSim_FinishCommand(handle) != DsVeosCoSim_Result_Ok) {
            DsVeosCoSim_Disconnect(handle);
            DsVeosCoSim_Destroy(handle);
            return 1;
        }
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

### 3.3 Creating a second CoSim Client

Create `main2.cpp` in the directory `DsVeosCoSimDemo`:

```cpp
#include <iomanip>
#include <iostream>
#include <sstream>

#include "DsVeosCoSim/DsVeosCoSim.h"

uint32_t ioSignalsCount{};
const DsVeosCoSim_IoSignal* ioSignals{};

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

// Called after every interval specified in the StepSize parameter of the CoSim JSON file
void OnEndStep(DsVeosCoSim_SimulationTime simulationTime, void* userData) {
    uint32_t length{};
    double value{};
    if (DsVeosCoSim_ReadIncomingSignal(userData, ioSignals[0].id, &length, &value) != DsVeosCoSim_Result_Ok) {
        DsVeosCoSim_Disconnect(userData);
        return;
    }

    std::cout << "Signal has the value " << value << " at " << DSVEOSCOSIM_SIMULATION_TIME_TO_SECONDS(simulationTime) << " s.\n";
}

// Called when an I/O signal has changed
void OnSignal(DsVeosCoSim_SimulationTime simulationTime, const DsVeosCoSim_IoSignal* ioSignal, uint32_t length, const void* value, void* userData) {
    std::cout << "Signal changed to the value " << *(double*)value << " at " << DSVEOSCOSIM_SIMULATION_TIME_TO_SECONDS(simulationTime) << " s.\n";
}

int main() {
    DsVeosCoSim_SetLogCallback(OnLogCallback);

    DsVeosCoSim_Handle handle = DsVeosCoSim_Create();
    if (handle == nullptr) {
        return 1;
    }

    DsVeosCoSim_ConnectConfig connectConfig{};
    connectConfig.serverName = "CoSimExample2";
    if (DsVeosCoSim_Connect(handle, connectConfig) != DsVeosCoSim_Result_Ok) {
        DsVeosCoSim_Destroy(handle);
        return 1;
    }

    if (DsVeosCoSim_GetIncomingSignals(handle, &ioSignalsCount, &ioSignals) != DsVeosCoSim_Result_Ok) {
        DsVeosCoSim_Disconnect(handle);
        DsVeosCoSim_Destroy(handle);
        return 1;
    }

    DsVeosCoSim_Callbacks callbacks{};
    callbacks.incomingSignalChangedCallback = OnSignal;
    callbacks.simulationEndStepCallback = OnEndStep;
    callbacks.userData = handle;  // Pass the handle to every callback
    if (DsVeosCoSim_RunCallbackBasedCoSimulation(handle, callbacks) != DsVeosCoSim_Result_Ok) {
        DsVeosCoSim_Disconnect(handle);
        DsVeosCoSim_Destroy(handle);
        return 1;
    }

    DsVeosCoSim_Destroy(handle);
    return 0;
}
```

### 3.4 Update Cmake file

Update `CMakeLists.txt` in the directory `DsVeosCoSimDemo`:

```cmake
cmake_minimum_required(VERSION 3.13)

project(DsVeosCoSimDemo VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_subdirectory(veos-cosim-client)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE DsVeosCoSim)

add_executable(Client2 main2.cpp)
target_link_libraries(Client2 PRIVATE DsVeosCoSim)
```

### 3.5 Build the second CoSim Client

Run the following commands in the directory `DsVeosCoSimDemo`:

```console
cmake ..
cmake --build .
```

### 3.6 Create the second CoSim Server

Create `Example2.json` in the directory `DsVeosCoSimDemo`:

```json
{
    "$schema": "file:///DsVeosCoSim.schema.json",
    "Type": "DsVeosCoSim",
    "Name": "CoSimExample2",
    "StepSize": 0.001,
    "IOSignals": {
        "Incoming": [
            {
                "Name": "Signal1",
                "DataType": "float64",
                "PortName": "Port1"
            }
        ]
    }
}
```

### 3.7 Create the second OSA file

Run the following command in the directory `DsVeosCoSimDemo`:

```console
veos-model import -n ./DsVeosCoSim2.osa -p ./Example2.json
```

### 3.8 Import the first CoSim server

Run the following command in the directory `DsVeosCoSimDemo`:

```console
veos-model import -p ./DsVeosCoSim.osa ./DsVeosCoSim2.osa
```

### 3.9 Connect Signals

Run the following command in the directory `DsVeosCoSimDemo`:

```console
veos-model connect ./DsVeosCoSim2.osa --autoconnect-signals
```

## 4 What it does

The modification of the first client makes the client check at each step if the current simulation time
is a multiple of 10 milliseconds. If yes, it changes the value of the outgoing signal to
the current simulation time in seconds.

The second client performs a callback-based simulation in which the `OnSignal` function is called when the input signal changes.
It then prints the new value and the corresponding simulation time.

The `OnEndStep` function is called at the end of each simulation step. It reads the incoming signal that was received first
and prints its value and the corresponding simulation time.

## 5 Running the Co-Simulation

### 5.1 Load the OSA File

Run the following command in the directory `DsVeosCoSimDemo`:

```console
veos-sim load ./DsVeosCoSim2.osa
```

### 5.2 Run first Client

Run the following command in the directory `DsVeosCoSimDemo`:

```console
./build/Debug/DsVeosCoSimDemo
```

### 5.3 Run second Client

Run the following command in the directory `DsVeosCoSimDemo`:

```console
./build/Debug/Client2
```

### 5.4 Start the Simulation

Run the following command:

```console
veos-sim start
```

## 6 Client output

The output of the second client performing the callback-based simulation looks like this:

```console
Signal has the value 0 at 0 s.
Signal has the value 0 at 0.001 s.
Signal has the value 0 at 0.002 s.
Signal has the value 0 at 0.003 s.
Signal has the value 0 at 0.004 s.
Signal has the value 0 at 0.005 s.
Signal has the value 0 at 0.006 s.
Signal has the value 0 at 0.007 s.
Signal has the value 0 at 0.008 s.
Signal has the value 0 at 0.009 s.
Signal has the value 0 at 0.01 s.
Signal changed to the value 0.01 at 0.011 s.
Signal has the value 0.01 at 0.011 s.
Signal has the value 0.01 at 0.012 s.
Signal has the value 0.01 at 0.013 s.
Signal has the value 0.01 at 0.014 s.
Signal has the value 0.01 at 0.015 s.
Signal has the value 0.01 at 0.016 s.
Signal has the value 0.01 at 0.017 s.
Signal has the value 0.01 at 0.018 s.
Signal has the value 0.01 at 0.019 s.
Signal has the value 0.01 at 0.02 s.
Signal changed to the value 0.02 at 0.021 s.
...
```

## 7 Cleanup

Run the following command:

```console
veos-sim unload
```

## 8 Next Step

Refer to [Step 9: Optional Client](step9-optional-client.md).
