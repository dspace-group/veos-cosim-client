# Tutorial

> [⬆️ Go to DsVeosCoSim Documentation](../documentation.md)

- [Tutorial](#tutorial)
  - [Introduction](#introduction)
  - [Workflow](#workflow)
  - [List of Tutorial Steps](#list-of-tutorial-steps)
    - [Requirements](#requirements)

## Introduction

The VEOS CoSim tutorial provides instructions for setting up a simple example CoSim server and implementing several kinds of CoSim clients to illustrate the basic co-simulation functionalities.

## Workflow

The VEOS CoSim demo leads you through the following steps:

- Create a simple JSON interface description file for a CoSim server

- Create an OSA in VEOS, import the JSON file, and load it to the simulator

- Implement a basic CoSim client and start a co-simulation

- Modify the basic CoSim client to familiarize yourself with the principal aspects of co-simulation

## List of Tutorial Steps

> [How to Prepare the CoSim Demo](prepare.md)

How to prepare the CoSim demo.

> [Step 1: Setting Up a Basic Co-Simulation](step1-basic.md)

Enable basic logging.

> [Step 2: Implementing Simulation State Change Callbacks](step2-state-change.md)

Tracking simulation state changes.

> [Step 3: Accessing the Data Interface](step3-data-interface.md)

Getting information on available bus controllers and I/O signals.

> [Step 4: Using Data Callback Functions to Get Information on CAN Bus Messages](step4-callback.md)

Getting information on newly received bus messages.

> [Step 5: Sending Data](step5-send.md)

Sending data to VEOS.

> [Step 6: Receiving Data](step6-receive.md)

Receiving data from VEOS.

> [Step 7: Running a Polling-Based Simulation](step7-poll.md)

Running a polling-based co-simulation.

> [Step 8: Handling I/O Signals](step8-io-signals.md)

Handling I/O signal communication for two client-server pairs.

> [Step 9: Optional Client](step9-optional-client.md)

Simulation behavior for a CoSim server for which a client is optional.

### Requirements

You can work with the demo either on Windows 10 or newer or on Ubuntu 20.04 LTS or later.

Additionally, you must have the following:

- CMake 3.15 or higher

- Either of the following compilers:

  - GCC 7 or higher

  - MSVC 2022 or higher

  - Clang 10 or higher
