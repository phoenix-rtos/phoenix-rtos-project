#!/usr/bin/env bash
#
# Shell script functions for flashing images.
#
# Copyright 2021, 2023 Phoenix Systems
# Author: Gerard Swiderski, Hubert Badocha
#

[ "${BASH_SOURCE[0]}" -ef "$0" ] && echo "You should source this script, not execute it!" && exit 1

trap 'exit' INT TERM ERR
trap 'kill 0' EXIT

#
# Silenced pushd and popd
#
pushd () {
	command pushd "$@" > /dev/null
}

popd () {
	command popd > /dev/null
}

#
# Print error message to stderr and exit
#
abort () {
	echo "$*" >&2
	exit 1
}

#
# Detect plo USB-CDC device
#
get_comm_linux () {
	local usb_dev="/dev/serial/by-id/usb-Phoenix_Systems_plo_CDC_ACM-if00"
	if [ -n "$USB_DEV" ]; then
		usb_dev="$USB_DEV"
	fi

	(( timeout = 3 ))
	while (( timeout > 0 )) && [ ! -c "${usb_dev}" ] ; do
		sleep 1
		(( timeout -= 1 ))
	done

	if [ -c "${usb_dev}" ] ; then
		echo "${usb_dev}"
	else
		# fallback in case of no default udev symlink
		dmesg | tail -1 | sed -ne "s_.*\(ttyACM[0-9]\+\):.*device\$_/dev/\1_p"
	fi
}

get_comm () {
	case "$(uname -s)" in
		Linux)
			get_comm_linux;
			;;
		*)
			abort "Unsupported system"
			;;
	esac
}

#
# Run phoenixd in background
#
run_phoenixd () {
	local -r work_path="$1"
	local -r com_dev="$(get_comm)"

	if [ -z "${com_dev}" ]; then
		abort "Unable to get ttyACM port, check if plo USB-HID device is connected to the host"
	fi

	./phoenixd -p "${com_dev}" -s "${work_path}" > /dev/null 2>&1 &

	if ! ps -p "$!" > /dev/null 2>&1 ; then
		abort "Unable to start phoenixd"
	fi

	echo "phoenixd is running in background"
}

#
# Send commands to plo tty
#
plo_command () {
	if [ -z "${console_tty}" ] ; then
		abort "console_tty must be defined"
	fi

	stty -F "${console_tty}" 115200 -brkint -icrnl -imaxbel -opost -isig -icanon -iexten -echo

	printf "%s\r" "$1" > "${console_tty}"
	printf "%s\n" "$1"
	sleep 1
}

#
# Print usage, check ($@) arguments and initialize environment
#
check_arguments () {	
	: "${TARGET:?variable unset}"

	if [ $# -ne 1 ]; then
		echo "Usage: $0 <port>"
		echo "where: <port> is plo tty device, e.g. /dev/ttyUSB0"
		exit 1
	fi

	console_tty="$1"

	if [ ! -c "${console_tty}" ] ; then
		abort "Please provide plo console port e.g. /dev/ttyUSB0"
	fi

	boot_path=$(realpath "$(pwd)/../_boot")

	if ! [ -d "${boot_path}" ] ; then
		abort "No _boot path"
	fi

	if ! [ -f "${boot_path}/${TARGET}/phoenix.disk" ] ; then
		abort "Please build ${TARGET} before programming device"
	fi

	prog_path=$(realpath "$(pwd)/../_build/${TARGET}/prog.stripped")

	# Variables to be used by sourcing script.
	export console_tty
	export boot_path
	export prog_path
}
