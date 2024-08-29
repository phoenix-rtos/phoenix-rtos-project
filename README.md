# phoenix-rtos-project

[![ci](https://github.com/phoenix-rtos/phoenix-rtos-project/actions/workflows/ci.yml/badge.svg)](https://github.com/phoenix-rtos/phoenix-rtos-project/actions/workflows/ci.yml)
[![nightly](https://github.com/phoenix-rtos/phoenix-rtos-project/actions/workflows/nightly.yml/badge.svg)](https://github.com/phoenix-rtos/phoenix-rtos-project/actions/workflows/nightly.yml)

Phoenix-RTOS is a scalable real-time operating system for IoT. It is based on its own microkernel and can be used either on small devices based on microcontrollers and on advanced computer systems based on multiple processors and equipped with gigabytes of RAM.

The POSIX application environment can be emulated to enable the execution of regular UN*X applications. The ARINC653 execution environment (APEX), advanced partitioning and DO-178C certification package for aerospace applications are under development.

Phoenix-RTOS supports multiple architectures, including ARM Cortex-M, ARM Cortex-A, Intel x86, RISC-V and some popular microcontrollers and reference boards. The system is still under development but it was implemented in numerous Smart Utility appliances e.g. in smart gas meters, smart energy meters and data concentrators (DCU).

This repository contains sample Phoenix-RTOS project. It aggregates all system components - kernel, standard library, device drivers, filesystems, loader, building tools. After building for target platform the final image is created and can be run on hardware or on emulators. The project was built and tested on minimal clean installation of Ubuntu 22.04.2 LTS for desktops.

## Building

The building process was described in [phoenix-rtos-doc/building](https://docs.phoenix-rtos.com/building/index.html).

## Running

To launch the created image on target architecture please see [phoenix-rtos-doc/quickstart](https://docs.phoenix-rtos.com/quickstart/index.html).

## Documentation

Phoenix-RTOS philosophy, architecture and internals are described in [phoenix-rtos-doc](https://docs.phoenix-rtos.com).

## Product website

Phoenix-RTOS website: [phoenix-rtos.com](https://phoenix-rtos.com).
