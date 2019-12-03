# phoenix-rtos-project
Sample project using Phoenix-RTOS

### Building

1. Clone the repository and *cd* into it.
```bash
git clone https://github.com/phoenix-rtos/phoenix-rtos-project.git
cd phoenix-rtos-project/
```

2. Initialize and update git submodules.
```bash
git submodule update --init --recursive
```

3. Run build.sh script.
```bash
./phoenix-rtos-build/build.sh clean all
```

After the build successfully completes, *phoenix-ia32.ext2* image file will be created and placed in the *_boot* directory.

### Starting the VM

- using QEMU:
   - **basic**
	1. Run ia32-qemu-basic.sh script.
		```bash
		./ia32-qemu-basic.sh
		```

   - **with networking**

		We're going to create a NAT-based network with a little help from *qemu-bridge-helper*.
	
	1. Run ```ifconfig``` and make sure *virbr0* bridged interface exists on the host (it should be created with QEMU installation).
	
	2. The setuid attribute needs to be turned on for the default network helper.
		```bash
		sudo chmod u+s /usr/local/libexec/qemu-bridge-helper
		```
	
	3. Allow connection through *virbr0* interface.
	Add ```allow virbr0``` line to the *bridge.conf* file (it's typically located in */etc/qemu/* or */usr/local/etc/qemu/*).

	1. Run ia32-qemu-network.sh script.
		```bash
		./ia32-qemu-network.sh
		```
