# phoenix-rtos-project

Sample project using Phoenix-RTOS. Built and tested on minimal clean installation of Ubuntu 18.04.3 LTS for desktops.

### Building

1. Clone the repository and *cd* into it:
````bash
git clone https://github.com/phoenix-rtos/phoenix-rtos-project.git
cd phoenix-rtos-project/
````
2. Initialize and update git submodules:
```bash
git submodule update --init --recursive
```

3. Build and install toolchains for all required target architectures:
````bash
   - Build the toolchain:
	(cd phoenix-rtos-build/toolchain/ && ./build-toolchain.sh i386-pc-phoenix ~/toolchains/i386-pc-phoenix)
	(cd phoenix-rtos-build/toolchain/ && ./build-toolchain.sh arm-phoenix ~/toolchains/arm-phoenix)
	(cd phoenix-rtos-build/toolchain/ && ./build-toolchain.sh riscv64-phoenix-elf ~/toolchains/riscv64-phoenix-elf)

   - Add toolchain binaries to PATH variable:
	export PATH=$PATH:~/toolchains/i386-pc-phoenix/i386-pc-phoenix/bin/
	export PATH=$PATH:~/toolchains/arm-phoenix/arm-phoenix/bin/
	export PATH=$PATH:~/toolchains/riscv64-phoenix-elf/riscv64-phoenix-elf/bin/
````
4. Build the project:
````
   - Edit build.project file and set TARGET variable to define the target architecture

   - Run build.sh script:
	./phoenix-rtos-build/build.sh clean all
````
After the build successfully completes, kernel and disk images will be created and placed in the *_boot* directory.

### Starting the image

To start the created image on target architecture please see [../phoenix-rtos-doc/quickstart](http://phoenix-rtos.com/quickstart) quide.
