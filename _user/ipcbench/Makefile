#
# Makefile for IPC benchmark
#
# Copyright 2026 Phoenix Systems
#
# Set IPCBENCH_NO_RR=1 to disable respondAndRecv benchmarks (for
# comparing against implementations without this syscall).
#

NAME := ipcbench
LOCAL_SRCS := main.c server.c bench.c

ifneq ($(IPCBENCH_NO_RR),)
LOCAL_CFLAGS := -DIPCBENCH_NO_RR
endif

ifneq ($(IPCBENCH_NO_EXT_BUF),)
LOCAL_CFLAGS := -DIPCBENCH_NO_EXT_BUF
endif

LOCAL_CFLAGS += -Wno-error

include $(binary.mk)
