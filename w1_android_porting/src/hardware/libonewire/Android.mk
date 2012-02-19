# Copyright 2006 The Android Open Source Project



# HAL module implemenation, not prelinked and stored in
# hw/<ONEWIRE_HARDWARE_MODULE_ID>.<ro.product.board>.so

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE := false

LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)

LOCAL_INCLUDES += $(LOCAL_PATH)

LOCAL_SHARED_LIBRARIES := \
	liblog 	\
	libcutils

LOCAL_SRC_FILES += \
	sh_log.c		\
	sh_thread.c		\
	sh_util.c		\
	w1_netlink_userspace.c	\
	w1_netlink_util.c		\
	w1_netlink_userservice.c	\
	w1_hal_impl.cpp		\
#	w1_hal.c

LOCAL_MODULE := libonewire

include $(BUILD_SHARED_LIBRARY)
