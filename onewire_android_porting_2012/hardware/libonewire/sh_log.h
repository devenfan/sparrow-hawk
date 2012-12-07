#ifndef SH_LOG_H_INCLUDED
#define SH_LOG_H_INCLUDED


//#define ANDROID_NDK

#define LOG_NDEBUG 0

#ifndef LOG_TAG
#define LOG_TAG "unknown_tag"
#endif


#ifdef ANDROID_NDK

#include <android/log.h>    //android ndk log support

#define android_debug(format, args...)                                  \
{                                                                       \
    __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, format, ##args);    \
    printf("[%s]: \t", LOG_TAG);                                        \
    printf(format, ##args);                                             \
    printf("\n");														\
}

#define android_error(format, args...)                                  \
{                                                                       \
    __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, format, ##args);    \
    printf("[%s]: \t", LOG_TAG);                                        \
    printf(format, ##args);                                             \
    printf("\n");														\
}

#else  //#ifdef ANDROID_PLATFORM

#include "utils/Log.h"   //LOG_TAG Defined inside...

#define android_debug(format, args...)                                  \
{                                                                       \
    LOGD(format, ##args);                                               \
    printf("[%s]: \t", LOG_TAG);                                        \
    printf(format, ##args);                                             \
    printf("\n");														\
}

#define android_error(format, args...)                                  \
{                                                                       \
    LOGE(format, ##args);                                               \
    printf("[%s]: \t", LOG_TAG);                                        \
    printf(format, ##args);                                             \
    printf("\n");														\
}

#endif


#endif // SH_LOG_H_INCLUDED
