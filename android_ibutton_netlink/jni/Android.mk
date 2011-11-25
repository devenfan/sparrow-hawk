LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

# ========================================================
# librxtx
# ========================================================

LOCAL_MODULE_TAGS := eng
LOCAL_MODULE:= ibutton_netlink_lib


LOCAL_SRC_FILES := \
	fuserImp.c \
	SerialImp.c

LOCAL_C_INCLUDES += \
	dalvik/libnativehelper/include/nativehelper \
	$(JNI_H_INCLUDE) \
	$(KERNEL_HEADERS) \
	$(LOCAL_PATH)

LOCAL_CFLAGS += \
	 -fPIC

LOCAL_PRELINK_MODULE := false
LOCAL_SHARED_LIBRARIES := libdl  liblog  libc

#### Add below line to resolve: undefined reference to `__android_log_print'
LOCAL_LDLIBS := -lm -llog

include $(BUILD_SHARED_LIBRARY)

