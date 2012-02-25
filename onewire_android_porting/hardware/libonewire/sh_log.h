#ifndef SH_LOG_H_INCLUDED
#define SH_LOG_H_INCLUDED




#ifdef ANDROID_NDK

#include <android/log.h>    //android ndk log support

#define android_debug(logTag, format, args...)                         \
{                                                                      \
    __android_log_print(ANDROID_LOG_DEBUG, logTag, format, ##args);    \
    printf("[%s]: \t", logTag);                                        \
    printf(format, ##args);                                            \
}

#endif



#ifdef ANDROID_PLATFORM

//#define  LOG_NDEBUG 0
//#define  LOG_TAG 	"OneWireNativeServiceJNI"
#include "utils/Log.h"   //LOG_TAG Defined inside...
#include "utils/misc.h"

#define android_debug(logTag, format, args...)                          \
{                                                                       \
    LOGD(format, ##args);                                               \
    printf("[%s]: \t", logTag);                                        \
    printf(format, ##args);                                             \
}

#endif


#ifdef __cplusplus
extern "C" {
#endif


void sh_debug(const char * TAG, const char * log);

void sh_info(const char * TAG, const char * log);

void sh_warn(const char * TAG, const char * log);

void sh_error(const char * TAG, const char * log);


#ifdef __cplusplus
}
#endif



#endif // SH_LOG_H_INCLUDED
