#
# Makefile for user application
#
# Copyright 2021 Phoenix Systems
#

NAME := rotrectangle
LOCAL_SRCS := main.c
LIBS := libgraph libvga libvirtio

ifneq ($(TARGET_FAMILY), host-pc)
	include $(binary.mk)
endif
