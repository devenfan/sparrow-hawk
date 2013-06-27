LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    com_android_server_AlarmManagerService.cpp \
    com_android_server_BatteryService.cpp \
    com_android_server_HardwareService.cpp \
    com_android_server_KeyInputQueue.cpp \
    com_android_server_SensorService.cpp \
    com_android_server_SystemServer.cpp \
	com_android_server_OneWireService.cpp \
    onload.cpp

LOCAL_C_INCLUDES += \
	$(JNI_H_INCLUDE)

LOCAL_SHARED_LIBRARIES := \
    libandroid_runtime \
	libcutils \
	libhardware \
	libhardware_legacy \
	libnativehelper \
    libsystem_server \
	libutils \
	libui \
#	libonewire \
#	libonewire_hal

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

LOCAL_MODULE:= libandroid_servers

include $(BUILD_SHARED_LIBRARY)

