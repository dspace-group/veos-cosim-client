# DsVeosCoSim_WriteFrMessageContainerToMessage

[⬆️ Go to Functions](functions.md)

- [DsVeosCoSim\_WriteFrMessageContainerToMessage](#dsveoscosim_writefrmessagecontainertomessage)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)

## Description

Converts a FlexRay message container to a FlexRay message.

## Syntax

```c
DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_WriteFrMessageContainerToMessage(
    const DsVeosCoSim_FrMessageContainer* messageContainer,
    DsVeosCoSim_FrMessage* message
);
```

## Parameters

> const [DsVeosCoSim_FrMessageContainer](../structures/DsVeosCoSim_FrMessageContainer.md)* messageContainer

The FlexRay message container to convert.

> [DsVeosCoSim_FrMessage](../structures/DsVeosCoSim_FrMessage.md)* message

The converted FlexRay message as an out parameter.

## Return values

A [DsVeosCoSim_Result](../enumerations/DsVeosCoSim_Result.md).
