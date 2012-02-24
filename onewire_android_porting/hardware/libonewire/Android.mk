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

# Linux library, not prelinked and stored in
# $(TARGET_OUT_SHARED_LIBRARIES)\libonewire.so

include $(CLEAR_VARS)

LOCAL_INCLUDES += $(LOCAL_PATH)

LOCAL_SHARED_LIBRARIES := \
	liblog 	\
	libcutils

LOCAL_SRC_FILES := \
	sh_log.c		\
	sh_thread.c		\
	sh_util.c		\
	w1_netlink_userspace.c	\
	w1_netlink_util.c		\
	w1_netlink_userservice.c

LOCAL_MODULE := libonewire

LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)

LOCAL_PRELINK_MODULE := false


# The headers will be copied to :
# out/target/product/XXXX/obj/include
LOCAL_COPY_HEADERS_TO := libonewire

LOCAL_COPY_HEADERS := sh_types.h \
    sh_error.h \
    sh_log.h \
    sh_thread.h \
    sh_util.h  \
    w1_netlink_userspace.h \
    w1_netlink_util.h \
    w1_netlink_userservice.h


include $(BUILD_SHARED_LIBRARY)
