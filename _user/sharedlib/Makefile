#
# Makefile for user application
#
# Copyright 2024 Phoenix Systems
#
ifeq ($(HAVE_SHLIB), y)

NAME := libdyn
LOCAL_HEADERS := dyn.h
LOCAL_SRCS := dyn.c

include $(shared-lib.mk)


NAME := sharedlib
LOCAL_SRCS := main.c
DEP_LIBS_SHARED := libdyn

include $(binary-dyn.mk)

endif
