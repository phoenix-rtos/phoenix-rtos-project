#
# Makefile for cognit_basic_demo
#
# Copyright 2025 Phoenix Systems
#
# Author: Mateusz Kobak
#
# %LICENSE%
#

LOCAL_PATH := $(call my-dir)

NAME := cognit_basic_demo
LIBS := libphoenix libcurl libmbedtls libmbedcrypto libmbedx509 libcognit
LOCAL_LDFLAGS := -Wl,-z,stack-size=200000
SRCS := $(wildcard $(LOCAL_PATH)*.c)

include $(binary.mk)
