# How to Prepare the CoSim Demo

> [⬆️ Go to Tutorial](tutorial.md)

- [How to Prepare the CoSim Demo](#how-to-prepare-the-cosim-demo)
  - [1 Objective](#1-objective)
  - [2 Prepare the Environment](#2-prepare-the-environment)
  - [3 Set up the CoSim server](#3-set-up-the-cosim-server)
    - [3.1 Create a Working Directory](#31-create-a-working-directory)
    - [3.2 Create a JSON File](#32-create-a-json-file)
    - [3.3 Create an OSA file](#33-create-an-osa-file)
    - [3.4 Load the OSA to the Simulator](#34-load-the-osa-to-the-simulator)
  - [4 Interim result](#4-interim-result)
  - [5 Set up the CoSim client](#5-set-up-the-cosim-client)
    - [5.1 Download VEOS CoSim Client](#51-download-veos-cosim-client)
    - [5.2 Create Make instructions](#52-create-make-instructions)
    - [5.3 Create Source Code](#53-create-source-code)
    - [5.4 Create compile\_flags.txt](#54-create-compile_flagstxt)
    - [5.4 Build the Client](#54-build-the-client)
    - [5.5 Run Client](#55-run-client)
  - [6 Result](#6-result)
  - [7 Next Step](#7-next-step)

## 1 Objective

To prepare the CoSim demo by performing the following tasks:

- Prepare the environment.

- Setting up the CoSim example server.

- Setting up a CoSim test client.

## 2 Prepare the Environment

If you are using Powershell on Windows, run the following command:

```console
$env:Path = '<VEOS installation directory>\bin;' + $env:Path
```

If you are using cmd on Windows, run the following command:

```console
set PATH=<VEOS installation directory>\bin;%PATH%
```

If you are using bash on Linux, run the following command:

```console
export PATH="<VEOS installation directory>/bin:$PATH"
```

## 3 Set up the CoSim server

### 3.1 Create a Working Directory

 Create the directory `DsVeosCoSimDemo` and change the current directory to the new directory:

```console
mkdir DsVeosCoSimDemo
cd DsVeosCoSimDemo
```

### 3.2 Create a JSON File

Create the file `DsVeosCoSimDemo/Example.json` with the following content:

```json
{
    "$schema": "file:///DsVeosCoSim.schema.json",
    "Type": "DsVeosCoSim",
    "Name": "CoSimExample",
    "StepSize": 0.001,
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

### 3.3 Create an OSA file

Run the following command in the directory `DsVeosCoSimDemo`:

```console
veos-model import -n ./DsVeosCoSim.osa -p ./Example.json
```

### 3.4 Load the OSA to the Simulator

Run the following command in the directory `DsVeosCoSimDemo`:

```console
veos-sim load ./DsVeosCoSim.osa
```

## 4 Interim result

You created a JSON interface description file for a CoSim server, created an OSA in VEOS, imported the JSON file into the OSA, and loaded it to the simulator.

The CoSim server waits for a client to connect.

## 5 Set up the CoSim client

### 5.1 Download VEOS CoSim Client

Download the latest `veos-cosim-client-<version>.zip` archive from [veos-cosim-client](https://github.com/dspace-group/veos-cosim-client/releases), extract it, rename the extracted directory to `veos-cosim-client` and copy it content to the directory `DsVeosCoSimDemo`.

### 5.2 Create Make instructions

In the `DsVeosCoSimDemo` directory, create the `CMakeLists.txt` file with the following content:

```cmake
cmake_minimum_required(VERSION 3.13)

project(DsVeosCoSimDemo VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_subdirectory(veos-cosim-client)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE DsVeosCoSim)
```

### 5.3 Create Source Code

Also in the `DsVeosCoSimDemo` directory, create the `main.cpp` file with the following content:

```cpp
#include <iomanip>
#include <iostream>
#include <sstream>

#include "DsVeosCoSim/DsVeosCoSim.h"

int main() {
    return 0;
}
```

### 5.4 Create compile_flags.txt

In the `DsVeosCoSimDemo` directory, create the `compile_flags.txt`file with the following content:

```
-xc++
-std=c++17
-Iveos-cosim-client/include
```

### 5.4 Build the Client

In the `DsVeosCoSimDemo` directory, run the following commands to create a `build` directory, configure the build project, and compile the test executable:

```console
mkdir build
cmake . -B build
cmake --build ./build
```

> [!Note]
>
> If you use Visual Studio on Windows, make sure to run these commands in the Visual Studio Developer Command Prompt.

### 5.5 Run Client

Run the executable by running the following command in the directory `DsVeosCoSimDemo`:

```console
./build/Debug/DsVeosCoSimDemo
```

The executable simply returns without any message.

## 6 Result

You set up a basic CoSim server in VEOS and set up a build project for CoSim clients. You also created a simple test executable to confirm the build project was configured correctly.

## 7 Next Step

Refer to [Step 1: Setting Up a Basic Co-Simulation](step1-basic.md).
