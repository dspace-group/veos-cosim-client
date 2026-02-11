# Step 9: Optional Client

> [⬆️ Go to Tutorial](tutorial.md)

- [Step 9: Optional Client](#step-9-optional-client)
  - [1 Introduction](#1-introduction)
  - [2 Preconditions](#2-preconditions)
  - [3 Modifying the CoSim server](#3-modifying-the-cosim-server)
    - [3.1 Modify JSON](#31-modify-json)
    - [3.2 Update the OSA file](#32-update-the-osa-file)
  - [4 What it does](#4-what-it-does)
  - [5 Running the Co-Simulation](#5-running-the-co-simulation)
    - [5.1 Load the OSA File](#51-load-the-osa-file)
    - [5.2 Start the Simulation](#52-start-the-simulation)
    - [5.3 Check the Simulation state](#53-check-the-simulation-state)
    - [5.4 Run Client](#54-run-client)
    - [5.5 Terminate Client](#55-terminate-client)
    - [5.6 Check the Simulation state](#56-check-the-simulation-state)
  - [Client output](#client-output)
  - [7 Cleanup](#7-cleanup)

## 1 Introduction

By default, a co-simulation can only be started if a CoSim client is connected to a server in VEOS.
However, by marking a client as optional in the JSON interface description file of the CoSim server,
the simulation can be started even when no client is connected.

## 2 Preconditions

You must have run [Step 8: Handling I/O Signals](step8-io-signals.md).

## 3 Modifying the CoSim server

### 3.1 Modify JSON

Update `Example.json` to add the property `IsClientOptional`:

```json
{
    "$schema": "file:///DsVeosCoSim.schema.json",
    "Type": "DsVeosCoSim",
    "Name": "CoSimExample",
    "StepSize": 0.001,
    "IsClientOptional": true,
    "Network": {
    "CAN": {
        "CanController": {
            "ClusterName": "HighSpeedCanCluster",
            "BitsPerSecond": 500000,
            "FlexibleDataRateBitsPerSecond": 2000000
        }
    }
    },
    "IOSignals": {
        "Outgoing": [
            {
                "Name": "Signal1",
                "DataType": "float64",
                "PortName": "Port1"
            }
        ]
    }
}
```

### 3.2 Update the OSA file

Run the following command in the directory `DsVeosCoSimDemo`:

```console
veos-model import -n ./DsVeosCoSim.osa -p ./Example.json
```

## 4 What it does

This change lets the simulation start without a connection. Also, if the connection is lost, the simulation continues.

## 5 Running the Co-Simulation

### 5.1 Load the OSA File

Run the following command in the directory `DsVeosCoSimDemo`:

```console
veos-sim load ./DsVeosCoSim.osa
```

### 5.2 Start the Simulation

Run the following command:

```console
veos-sim start
```

### 5.3 Check the Simulation state

Run the following command:

```console
veos-sim info -p State
```

### 5.4 Run Client

Run the following command in the directory `DsVeosCoSimDemo`:

```console
./build/Debug/DsVeosCoSimDemo
```

### 5.5 Terminate Client

Terminate the client executable by pressing `Ctrl`+`C`.

### 5.6 Check the Simulation state

Run the following command:

```console
veos-sim info -p State
```

## Client output

The output of the command `veos-sim info -p State` should both time show the following output:

```console
Running
```

## 7 Cleanup

Run the following command:

```console
veos-sim unload
```
