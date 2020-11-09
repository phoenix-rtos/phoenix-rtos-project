<a href="https://github.com/phoenix-rtos/phoenix-rtos-project/actions?query=workflow%3Aci"><img alt="CI build" src="https://github.com/phoenix-rtos/phoenix-rtos-project/workflows/ci/badge.svg"></a>

# phoenix-rtos-project

Phoenix-RTOS is a scalable real-time operating system for IoT. It is based on its own microkernel and can be used either on small devices based on microcontrollers and on advanced computer systems based on multiple processors and equipped with gigabytes of RAM.

The POSIX application environment can be emulated to enable the execution of regular UN*X application. The ARINC653 execution environment (APEX), advanced partitioning and DO-178C certification packaged for aerospace applications are under development.

Phoenix-RTOS supports multiple architectures, including ARM Cortex-M, ARM Cortex-A, Intel x86, RISC-V and some popoular microcontrollers and reference boards. The system is still under development but it was implemented in numerous Smart Utility appliances e.g. in smart gas meters, smart energy meters and data concetrators (DCU).

This repository contains sample Phoenix-RTOS project. It aggregates all system components - kernel, standard library, device drivers, filesystems, loader, building tools. After building for target platform the final image is created and can by run on hardware or on emulators. The project was built and tested on minimal clean installation of Ubuntu 18.04.3 LTS for desktops.

## Building
The building process was described in [phoenix-rtos-doc/building/](https://github.com/phoenix-rtos/phoenix-rtos-doc/blob/master/building/README.md).

## Running
To launch the created image on target architecture please see [phoenix-rtos-doc/quickstart/](https://github.com/phoenix-rtos/phoenix-rtos-doc/blob/master/quickstart/README.md).

## Documentation
Phoenix-RTOS philosophy, architecture and insights can be found here [phoenix-rtos-doc/](https://github.com/phoenix-rtos/phoenix-rtos-doc/blob/master/README.md).

## Product website
Phoenix-RTOS website: [phoenix-rtos.com](https://phoenix-rtos.com).
