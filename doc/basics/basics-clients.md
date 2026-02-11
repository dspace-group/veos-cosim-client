# Basics on CoSim Clients

> [⬆️ Go to Basics on Co-Simulation](basics.md)

- [Basics on CoSim Clients](#basics-on-cosim-clients)
  - [Introduction](#introduction)
  - [Workflow](#workflow)
  - [Connecting to a CoSim server](#connecting-to-a-cosim-server)
  - [Callback-based vs. polling-based co-simulation](#callback-based-vs-polling-based-co-simulation)
  - [Basics on callbacks](#basics-on-callbacks)
  - [Basics on timing](#basics-on-timing)

## Introduction

You have to implement a CoSim client in C++ code that uses the CoSim API to interact with the CoSim server.

You can then integrate the client as a submodule in the project containing your binary (recommended) or precompile the CoSim sources as a static library to link it to your binary.

If you want to access the co-simulation interface via Python, you can also precompile the client sources as a dynamic library.

## Workflow

The following steps outline the workflow for creating a CoSim client:

1. Download the latest `veos-cosim-client-<version>.zip` archive from [veos-cosim-client](https://github.com/dspace-group/veos-cosim-client) and extract it to a dedicated folder, e.g., `DsVeosCoSim/ThirdParty/veos-cosim-client` in your home directory.

2. Depending on your use case, take the following steps:

   - For including the client as a submodule in your project, refer to Example: Setting Up a Basic Co-Simulation.

   - For precompiling as a dynamic library, execute the following commands in the folder to which you extracted the DsVeosCoSim.zip archive:

     ```console
     mkdir Debug
     cd Debug
     cmake .. -DBUILD_SHARED_LIBS=ON
     cmake --build . --config=Debug
     ```

     This produces a dynamic library file in the Src/Debug subdirectory.

     Replace Debug with Release in the listing above to create a release version.

   - For precompiling as a static library, execute the following commands in the folder to which you extracted the DsVeosCoSim.zip archive:

     ```console
     mkdir Debug
     cd Debug
     cmake ..
     cmake --build . --config Debug
     cmake --install . --config Debug --prefix ../install
     ```

     This produces a static library file in the install subdirectory.

     Replace Debug with Release in the listing above to create a release version.

## Connecting to a CoSim server

By default, a CoSim client asks the CoSim port mapper for the actual TCP port of the related CoSim server and then connects to this port. For this to work, you have to specify the server name via [DsVeosCoSim_ConnectConfig.serverName](../api-reference/structures/DsVeosCoSim_ConnectConfig.md).

However, if you specified a static TCP port for the CoSim server, you have to explicitly connect to this port by providing it via [DsVeosCoSim_ConnectConfig.remotePort](../api-reference/structures/DsVeosCoSim_ConnectConfig.md) in the implementation of your client.

If you specify both `serverName` and `remotePort`, `remotePort` is used to establish the connection.

If the server is running on a different host, you must specify the IP address using [DsVeosCoSim_ConnectConfig.remoteIpAddress](../api-reference/structures/DsVeosCoSim_ConnectConfig.md).

The local port is also created dynamically by default. For special cases like creating a tunnel between client and server, you can overwrite this with a specific port using [DsVeosCoSim_ConnectConfig.localPort](../api-reference/structures/DsVeosCoSim_ConnectConfig.md).

> [!Tip]
>
> You can use [DsVeosCoSim_ConnectConfig.clientName](../api-reference/structures/DsVeosCoSim_ConnectConfig.md) to provide a name for the client that can be used in VEOS messages for better readability. For example, if you set `DsVeosCoSim_ConnectConfig.clientName = "CustomClient"`, a message might look like this: `dSPACE VEOS CoSim client 'CustomClient' at 127.0.0.1:56248 connected.`.

## Callback-based vs. polling-based co-simulation

You can configure the CoSim client for two different co-simulation modes:

- Callback-based: This is a blocking simulation which only stops when the client is disconnected from the server. While it is running, the registered callbacks are processed for each simulation step.

  During each callback, VEOS does not advance the simulation until the callback is finished.

  In the absence of errors, a callback-based co-simulation only returns when the client is disconnected from the server.

  For a basic example, refer to [Step 1: Setting Up a Basic Co-Simulation](../tutorial/step1-basic.md).

- Polling-based: The simulation polls for commands and performs the actions that are specified in the implementation for each command. This can also include callbacks, which are received in the same order as for a callback-based co-simulation.

  This mode can be useful in cases where the CoSim server has to communicate with another API that is callback-based so you do not have to take care of synchronizing the callbacks.

  For an example of implementing a polling-based co-simulation, refer to [Step 7: Running a Polling-Based Simulation](../tutorial/step7-poll.md).

  > **Note**
  >
  > Once one of these modi is selected, it cannot be changed until the client is disconnected from the server.

## Basics on callbacks

You can use callback functions to get information on state changes of the simulation and on data exchange:

- State changes: Started, stopped, paused, continued, terminated

- Steps: begin and end step

- Bus message received: CAN, Ethernet, LIN

- I/O signal changed (for incoming signals)

Callbacks are optional. However, not registering any callbacks makes sense only for polling-based simulations.

Each time the server notifies the client of a new step, the callbacks are received in the following order:

- Begin step

- Bus message and I/O signal callbacks in no specific order

- End step

## Basics on timing

Each time VEOS receives the simulation start or continue command, the server begins to send steps to the client and stops when a simulation stop, pause, or terminate command is received.

The first step always takes place at simulation time 0. After that, the simulation is advanced by a step after each time interval specified in the StepSize property in the CoSim server JSON file.

The client can intervene via the [DsVeosCoSim_SetNextSimulationTime](../api-reference/functions/DsVeosCoSim_SetNextSimulationTime.md) function, which specifies the condition for the server to send the next step. This can be done for each step or just from time to time. Once the server no longer receives this condition, it proceeds with the predefined step size. If the time specified via the [DsVeosCoSim_SetNextSimulationTime](../api-reference/functions/DsVeosCoSim_SetNextSimulationTime.md) function is in the past or equal to the present time, it is ignored.

If no step size is specified in the JSON file, the server sends only one step at simulation time 0. The simulation is only advanced if the client executes the [DsVeosCoSim_SetNextSimulationTime](../api-reference/functions/DsVeosCoSim_SetNextSimulationTime.md) function. Otherwise, the simulation in VEOS continues without the co-simulator.

Bus messages and I/O signals are always sent between server and client in the context of a step.
