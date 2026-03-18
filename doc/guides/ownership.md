# Ownership and Lifetime Rules

> [⬆️ Go to Guides](guides.md)

- [Ownership and Lifetime Rules](#ownership-and-lifetime-rules)
  - [Handle Ownership](#handle-ownership)
  - [Descriptor Arrays Returned by Get Functions](#descriptor-arrays-returned-by-get-functions)
  - [Callback Arguments](#callback-arguments)
  - [Message and Signal Buffers](#message-and-signal-buffers)
  - [Practical Recommendations](#practical-recommendations)

## Handle Ownership

- [DsVeosCoSim_Create](../api-reference/functions/DsVeosCoSim_Create.md) creates a handle owned by the caller.
- Release that handle with [DsVeosCoSim_Destroy](../api-reference/functions/DsVeosCoSim_Destroy.md).
- Do not free the handle yourself.

## Descriptor Arrays Returned by Get Functions

The following functions return pointers to arrays owned by the client handle:

- [DsVeosCoSim_GetIncomingSignals](../api-reference/functions/DsVeosCoSim_GetIncomingSignals.md)
- [DsVeosCoSim_GetOutgoingSignals](../api-reference/functions/DsVeosCoSim_GetOutgoingSignals.md)
- [DsVeosCoSim_GetCanControllers](../api-reference/functions/DsVeosCoSim_GetCanControllers.md)
- [DsVeosCoSim_GetEthControllers](../api-reference/functions/DsVeosCoSim_GetEthControllers.md)
- [DsVeosCoSim_GetLinControllers](../api-reference/functions/DsVeosCoSim_GetLinControllers.md)
- [DsVeosCoSim_GetFrControllers](../api-reference/functions/DsVeosCoSim_GetFrControllers.md)

Usage rules:

- Do not modify these arrays.
- Do not free these arrays.
- Reacquire them after connecting again with the same handle.
- Do not rely on previously returned pointers after reconnecting or destroying the handle.

If your application needs long-term ownership, copy the data.

## Callback Arguments

Treat all pointers passed into callbacks as callback-scoped data.

This applies to pointers such as:

- controller pointers
- signal pointers
- message pointers
- message-container pointers

If you need the data after the callback returns, copy it during the callback.

## Message and Signal Buffers

- Receive functions fill caller-provided output structures.
- Signal read and write functions operate on caller-provided buffers.
- For [DsVeosCoSim_WriteOutgoingSignal](../api-reference/functions/DsVeosCoSim_WriteOutgoingSignal.md), the caller retains ownership of the provided value buffer.

## Practical Recommendations

- Copy descriptor data into your own containers if you cache it.
- Copy callback data if another thread or another part of the application needs it later.
- Requery signal and controller lists after each successful connect.
