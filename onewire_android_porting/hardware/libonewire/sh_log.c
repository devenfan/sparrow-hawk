
#include <stdio.h>

#include "sh_log.h"


void sh_debug(const char * TAG, const char * log)
{
#ifdef ANDROID_NDK
    __android_log_print(ANDROID_LOG_DEBUG, TAG, log);
#endif

    printf("[%s]: \t%s\n", TAG, log);
}

void sh_info(const char * TAG, const char * log)
{
#ifdef ANDROID_NDK
    __android_log_print(ANDROID_LOG_INFO, TAG, log);
#endif
    printf("[%s]: \t%s\n", TAG, log);
}

void sh_warn(const char * TAG, const char * log)
{
#ifdef ANDROID_NDK
    __android_log_print(ANDROID_LOG_WARN, TAG, log);
#endif
    printf("[%s]: \t%s\n", TAG, log);
}

void sh_error(const char * TAG, const char * log)
{
#ifdef ANDROID_NDK
    __android_log_print(ANDROID_LOG_ERROR, TAG, log);
#endif
    printf("[%s]: \t%s\n", TAG, log);
}
