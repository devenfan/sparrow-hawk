LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

# [optional, user, eng]
# eng = required
# optinal = no install on target
LOCAL_MODULE_TAGS := eng

# All of the source files that we will compile.
LOCAL_SRC_FILES:= \
	com_android_server_onewire_OneWireNativeService.cpp \
    onload.cpp

# Also need the JNI headers.
LOCAL_C_INCLUDES += \
	$(JNI_H_INCLUDE)

# All of the shared libraries we link against.
LOCAL_SHARED_LIBRARIES := \
    libandroid_runtime \
	libcutils \
	libhardware \
	libhardware_legacy \
	libnativehelper \
    libsystem_server \
	libutils \
	libui \
	libonewire \
	libonewire_hal

ifeq ($(TARGET_SIMULATOR),true)
ifeq ($(TARGET_OS),linux)
ifeq ($(TARGET_ARCH),x86)
LOCAL_LDLIBS += -lpthread -ldl -lrt
endif
endif
endif

ifeq ($(WITH_MALLOC_LEAK_CHECK),true)
	LOCAL_CFLAGS += -DMALLOC_LEAK_CHECK
endif

# No specia compiler flags.
LOCAL_CFLAGS +=

LOCAL_MODULE:= libandroid_onewire

# Target install path.
LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)

# Don't prelink this library. Or you will get this error:
# "library 'libandroid_onewire.so' not in prelink map" .
# For more efficient code, you may want to add this library to
# the prelink map and set this to true.
LOCAL_PRELINK_MODULE := false

include $(BUILD_SHARED_LIBRARY)

