# DISCO-II Cortex-M7 scheduler
Source code and build setup for the Cortex-M7 auxiliary core payload scheduler aboard the DISCO-II student cubesat.

The Cortex-M7 is a secondary core on the NXP i.MX8MP applications processor inside the PicoCore™MX8MP system-on-module (SoM) used in the scientific payload of the DISCO-II student cubesat. In the context of the DISCO-II mission, the Cortex-M7 is used for low power operations while the main Cortex-A53 cores are put into a low power state. This mainly includes GNSS monitoring to time wake-up events for the main cores and orchestration of the satellite's components via programmable procedures in-flight.

## TLDR
Got [docker](https://www.docker.com/)? Great, clone the repository (including submodules), open a shell and run:
```bash
docker compose build && docker compose run build-service
```

This will compile and install a binary into the `bin/` directory, ready to be loaded onto a PicoCore™MX8MP.

#### Environment
The build process is handled by a docker container that contains the necessary GNU Arm Embedded toolchain and libraries to compile the application - see `Dockerfile` for reference. The actual build process is invoked via the `src/armgcc/build-service-start.sh` shell script, and CMake is used to manage the build process (`src/armgcc/CMakeLists.txt`). `src/armgcc/MIMX8ML8xxxxx_cm7_ram.ld` contains the linker script which specifies the memory layout of this application for the Cortex-M7.

## Application
The application relies on FreeRTOS - as provided with the SDK from F&S Elektronik Systeme GmbH (`fsimx8mp-m7-sdk/` subdirectory) - and the following libraries: 
- `libcsp`: CSP, CubeSat Space Protocol, a communication protocol used for network communication between the satellite's components as well as with the ground station. (`lib/csp` subdirectory)

- `libparam`: A library that adds a parameter system to a CSP network. This provides a mechanism for nodes in the network to store and expose parameters over the network. (`lib/param` subdirectory)

- `csp_proc`: A custom-made library that provides a framework for creating and running procedures on the nodes in a CSP network via a register-based virtual machine, utilizing the `libparam` parameter system for its register set. This is used to orchestrate the satellite's components in-flight via programmable procedures sent from the ground station. (`lib/csp_proc` subdirectory)

The custom application code contains the following components:
- `src/main.c`: The main entry point for the application. Initializes the FreeRTOS scheduler and calls the initialization functions for e.g. network, clock and `csp_proc` configuration.
- `src/board_config.c`: Contains configuration of memory regions and RDC.
- `src/clock_config.c`: Contains the clock configuration for the Cortex-M7.
- `src/FreeRTOSConfig.h`: Contains the FreeRTOS configuration, including e.g. heap size and supported features.
- `src/gnss.c`: Contains the GNSS configuration and task for parsing NMEA strings.
- `src/hooks.c`: Contains implementation of periodic tasks such as updating `libparam` parameters with temperature readings.
- `src/lpm.c`: Contains low power mode configuration functionallity.
- `src/network_config.c`: Contains the network configuration for the CSP network, including VMEM and parameter server task initialization for `libparam` to function properly.
    - `src/can_iface.c`: Contains a custom implementation of a CAN driver for the CSP network.
    - `src/si_lock.c`: Platform-specific implementation of semaphore for use with the CSP network.
- `src/param_config.c`: Specifies the parameters that are exposed over the network via `libparam`.
- `src/pin_mux.c`: Contains the pin multiplexing configuration (CAN and UART)
- `src/sys_comm_service.c`: Contains an interface for using the MU to wake up the A53 cores.
- `src/tinygps.c`: Contains functions for parsing NMEA strings from a GNSS module.
- `src/tmu.c`: Contains an interface to the Thermal Management Unit (TMU) on the i.MX8MP for temperature monitoring.
