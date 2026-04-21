# DsVeosCoSim_WriteFrMessageToMessageContainer

[⬆️ Go to Functions](functions.md)

- [DsVeosCoSim\_WriteFrMessageToMessageContainer](#dsveoscosim_writefrmessagetomessagecontainer)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)

## Description

Converts a FlexRay message to a FlexRay message container.

## Syntax

```c
DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_WriteFrMessageToMessageContainer(
    const DsVeosCoSim_FrMessage* message,
    DsVeosCoSim_FrMessageContainer* messageContainer
);
```

## Parameters

> const [DsVeosCoSim_FrMessage](../structures/DsVeosCoSim_FrMessage.md)* message

The FlexRay message to convert.

> [DsVeosCoSim_FrMessageContainer](../structures/DsVeosCoSim_FrMessageContainer.md)* messageContainer

The converted FlexRay message container as an out parameter.

## Return values

A [DsVeosCoSim_Result](../enumerations/DsVeosCoSim_Result.md).
