#!/usr/bin/env bash
#
# Shell script for running Phoenix-RTOS on SPIKE (riscv64-generic-spike)
#
# Copyright 2024 Phoenix Systems
# Author: Lukasz Leczkowski
#

exec spike -m0x80000000:0x10000000 --real-time-clint "$(dirname "${BASH_SOURCE[0]}")/../_boot/riscv64-generic-spike/sbi-generic.elf"
