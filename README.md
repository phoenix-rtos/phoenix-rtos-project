<a href="https://github.com/phoenix-rtos/phoenix-rtos-project/actions?query=workflow%3Aci"><img alt="CI build" src="https://github.com/phoenix-rtos/phoenix-rtos-project/workflows/ci/badge.svg"></a>

# phoenix-rtos-project

Sample project using Phoenix-RTOS. Built and tested on minimal clean installation of Ubuntu 18.04.3 LTS for desktops.

## Obtaining sources

Clone the repository **recursively** (to get the submodules):
```bash
git clone --recursive https://github.com/phoenix-rtos/phoenix-rtos-project.git
```

## Building

### Available targets
If You don't provide a valid `TARGET` via ENV, all possible targets will be listed, eg:

```bash
$ ./phoenix-rtos-build/build.sh
TARGET variable not set
Please specify a valid traget by setting TARGET variable to one of:
armv7a7-imx6ull
armv7m4-stm32l4x6
armv7m7-imxrt105x
armv7m7-imxrt106x
ia32-generic
riscv64-spike
riscv64-virt
```

### Using docker

This is the quickest way to start development - all necessary tools are distributed in docker image.

To build - provide a `TARGET` via ENV variable:
```bash
cd phoenix-rtos-project/
TARGET=ia32-generic ./docker-build.sh clean all
```

### Using native toolchain

You need to compile the toolchains Yourself:
1. Build and install toolchains for all required target architectures:
   - Build the toolchain:
    ```bash
    cd phoenix-rtos-project
    (cd phoenix-rtos-build/toolchain/ && ./build-toolchain.sh i386-pc-phoenix ~/toolchains/i386-pc-phoenix)
    (cd phoenix-rtos-build/toolchain/ && ./build-toolchain.sh arm-phoenix ~/toolchains/arm-phoenix)
    (cd phoenix-rtos-build/toolchain/ && ./build-toolchain.sh riscv64-phoenix ~/toolchains/riscv64-phoenix)
    ```

   - Add toolchain binaries to PATH variable:
    ```bash
    export PATH=$PATH:~/toolchains/i386-pc-phoenix/i386-pc-phoenix/bin/
    export PATH=$PATH:~/toolchains/arm-phoenix/arm-phoenix/bin/
    export PATH=$PATH:~/toolchains/riscv64-phoenix/riscv64-phoenix/bin/
    ```

2. Build project - provide a `TARGET` via ENV variable:
```bash
TARGET=ia32-generic ./phoenix-rtos-build/build.sh clean all
```
After the build successfully completes, kernel and disk images will be created and placed in the *_boot* directory.

## Starting the image

To start the created image on target architecture please see [phoenix-rtos-doc/quickstart](https://github.com/phoenix-rtos/phoenix-rtos-doc/tree/master/quickstart) guide.
