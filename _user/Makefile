#
# Makefile for user app
#
# Copyright 2021 Phoenix Systems
#
# %LICENSE%
#

include ../phoenix-rtos-build/Makefile.common

.DEFAULT_GOAL := all

# default path for the programs to be installed in rootfs
DEFAULT_INSTALL_PATH := /usr/bin

# read out all components
ALL_MAKES := $(shell find . -mindepth 2 -name Makefile)
include $(ALL_MAKES)

# create generic targets
DEFAULT_COMPONENTS := $(ALL_COMPONENTS)

.PHONY: all install clean
all: $(DEFAULT_COMPONENTS)
install: $(patsubst %,%-install,$(DEFAULT_COMPONENTS))
clean: $(patsubst %,%-clean,$(ALL_COMPONENTS))
