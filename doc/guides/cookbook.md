# Cookbook

> [⬆️ Go to Guides](guides.md)

- [Cookbook](#cookbook)
  - [Connect to a Local Server by Name](#connect-to-a-local-server-by-name)
  - [Connect to a Remote Server](#connect-to-a-remote-server)
  - [Use Callback-Based Execution](#use-callback-based-execution)
  - [Use Polling-Based Execution](#use-polling-based-execution)
  - [Read Incoming Signals](#read-incoming-signals)
  - [Write Outgoing Signals](#write-outgoing-signals)
  - [Receive Bus Data Without Callbacks](#receive-bus-data-without-callbacks)

## Connect to a Local Server by Name

```cpp
DsVeosCoSim_ConnectConfig connectConfig{};
connectConfig.serverName = "CoSimExample";
DsVeosCoSim_Result result = DsVeosCoSim_Connect(handle, connectConfig);
```

## Connect to a Remote Server

```cpp
DsVeosCoSim_ConnectConfig connectConfig{};
connectConfig.remoteIpAddress = "192.168.1.50";
connectConfig.serverName = "CoSimExample";
DsVeosCoSim_Result result = DsVeosCoSim_Connect(handle, connectConfig);
```

If the server uses a static TCP port, set `remotePort` explicitly.

## Use Callback-Based Execution

```cpp
DsVeosCoSim_Callbacks callbacks{};
callbacks.simulationEndStepCallback = OnEndStep;
callbacks.userData = &handle;

DsVeosCoSim_Result result = DsVeosCoSim_RunCallbackBasedCoSimulation(handle, callbacks);
```

## Use Polling-Based Execution

```cpp
DsVeosCoSim_Callbacks callbacks{};

if (DsVeosCoSim_StartPollingBasedCoSimulation(handle, callbacks) == DsVeosCoSim_Result_Ok) {
    for (;;) {
        DsVeosCoSim_SimulationTime simulationTime{};
        DsVeosCoSim_Command command{};

        if (DsVeosCoSim_PollCommand(handle, &simulationTime, &command) != DsVeosCoSim_Result_Ok) {
            break;
        }

        if (command == DsVeosCoSim_Command_Terminate) {
            DsVeosCoSim_FinishCommand(handle);
            break;
        }

        DsVeosCoSim_FinishCommand(handle);
    }
}
```

## Read Incoming Signals

```cpp
uint32_t signalCount = 0;
const DsVeosCoSim_IoSignal* signals = nullptr;

if (DsVeosCoSim_GetIncomingSignals(handle, &signalCount, &signals) == DsVeosCoSim_Result_Ok && signalCount > 0) {
    uint32_t length = signals[0].length;
    double value = 0.0;
    DsVeosCoSim_ReadIncomingSignal(handle, signals[0].id, &length, &value);
}
```

## Write Outgoing Signals

```cpp
uint32_t signalCount = 0;
const DsVeosCoSim_IoSignal* signals = nullptr;

if (DsVeosCoSim_GetOutgoingSignals(handle, &signalCount, &signals) == DsVeosCoSim_Result_Ok && signalCount > 0) {
    double value = 42.0;
    DsVeosCoSim_WriteOutgoingSignal(handle, signals[0].id, sizeof(value), &value);
}
```

## Receive Bus Data Without Callbacks

```cpp
DsVeosCoSim_CanMessage message{};
DsVeosCoSim_Result result = DsVeosCoSim_ReceiveCanMessage(handle, &message);

if (result == DsVeosCoSim_Result_Ok) {
    std::cout << "Received CAN message id=" << message.id << "\n";
}

```