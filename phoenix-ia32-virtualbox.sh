#!/bin/bash

set -e

VM="phoenix-ia32-generic"

create_vm() {
	echo -e "\033[1;33m--- Creating ${VM} Virtualbox VM ---\033[0m"

	pushd _boot/
	# Convert raw ext2 image to vdi
#	VBoxManage convertfromraw "$VM.disk" "$VM.vdi"

	# Create and register VM
#	VBoxManage createvm --name $VM --ostype "Other" --register

	# Add IDE Controller with created vdi disk attached
#	VBoxManage storagectl $VM --name "IDE Controller" --add ide --controller ICH6
	VBoxManage storageattach $VM --storagectl "IDE Controller" --port 0 --device 0 --type hdd --medium "$VM.vdi"

	# Modify misc system settings
#	VBoxManage modifyvm $VM --ioapic on
#	VBoxManage modifyvm $VM --boot1 disk --boot2 none --boot3 none --boot4 none
#	VBoxManage modifyvm $VM --memory 64 --vram 32
	popd
}


create_vm
exit 0


if [ "$(VBoxManage list vms | grep -c -e $VM)" == "1" ]; then
	if [ "$(VBoxManage showvminfo $VM | grep -c "running (since")" == "1" ]; then
		if [ "$1" == "-f" ]; then
			echo "${VM} Virtualbox VM is running. Can't force rebuild the VM"
		else
			echo "${VM} Virtualbox VM is already running"
		fi
		exit 1
	else
		if [ "$1" == "-f" ]; then
			# Unregister and delete the VM
			VBoxManage unregistervm --delete $VM

			# Force rebuild the VM
			create_vm
		fi
	fi
else
	# Create the VM
	create_vm
fi

VBoxManage startvm $VM

exit 0
