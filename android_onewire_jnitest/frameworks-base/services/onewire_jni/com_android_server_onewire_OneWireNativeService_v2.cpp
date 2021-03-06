/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "JNIHelp.h"
#include "jni.h"


#include "hardware/hardware.h"
#include "hardware_legacy/power.h"

#define  LOG_NDEBUG 0
#define  LOG_TAG 	"OneWireNativeServiceJNI"
#include "utils/Log.h"
#include "utils/misc.h"
#include "android_runtime/AndroidRuntime.h" //libandroid_runtime.so

#include <string.h>
#include <pthread.h>

#include "libonewire/sh_types.h"
#include "libonewire/w1_userspace.h"
#include "libonewire/w1_userservice.h"

#include "libonewire_hal/w1_hal.h"

#define ONEWIRE_LEGACY_MODE 1

static jobject mCallbacksObj = NULL;


static jmethodID method_masterAdded;
static jmethodID method_masterRemoved;
static jmethodID method_slaveAdded;
static jmethodID method_slaveRemoved;


static const w1hal_interface * sOneWireInterface = NULL;


#define WAKE_LOCK_NAME  "OneWireWakeLock"

#define MAX_SLAVE_COUNT  10
#define MAX_MASTER_COUNT  3

namespace android
{




static void convert_jlong_to_slave_id(const jlong * idIn, w1_slave_rn * idOut)
{
    memcpy(idOut, idIn, sizeof(w1_slave_rn));
}


static void convert_slave_id_to_jlong(const w1_slave_rn * idIn, jlong * idOut)
{
    memcpy(idOut, idIn, sizeof(jlong));
}

static void convert_jint_to_master_id(const jint * idIn, w1_master_id * idOut)
{
    memcpy(idOut, idIn, sizeof(w1_master_id));
}

static void convert_master_id_to_jint(const w1_master_id * idIn, jint * idOut)
{
    memcpy(idOut, idIn, sizeof(jint));
}





static void checkAndClearExceptionFromCallback(JNIEnv* env, const char* methodName)
{
    if (env->ExceptionCheck())
    {
        LOGE("An exception was thrown by callback '%s'.", methodName);
        //LOGE_EX(env);
        env->ExceptionClear();
    }
}



static void acquire_wakelock_callback()
{
    acquire_wake_lock(PARTIAL_WAKE_LOCK, WAKE_LOCK_NAME);
}

static void release_wakelock_callback()
{
    release_wake_lock(WAKE_LOCK_NAME);
}

static pthread_t create_thread_callback(const char* name, void (*start)(void *), void* arg)
{
    AndroidRuntime::createJavaThread(name, start, arg);
    return 1;
}





static void master_added_callback(w1_master_id masterId)
{
    JNIEnv* env = AndroidRuntime::getJNIEnv();

    if(NULL == env)
    {
        LOGE("Cannot getJNIEnv from AndroidRuntime!");
        return;
    }

    jint id;
    convert_master_id_to_jint(&masterId, &id);
    env->CallVoidMethod(mCallbacksObj, method_masterAdded, id);

    checkAndClearExceptionFromCallback(env, __FUNCTION__);
}

static void master_removed_callback(w1_master_id masterId)
{
    JNIEnv* env = AndroidRuntime::getJNIEnv();

    if(NULL == env)
    {
        LOGE("Cannot getJNIEnv from AndroidRuntime!");
        return;
    }

    jint id;
    convert_master_id_to_jint(&masterId, &id);
    env->CallVoidMethod(mCallbacksObj, method_masterRemoved, id);

    checkAndClearExceptionFromCallback(env, __FUNCTION__);
}

static void slave_added_callback(w1_slave_rn slaveRN)
{

    JNIEnv* env = AndroidRuntime::getJNIEnv();

    if(NULL == env)
    {
        LOGE("Cannot getJNIEnv from AndroidRuntime!");
        return;
    }

    jlong id;
    convert_slave_id_to_jlong(&slaveRN, &id);

    env->CallVoidMethod(mCallbacksObj, method_slaveAdded, id);

    checkAndClearExceptionFromCallback(env, __FUNCTION__);
}

static void slave_removed_callback(w1_slave_rn slaveRN)
{
    JNIEnv* env = AndroidRuntime::getJNIEnv();

    if(NULL == env)
    {
        LOGE("Cannot getJNIEnv from AndroidRuntime!");
        return;
    }

    jlong id;
    convert_slave_id_to_jlong(&slaveRN, &id);
    env->CallVoidMethod(mCallbacksObj, method_slaveRemoved, id);

    checkAndClearExceptionFromCallback(env, __FUNCTION__);
}



w1_user_callbacks sW1UserCallbacks =
{
    acquire_wakelock_callback,
    release_wakelock_callback,
    create_thread_callback,

    master_added_callback,
    master_removed_callback,
    slave_added_callback,
    slave_removed_callback,
};




static void android_onewire_OneWireNativeService_class_init_native(JNIEnv* env, jclass clazz)
{
    int err;
    hw_module_t* module;

    method_masterAdded      = env->GetMethodID(clazz, "masterAdded", "(I)V");
    method_masterRemoved    = env->GetMethodID(clazz, "masterRemoved", "(I)V");
    method_slaveAdded       = env->GetMethodID(clazz, "slaveAdded", "(J)V");
    method_slaveRemoved     = env->GetMethodID(clazz, "slaveRemoved", "(J)V");


#if ONEWIRE_LEGACY_MODE

    sOneWireInterface = hw_get_w1_interface();

#else

    err = hw_get_module(ONEWIRE_HARDWARE_MODULE_ID, (hw_module_t const**)&module);

    if (err == 0)
    {
        LOGI("w1 Stub found.");

        hw_device_t* device;
        err = module->methods->open(module, ONEWIRE_HARDWARE_MODULE_ID, &device);
        if (err == 0)
        {
            struct w1hal_device_t * onewire_device = (struct w1hal_device_t *)device;
            sOneWireInterface = onewire_device->get_w1_interface(onewire_device);

            LOGI("Got w1 Stub operations.");
        }
        else
        {
            LOGE("Cannot open w1 device!!! Error[%d]", err);
        }
    }
    else
    {
        LOGE("w1 Stub not found!!! Error[%d]", err);
    }

#endif

    if(NULL == sOneWireInterface)
    {
        LOGE("Error! w1 Stub operations not found!");
    }
    else
    {
        LOGI("Good! w1 Stub operations found!");
    }

}


/**
 * Package Name: 	see - register_android_server_onewire_OneWireNativeService()
 * Class Name: 		OneWireNativeService
 * Method Name:  	is_supported
*/
static jboolean android_onewire_OneWireNativeService_is_supported(JNIEnv* env, jclass clazz)
{
    if(!sOneWireInterface)
    {
        LOGE("w1 Stub operations not exist!");
    }

    return (sOneWireInterface != NULL);
}

/**
 * Package Name: 	see - register_android_server_onewire_OneWireNativeService()
 * Class Name: 		OneWireNativeService
 * Method Name:  	start
*/
static jboolean android_onewire_OneWireNativeService_start(JNIEnv* env, jobject obj)
{
    // this must be set before calling into the HAL library
    if (!mCallbacksObj)
    {
        mCallbacksObj = env->NewGlobalRef(obj);
    }

    LOGD("mCallbacksObj: 0x%p", mCallbacksObj);

    if(!sOneWireInterface)
    {
        LOGE("w1 Stub operations not exist!");
        return JNI_FALSE;
    }

    sOneWireInterface->init(&sW1UserCallbacks);

    return (sOneWireInterface->start());
}

/**
 * Package Name: 	see - register_android_server_onewire_OneWireNativeService()
 * Class Name: 		OneWireNativeService
 * Method Name:  	stop
*/
static void android_onewire_OneWireNativeService_stop(JNIEnv* env, jobject obj)
{
    if(!sOneWireInterface)
    {
        LOGE("w1 Stub operations not exist!");
        return;
    }

    sOneWireInterface->stop();
}

/**
 * Package Name: 	see - register_android_server_onewire_OneWireNativeService()
 * Class Name: 		OneWireNativeService
 * Method Name:  	get_current_master
*/
static jint android_onewire_OneWireNativeService_get_current_master(JNIEnv* env, jobject obj)
{
    if(!sOneWireInterface)
    {
        LOGE("w1 Stub operations not exist!");
        return 0;
    }

    jint idOut = 0;
    w1_master_id master = sOneWireInterface->get_current_master();
    convert_master_id_to_jint(&master, &idOut);

    return idOut;
}

/**
 * Package Name: 	see - register_android_server_onewire_OneWireNativeService()
 * Class Name: 		OneWireNativeService
 * Method Name:  	get_current_slaves
*/
static jint android_onewire_OneWireNativeService_get_current_slaves(JNIEnv* env, jobject obj, jlongArray slaveRNs)
{
    jint slaveCount = 0;
    w1_slave_rn slaves[MAX_SLAVE_COUNT];

    if(!sOneWireInterface)
    {
        LOGE("w1 Stub operations not exist!");
        return 0;
    }

    sOneWireInterface->get_current_slaves(slaves, &slaveCount);
    if(slaveCount > 0)
    {
        if(slaveCount > MAX_SLAVE_COUNT)
            slaveCount = MAX_SLAVE_COUNT;

        jlong* l = env->GetLongArrayElements(slaveRNs, NULL);

        memcpy(l, slaves, sizeof(w1_slave_rn) * slaveCount);

        env->ReleaseLongArrayElements(slaveRNs, l, 0);
    }

    return slaveCount;
}

/**
 * Package Name: 	see - register_android_server_onewire_OneWireNativeService()
 * Class Name: 		OneWireNativeService
 * Method Name:  	begin_exclusive
*/
static jboolean android_onewire_OneWireNativeService_begin_exclusive(JNIEnv* env, jobject obj, jint masterId)
{
    if(!sOneWireInterface)
    {
        LOGE("w1 Stub operations not exist!");
        return JNI_FALSE;
    }

    w1_master_id idOut;
    convert_jint_to_master_id(&masterId, &idOut);

    return sOneWireInterface->begin_exclusive(idOut);
}

/**
 * Package Name: 	see - register_android_server_onewire_OneWireNativeService()
 * Class Name: 		OneWireNativeService
 * Method Name:  	end_exclusive
*/
static void android_onewire_OneWireNativeService_end_exclusive(JNIEnv* env, jobject obj, jint masterId)
{
    if(!sOneWireInterface)
    {
        LOGE("w1 Stub operations not exist!");
    }

    w1_master_id idOut;
    convert_jint_to_master_id(&masterId, &idOut);

    sOneWireInterface->end_exclusive(idOut);
}

/**
 * Package Name: 	see - register_android_server_onewire_OneWireNativeService()
 * Class Name: 		OneWireNativeService
 * Method Name:  	list_masters
*/
static jint android_onewire_OneWireNativeService_list_masters(JNIEnv* env, jobject obj, jintArray masterIDs)
{
    jint masterCount = 0;
    w1_master_id masters[MAX_MASTER_COUNT];
    /*
    if(sOneWireInterface)
    {
    	if(sOneWireInterface->list_masters(masters, &masterCount))
    	{
    		if(masterCount > 0)
    		{
    			if(masterCount > MAX_MASTER_COUNT)
    				masterCount = MAX_MASTER_COUNT;

    			jint* i = env->GetIntArrayElements(masterIDs, NULL);

    			memcpy(i, masters, sizeof(w1_master_id) * masterCount);

    			env->ReleaseIntArrayElements(masterIDs, i, 0);
    		}
    	}
    }
    */
    return masterCount;
}

/**
 * Package Name: 	see - register_android_server_onewire_OneWireNativeService()
 * Class Name: 		OneWireNativeService
 * Method Name:  	search_slaves
*/
static jint android_onewire_OneWireNativeService_search_slaves(JNIEnv* env, jobject obj,
        jint masterId, jlongArray slaveRNs)
{
    jint slaveCount = 0;
    w1_slave_rn slaves[MAX_SLAVE_COUNT];

    w1_master_id idOut;
    convert_jint_to_master_id(&masterId, &idOut);

    if(sOneWireInterface)
    {
        if(sOneWireInterface->search_slaves(idOut, slaves, &slaveCount))
        {
            if(slaveCount > 0)
            {
                if(slaveCount > MAX_SLAVE_COUNT)
                    slaveCount = MAX_SLAVE_COUNT;

                jlong* l = env->GetLongArrayElements(slaveRNs, NULL);

                memcpy(l, slaves, sizeof(w1_slave_rn) * slaveCount);

                env->ReleaseLongArrayElements(slaveRNs, l, 0);
            }
        }
    }

    return slaveCount;
}

/**
 * Package Name: 	see - register_android_server_onewire_OneWireNativeService()
 * Class Name: 		OneWireNativeService
 * Method Name:  	master_reset
*/
static jboolean android_onewire_OneWireNativeService_master_reset(JNIEnv* env, jobject obj, jint masterId)
{
    jboolean result = JNI_FALSE;

    w1_master_id idOut;
    convert_jint_to_master_id(&masterId, &idOut);

    if(sOneWireInterface)
    {
        result = sOneWireInterface->master_reset(idOut);
    }
    return result;
}

/**
 * Package Name: 	see - register_android_server_onewire_OneWireNativeService()
 * Class Name: 		OneWireNativeService
 * Method Name:  	master_touch
*/
static jboolean android_onewire_OneWireNativeService_master_touch(JNIEnv* env, jobject obj,
        jint masterId, jbyteArray dataIn, jint dataInLen, jbyteArray dataOut)
{
    jboolean result = JNI_FALSE;
    jint dataOutLen = 0;

    w1_master_id idOut;
    convert_jint_to_master_id(&masterId, &idOut);

    if(sOneWireInterface)
    {
        jbyte* b1 = env->GetByteArrayElements(dataIn, NULL);
        jbyte* b2 = env->GetByteArrayElements(dataOut, NULL);

        result = sOneWireInterface->master_touch(idOut,
                 (BYTE *)b1, dataInLen, (BYTE *)b2, &dataOutLen);

        env->ReleaseByteArrayElements(dataIn, b1, 0);
        env->ReleaseByteArrayElements(dataOut, b2, 0);

        if(result && dataInLen == dataOutLen)
            result = JNI_TRUE; //how many in, how many out...
    }
    return result;
}

/**
 * Package Name: 	see - register_android_server_onewire_OneWireNativeService()
 * Class Name: 		OneWireNativeService
 * Method Name:  	master_read
*/
static jboolean android_onewire_OneWireNativeService_master_read(JNIEnv* env, jobject obj,
        jint masterId, jint readLen, jbyteArray dataReadOut)
{
    jboolean result = JNI_FALSE;

    w1_master_id idOut;
    convert_jint_to_master_id(&masterId, &idOut);

    if(sOneWireInterface)
    {
        jbyte* b = env->GetByteArrayElements(dataReadOut, NULL);

        result = sOneWireInterface->master_read(idOut, readLen, (BYTE *)b);

        env->ReleaseByteArrayElements(dataReadOut, b, 0);
    }
    return result;
}

/**
 * Package Name: 	see - register_android_server_onewire_OneWireNativeService()
 * Class Name: 		OneWireNativeService
 * Method Name:  	master_write
*/
static jboolean android_onewire_OneWireNativeService_master_write(JNIEnv* env, jobject obj,
        jint masterId, jint writeLen, jbyteArray dataWriteIn)
{
    jboolean result = JNI_FALSE;

    w1_master_id idOut;
    convert_jint_to_master_id(&masterId, &idOut);

    if(sOneWireInterface)
    {
        jbyte* b = env->GetByteArrayElements(dataWriteIn, NULL);

        result = sOneWireInterface->master_write(idOut, writeLen, (BYTE *)b);

        env->ReleaseByteArrayElements(dataWriteIn, b, 0);
    }
    return result;
}





static JNINativeMethod sMethods[] =
{
    /* name, 						signature, 				funcPtr */
    {"class_init_native", 			"()V", 					(void*)android_onewire_OneWireNativeService_class_init_native},
    {"native_is_supported", 		"()Z", 					(void*)android_onewire_OneWireNativeService_is_supported},
    {"native_start", 				"()Z", 					(void*)android_onewire_OneWireNativeService_start},
    {"native_stop", 				"()V", 					(void*)android_onewire_OneWireNativeService_stop},

    {"native_get_current_master", 	"()I", 					(void*)android_onewire_OneWireNativeService_get_current_master},
    {"native_get_current_slaves", 	"([J)I", 				(void*)android_onewire_OneWireNativeService_get_current_slaves},
    {"native_begin_exclusive", 		"(I)Z", 					(void*)android_onewire_OneWireNativeService_begin_exclusive},
    {"native_end_exclusive", 		"(I)V", 					(void*)android_onewire_OneWireNativeService_end_exclusive},

    //{"native_list_masters", 		"([I)I", 				(void*)android_onewire_OneWireNativeService_list_masters},
    {"native_search_slaves", 		"(I[J)I", 				(void*)android_onewire_OneWireNativeService_search_slaves},
    {"native_master_reset", 		"(I)Z", 				(void*)android_onewire_OneWireNativeService_master_reset},
    {"native_master_touch", 		"(I[BI[B)Z", 			(void*)android_onewire_OneWireNativeService_master_touch},
    {"native_master_read", 			"(II[B)Z", 				(void*)android_onewire_OneWireNativeService_master_read},
    {"native_master_write", 		"(II[B)Z", 				(void*)android_onewire_OneWireNativeService_master_write},

};




int register_android_server_onewire_OneWireNativeService(JNIEnv* env)
{
    //return jniRegisterNativeMethods(env, "com/android/server/onewire/OneWireNativeService", sMethods, NELEM(sMethods));
    return jniRegisterNativeMethods(env, "net/sh/android/onewire/legacy/OneWireNativeService", sMethods, NELEM(sMethods));
}


} /* namespace android */
