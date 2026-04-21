# DsVeosCoSim_LinMessageToString

[⬆️ Go to Functions](functions.md)

- [DsVeosCoSim\_LinMessageToString](#dsveoscosim_linmessagetostring)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)

## Description

Formats a LIN message as a string.

This function is only available in C++.

## Syntax

```cpp
extern DSVEOSCOSIM_API std::string DsVeosCoSim_LinMessageToString(
    const DsVeosCoSim_LinMessage* message
);
```

## Parameters

> const [DsVeosCoSim_LinMessage](../structures/DsVeosCoSim_LinMessage.md)* message

The LIN message to format.

## Return values

> std::string

A string representation of the LIN message.
