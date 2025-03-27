#
# Makefile for user application
#
# Copyright 2024 Phoenix Systems
#
ifeq ($(HAVE_SHLIB), y)

NAME := libhello
LOCAL_HEADERS := hello.h
LOCAL_SRCS := hello.c

include $(shared-lib.mk)


NAME := dlopen
LOCAL_SRCS := main.c

include $(binary-dyn.mk)

endif
