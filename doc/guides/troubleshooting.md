# Troubleshooting

> [⬆️ Go to Guides](guides.md)

- [Troubleshooting](#troubleshooting)
  - [Connection Problems](#connection-problems)
  - [Co-Simulation Mode Problems](#co-simulation-mode-problems)
  - [Polling Problems](#polling-problems)
  - [Data Exchange Problems](#data-exchange-problems)
  - [Result Codes](#result-codes)

## Connection Problems

Problem: `DsVeosCoSim_Connect` fails.

Check:

- `serverName` is set when using the port mapper.
- `remotePort` is set when connecting to a static port.
- `remoteIpAddress` is set when the server is not local.
- the VEOS simulator has loaded the OSA and the CoSim server is available.

Problem: The wrong server is reached or the connection goes to the wrong endpoint.

Check:

- If both `serverName` and `remotePort` are specified, the configured `remotePort` is used for the connection.
- The VEOS `bin` directory is available on `PATH` when using the VEOS command-line tools shown in the tutorial.

## Co-Simulation Mode Problems

Problem: The client does not return from callback-based mode.

Expected behavior:

- [DsVeosCoSim_RunCallbackBasedCoSimulation](../api-reference/functions/DsVeosCoSim_RunCallbackBasedCoSimulation.md) blocks until the client disconnects or the server unloads the session.

Problem: The wrong mode was started.

Action:

- Disconnect and reconnect the handle before switching between callback-based and polling-based execution.

## Polling Problems

Problem: `PollCommand` fails after a previous successful poll.

Likely cause:

- The previous command was not finished with [DsVeosCoSim_FinishCommand](../api-reference/functions/DsVeosCoSim_FinishCommand.md).

Problem: `FinishCommand` fails.

Likely cause:

- No current command is pending because `PollCommand` was not called successfully first.

## Data Exchange Problems

Problem: Receive functions do not return the expected bus data.

Check:

- A callback for the same bus type is not already registered.
- You are not mixing message and message-container processing styles unintentionally.

Problem: Signal access fails.

Check:

- The signal ID was obtained from the currently connected handle.
- The provided buffer length matches the signal data you expect to read or write.

Problem: Transmit calls return a full-buffer result.

Likely cause:

- The client-side bus buffer is full for the corresponding controller.

## Result Codes

Common result codes:

- [DsVeosCoSim_Result_Ok](../api-reference/enumerations/DsVeosCoSim_Result.md): The operation succeeded.
- [DsVeosCoSim_Result_Disconnected](../api-reference/enumerations/DsVeosCoSim_Result.md): Normal termination for callback-based co-simulation or a disconnected session.
- [DsVeosCoSim_Result_Error](../api-reference/enumerations/DsVeosCoSim_Result.md): A protocol, sequencing, or connection problem occurred.
- [DsVeosCoSim_Result_InvalidArgument](../api-reference/enumerations/DsVeosCoSim_Result.md): A required pointer or argument was invalid.
- [DsVeosCoSim_Result_Empty](../api-reference/enumerations/DsVeosCoSim_Result.md): No buffered data is currently available.
- [DsVeosCoSim_Result_Full](../api-reference/enumerations/DsVeosCoSim_Result.md): A transmit buffer is full.
