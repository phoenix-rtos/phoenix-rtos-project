COMMON_LOCAL_SRCS := common.c

NAME := mmc
LOCAL_SRCS := client.c $(COMMON_LOCAL_SRCS)
LIBS := libtrace

include $(binary.mk)

NAME := mms
LOCAL_SRCS := server.c $(COMMON_LOCAL_SRCS)

include $(binary.mk)

NAME := mmfc
LOCAL_SRCS := fastpath_client.c $(COMMON_LOCAL_SRCS)
LIBS := libtrace

include $(binary.mk)

NAME := mmfs
LOCAL_SRCS := fastpath_server.c $(COMMON_LOCAL_SRCS)

include $(binary.mk)

NAME := just_memcpy
LOCAL_SRCS := just_memcpy.c $(COMMON_LOCAL_SRCS)

include $(binary.mk)

NAME := important
LOCAL_SRCS := important.c $(COMMON_LOCAL_SRCS)

include $(binary.mk)
