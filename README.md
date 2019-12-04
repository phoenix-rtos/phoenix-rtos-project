# phoenix-rtos-project
Sample project using Phoenix-RTOS

### Building

1. Clone the repository and *cd* into it:
```bash
git clone https://github.com/phoenix-rtos/phoenix-rtos-project.git
cd phoenix-rtos-project/
```

2. Initialize and update git submodules:
```bash
git submodule update --init --recursive
```

3. Build and install *i386-pc-phoenix* toolchain, if you haven't already:
   - Install necessary packages:
	```bash
	sudo apt-get install m4
	```
   - Build the toolchain:
	```bash
	./phoenix-rtos-kernel/toolchain/build-toolchain.sh i386-pc-phoenix ~/toolchains/i386-pc-phoenix
	```
   - Add toolchain binaries to $PATH:
	```bash
	export PATH=$PATH:~/toolchains/i386-pc-phoenix/i386-pc-phoenix/bin/
	```

4. Build the sample project:
   - Install necessary packages:
	```bash
	# Required by GRUB
	sudo apt-get install bison flex
	```
   - Run build.sh script:
	```bash
	./phoenix-rtos-build/build.sh clean all
	```

After the build successfully completes, *phoenix-ia32.ext2* image file will be created and placed in the *_boot* directory.

### Starting the VM

- with QEMU:

	First, we're going to set up a NAT-based network using *libvirt* and *qemu-bridge-helper*:

	1. Install *libvirt*:
	```bash
	sudo apt-get install libvirt-bin
	```

	2. Run ```sudo virsh net-list --all``` and make sure that the *'default'* network is active (it should automatically start after *libvirt* installation). If it's inactive, make sure that the *'tun'* module is loaded (run ```sudo modprobe tun```) and try starting it manually with ```sudo virsh net-start default```. Successfully running *'default'* network provides us with *virbr0* bridged network interface listed in ```ifconfig```.

	3. The setuid attribute needs to be turned on for the default network helper (*qemu-network-helper* needs root permissions to create the tap device):
	```bash
	sudo chmod u+s /usr/lib/qemu/qemu-bridge-helper
	```
	
	4. Add ```allow virbr0``` line to the */etc/qemu/bridge.conf*:
	```bash
	echo "allow virbr0" | sudo tee -a /etc/qemu/bridge.conf > /dev/null
	```

	5. Run ia32-qemu.sh script.
	```bash
	./ia32-qemu.sh
	```
