# DsVeosCoSim_GetRoundTripTime

[⬆️ Go to Functions](functions.md)

- [DsVeosCoSim\_GetRoundTripTime](#dsveoscosim_getroundtriptime)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)

## Description

Gets the round trip time.

## Syntax

```c
DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_GetRoundTripTime(
    DsVeosCoSim_Handle handle,
    int64_t* roundTripTimeInNanoseconds
);
```

## Parameters

> [DsVeosCoSim_Handle](../simple-types/DsVeosCoSim_Handle.md) handle

The handle of the VEOS CoSim client.

> int64_t* roundTripTimeInNanoseconds

The round trip time in nanoseconds as an out parameter.

## Return values

A [DsVeosCoSim_Result](../enumerations/DsVeosCoSim_Result.md).

*** Add File: c:\repos\github\veos-cosim-client\doc\api-reference\functions\DsVeosCoSim_StartSimulation.md
# DsVeosCoSim_StartSimulation

[⬆️ Go to Functions](functions.md)

- [DsVeosCoSim\_StartSimulation](#dsveoscosim_startsimulation)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)

## Description

Starts the simulation.

## Syntax

```c
DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_StartSimulation(
    DsVeosCoSim_Handle handle
);
```

## Parameters

> [DsVeosCoSim_Handle](../simple-types/DsVeosCoSim_Handle.md) handle

The handle of the VEOS CoSim client.

## Return values

A [DsVeosCoSim_Result](../enumerations/DsVeosCoSim_Result.md).

*** Add File: c:\repos\github\veos-cosim-client\doc\api-reference\functions\DsVeosCoSim_StopSimulation.md
# DsVeosCoSim_StopSimulation

[⬆️ Go to Functions](functions.md)

- [DsVeosCoSim\_StopSimulation](#dsveoscosim_stopsimulation)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)

## Description

Stops the simulation.

## Syntax

```c
DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_StopSimulation(
    DsVeosCoSim_Handle handle
);
```

## Parameters

> [DsVeosCoSim_Handle](../simple-types/DsVeosCoSim_Handle.md) handle

The handle of the VEOS CoSim client.

## Return values

A [DsVeosCoSim_Result](../enumerations/DsVeosCoSim_Result.md).

*** Add File: c:\repos\github\veos-cosim-client\doc\api-reference\functions\DsVeosCoSim_PauseSimulation.md
# DsVeosCoSim_PauseSimulation

[⬆️ Go to Functions](functions.md)

- [DsVeosCoSim\_PauseSimulation](#dsveoscosim_pausesimulation)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)

## Description

Pauses the simulation.

## Syntax

```c
DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_PauseSimulation(
    DsVeosCoSim_Handle handle
);
```

## Parameters

> [DsVeosCoSim_Handle](../simple-types/DsVeosCoSim_Handle.md) handle

The handle of the VEOS CoSim client.

## Return values

A [DsVeosCoSim_Result](../enumerations/DsVeosCoSim_Result.md).