#
# Makefile for cognit_app
#
# Copyright 2025 Phoenix Systems
#
# Author: Mateusz Kobak
#
# %LICENSE%
#

LOCAL_PATH := $(call my-dir)

NAME := cognit_app
LIBS := libphoenix libmodbus libcurl libmbedtls libmbedcrypto libmbedx509 libcognit libjansson
# LOCAL_CFLAGS += -DDEBUG_NO_OFFLOAD
LOCAL_LDFLAGS := -Wl,-z,stack-size=32768
SRCS := $(wildcard $(LOCAL_PATH)*.c)

include $(binary.mk)
