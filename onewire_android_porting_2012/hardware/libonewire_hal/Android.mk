# Copyright (C) 2008 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

LOCAL_PATH := $(call my-dir)

# HAL module implemenation, not prelinked and stored in
# hw/<ONEWIRE_HARDWARE_MODULE_ID>.<ro.product.board>.so

include $(CLEAR_VARS)

#LOCAL_INCLUDES += $(LOCAL_PATH)

#LOCAL_C_INCLUDES += vendor/sh/onewire

#LOCAL_STATIC_LIBRARIES :=

LOCAL_SHARED_LIBRARIES := \
	liblog 	\
	libcutils \
	libonewire

LOCAL_SRC_FILES := \
	w1_hal_impl.cpp		\
	w1_hal.c

LOCAL_MODULE := libonewire_hal

LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)

LOCAL_PRELINK_MODULE := false


# The headers will be copied to :
# out/target/product/XXXX/obj/include
LOCAL_COPY_HEADERS_TO := libonewire_hal

LOCAL_COPY_HEADERS := w1_hal.h

include $(BUILD_SHARED_LIBRARY)
