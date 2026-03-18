# Quickstart

> [⬆️ Go to Guides](guides.md)

- [Quickstart](#quickstart)
  - [Objective](#objective)
  - [Prerequisites](#prerequisites)
  - [Minimal Workflow](#minimal-workflow)
  - [Example](#example)
  - [What Happens](#what-happens)
  - [Next Steps](#next-steps)

## Objective

This guide shows the shortest practical path for creating a DsVeosCoSim client, connecting it to VEOS, running a callback-based co-simulation, and shutting it down cleanly.

## Prerequisites

- A VEOS CoSim server is already configured and loaded.
- You know either the server name or the static TCP port of the server.
- Your build links against the `DsVeosCoSim` target.

For detailed setup instructions, refer to [How to Prepare the CoSim Demo](../tutorial/prepare.md).

## Minimal Workflow

1. Create a client handle with [DsVeosCoSim_Create](../api-reference/functions/DsVeosCoSim_Create.md).
2. Fill a [DsVeosCoSim_ConnectConfig](../api-reference/structures/DsVeosCoSim_ConnectConfig.md).
3. Connect with [DsVeosCoSim_Connect](../api-reference/functions/DsVeosCoSim_Connect.md).
4. Register callbacks in [DsVeosCoSim_Callbacks](../api-reference/structures/DsVeosCoSim_Callbacks.md).
5. Start the co-simulation with [DsVeosCoSim_RunCallbackBasedCoSimulation](../api-reference/functions/DsVeosCoSim_RunCallbackBasedCoSimulation.md).
6. Disconnect with [DsVeosCoSim_Disconnect](../api-reference/functions/DsVeosCoSim_Disconnect.md) when you want to stop.
7. Free the handle with [DsVeosCoSim_Destroy](../api-reference/functions/DsVeosCoSim_Destroy.md).

## Example

```cpp
#include <iostream>

#include "DsVeosCoSim/DsVeosCoSim.h"

void OnLog(DsVeosCoSim_Severity severity, const char* message) {
    std::cout << static_cast<int>(severity) << ": " << message << "\n";
}

void OnEndStep(DsVeosCoSim_SimulationTime simulationTime, void* userData) {
    DsVeosCoSim_Handle handle = userData;
    std::cout << "Step at " << DSVEOSCOSIM_SIMULATION_TIME_TO_SECONDS(simulationTime) << " s\n";

    if (simulationTime >= DSVEOSCOSIM_SECONDS_TO_SIMULATION_TIME(1.0)) {
        DsVeosCoSim_Disconnect(handle);
    }
}

int main() {
    DsVeosCoSim_SetLogCallback(OnLog);

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
    callbacks.userData = handle;

    DsVeosCoSim_Result result = DsVeosCoSim_RunCallbackBasedCoSimulation(handle, callbacks);
    DsVeosCoSim_Destroy(handle);

    return result == DsVeosCoSim_Result_Disconnected ? 0 : 1;
}
```

## What Happens

- The log callback receives diagnostic messages from the client library.
- The end-step callback runs once per completed step.
- The callback disconnects the client after one simulated second.
- [DsVeosCoSim_RunCallbackBasedCoSimulation](../api-reference/functions/DsVeosCoSim_RunCallbackBasedCoSimulation.md) normally returns [DsVeosCoSim_Result_Disconnected](../api-reference/enumerations/DsVeosCoSim_Result.md) when the co-simulation stops cleanly.

## Next Steps

- Use [Client Lifecycle](lifecycle.md) to understand the allowed call sequence.
- Use [Messages vs. Message Containers](messages-vs-containers.md) before choosing data APIs.
- Use [Callback Ordering and Buffering](callbacks-and-buffering.md) when mixing callbacks and receive APIs.
- Use [Cookbook](cookbook.md) for more specific integration tasks.
