# Copyright 2006 The Android Open Source Project



# HAL module implemenation, not prelinked and stored in
# hw/<ONEWIRE_HARDWARE_MODULE_ID>.<ro.product.board>.so
include $(CLEAR_VARS)
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)
LOCAL_SHARED_LIBRARIES := liblog libcutils
LOCAL_SRC_FILES := w1_hal.c w1_hal_impl.cpp
LOCAL_MODULE := libonewire
include $(BUILD_SHARED_LIBRARY)