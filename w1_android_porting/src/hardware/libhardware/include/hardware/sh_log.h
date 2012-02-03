#ifndef SH_LOG_H_INCLUDED
#define SH_LOG_H_INCLUDED

#ifdef ANDROID_NDK

#include <android/log.h>    //android ndk log support

#define android_debug(LOG_TAG, format, args...)                         \
{                                                                       \
    __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, format, ##args);    \
    printf("[%s]: \t", LOG_TAG);                                        \
    printf(format, ##args);                                             \
}

#endif

#ifdef ANDROID_PLATFORM

#endif

void sh_debug(const char * TAG, const char * log);

void sh_info(const char * TAG, const char * log);

void sh_warn(const char * TAG, const char * log);

void sh_error(const char * TAG, const char * log);


#endif // SH_LOG_H_INCLUDED
