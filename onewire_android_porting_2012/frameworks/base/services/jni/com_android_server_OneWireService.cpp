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
#define  LOG_TAG 	"OneWireServiceJNI"
#include "utils/Log.h"
#include "utils/misc.h"
#include "android_runtime/AndroidRuntime.h" //libandroid_runtime.so

#include <string.h>
#include <pthread.h>

#include "libonewire/sh_types.h"
#include "libonewire/w1_userspace.h"
#include "libonewire/w1_userservice.h"

#include "libonewire_hal/w1_hal.h"

//------------------------------------------------------------------

//#define ONEWIRE_LEGACY_MODE 1

#define WAKE_LOCK_NAME  "OneWireWakeLock"

#define MAX_SLAVE_COUNT  100
#define MAX_MASTER_COUNT  10

//------------------------------------------------------------------

static jobject mCallbacksObj = NULL;


static jmethodID method_masterAdded;
static jmethodID method_masterRemoved;
static jmethodID method_slaveAdded;
static jmethodID method_slaveRemoved;


static const onewire_interface * sOneWireInterface = NULL;


//------------------------------------------------------------------

namespace android
{


// Utilities ------------------------------------------------------------------

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
        LOGE("w1 exception was thrown by callback '%s'.", methodName);
        //LOGE_EX(env);
        env->ExceptionClear();
    }
}

// callbacks ------------------------------------------------------------------

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

	LOGI("OneWire(1-Wire or w1) callback: master_added_callback");
	
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

	LOGI("OneWire(1-Wire or w1) callback: master_removed_callback");
	
    env->CallVoidMethod(mCallbacksObj, method_masterRemoved, id);

    checkAndClearExceptionFromCallback(env, __FUNCTION__);
}

static void slave_added_callback(w1_master_id masterId, w1_slave_rn slaveRN)
{

    JNIEnv* env = AndroidRuntime::getJNIEnv();

    if(NULL == env)
    {
        LOGE("Cannot getJNIEnv from AndroidRuntime!");
        return;
    }

    jint mid;
    jlong rid;
    convert_master_id_to_jint(&masterId, &mid);
    convert_slave_id_to_jlong(&slaveRN, &rid);

	LOGI("OneWire(1-Wire or w1) callback: slave_added_callback");

    env->CallVoidMethod(mCallbacksObj, method_slaveAdded, mid, rid);

    checkAndClearExceptionFromCallback(env, __FUNCTION__);
}

static void slave_removed_callback(w1_master_id masterId, w1_slave_rn slaveRN)
{
    JNIEnv* env = AndroidRuntime::getJNIEnv();

    if(NULL == env)
    {
        LOGE("Cannot getJNIEnv from AndroidRuntime!");
        return;
    }

    jint mid;
    jlong rid;
    convert_master_id_to_jint(&masterId, &mid);
    convert_slave_id_to_jlong(&slaveRN, &rid);

	LOGI("OneWire(1-Wire or w1) callback: slave_removed_callback");

    env->CallVoidMethod(mCallbacksObj, method_slaveRemoved, mid, rid);

    checkAndClearExceptionFromCallback(env, __FUNCTION__);
}

//--------------------------------------------------------------------------------

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


//--------------------------------------------------------------------------------

static void android_onewire_OneWireService_class_init_native(JNIEnv* env, jclass clazz)
{
    int err;
    hw_module_t* module;

    method_masterAdded      = env->GetMethodID(clazz, "masterAdded", "(I)V");
    method_masterRemoved    = env->GetMethodID(clazz, "masterRemoved", "(I)V");
    method_slaveAdded       = env->GetMethodID(clazz, "slaveAdded", "(IJ)V");
    method_slaveRemoved     = env->GetMethodID(clazz, "slaveRemoved", "(IJ)V");


#if ONEWIRE_LEGACY_MODE

    sOneWireInterface = hw_get_onewire_interface();

#else

    err = hw_get_module(ONEWIRE_HARDWARE_MODULE_ID, (hw_module_t const**)&module);

    if (err == 0)
    {
        LOGI("OneWire(1-Wire or w1) HAL module found!!!");

        hw_device_t* device;
        err = module->methods->open(module, ONEWIRE_HARDWARE_MODULE_ID, &device);
        if (err == 0)
        {
            struct onewire_device_t * onewireDevice = (struct onewire_device_t *)device;
            sOneWireInterface = onewireDevice->get_onewire_interface(onewireDevice);
        }
        else
        {
            LOGE("Cannot open OneWire(1-Wire or w1) HAL device!!! Error[%d]", err);
        }
    }
    else
    {
        LOGE("OneWire(1-Wire or w1) HAL module not found!!! Error[%d]", err);
    }

#endif

    if(NULL == sOneWireInterface)
    {
        LOGE("Error! OneWire(1-Wire or w1) Stub operations not found!");
    }
    else
    {
        LOGI("Good! OneWire(1-Wire or w1) Stub operations found!");
    }

}


/**
 * Package Name: 	see - register_android_server_onewire_OneWireService()
 * Class Name: 		OneWireService
 * Method Name:  	is_supported
*/
static jboolean android_onewire_OneWireService_is_supported(JNIEnv* env, jclass clazz)
{
    if(!sOneWireInterface)
    {
        LOGE("OneWire(1-Wire or w1) Stub operations not exist!");
    }

    return (sOneWireInterface != NULL);
}

/**
 * Package Name: 	see - register_android_server_onewire_OneWireService()
 * Class Name: 		OneWireService
 * Method Name:  	start
*/
static jboolean android_onewire_OneWireService_start(JNIEnv* env, jobject obj)
{
    // this must be set before calling into the HAL library
    if (!mCallbacksObj)
    {
        mCallbacksObj = env->NewGlobalRef(obj);
    }

    LOGD("mCallbacksObj: 0x%p", mCallbacksObj);

    if(!sOneWireInterface)
    {
        LOGE("OneWire(1-Wire or w1) Stub operations not exist!");
        return JNI_FALSE;
    }

    sOneWireInterface->init(&sW1UserCallbacks);

    return (sOneWireInterface->start());
}

/**
 * Package Name: 	see - register_android_server_onewire_OneWireService()
 * Class Name: 		OneWireService
 * Method Name:  	stop
*/
static void android_onewire_OneWireService_stop(JNIEnv* env, jobject obj)
{
    if(!sOneWireInterface)
    {
        LOGE("OneWire(1-Wire or w1) Stub operations not exist!");
        return;
    }

    sOneWireInterface->stop();
}

/**
 * Package Name: 	see - register_android_server_onewire_OneWireService()
 * Class Name: 		OneWireService
 * Method Name:  	is_debug_enabled
*/

static jboolean android_onewire_OneWireService_is_debug_enabled(JNIEnv* env, jobject obj)
{
    if(!sOneWireInterface)
    {
        LOGE("OneWire(1-Wire or w1) Stub operations not exist!");
        return JNI_FALSE;
    }

    return sOneWireInterface->is_debug_enabled();
}


/**
 * Package Name: 	see - register_android_server_onewire_OneWireService()
 * Class Name: 		OneWireService
 * Method Name:  	set_debug_enabled
*/

static void android_onewire_OneWireService_set_debug_enabled(JNIEnv* env, jobject obj, jboolean enabled)
{
    if(!sOneWireInterface)
    {
        LOGE("OneWire(1-Wire or w1) Stub operations not exist!");
        return;
    }

    sOneWireInterface->set_debug_enabled(enabled);
}


/**
 * Package Name: 	see - register_android_server_onewire_OneWireService()
 * Class Name: 		OneWireService
 * Method Name:  	begin_exclusive
*/
static jboolean android_onewire_OneWireService_begin_exclusive(JNIEnv* env, jobject obj)
{
    if(!sOneWireInterface)
    {
        LOGE("OneWire(1-Wire or w1) Stub operations not exist!");
        return JNI_FALSE;
    }

    //w1_master_id idOut;
    //convert_jint_to_master_id(&masterId, &idOut);

    return sOneWireInterface->begin_exclusive();
}

/**
 * Package Name: 	see - register_android_server_onewire_OneWireService()
 * Class Name: 		OneWireService
 * Method Name:  	end_exclusive
*/
static void android_onewire_OneWireService_end_exclusive(JNIEnv* env, jobject obj)
{
    if(!sOneWireInterface)
    {
        LOGE("OneWire(1-Wire or w1) Stub operations not exist!");
    }

    //w1_master_id idOut;
    //convert_jint_to_master_id(&masterId, &idOut);

    sOneWireInterface->end_exclusive();
}




/**
 * Package Name: 	see - register_android_server_onewire_OneWireService()
 * Class Name: 		OneWireService
 * Method Name:  	get_current_masters
*/
static jint android_onewire_OneWireService_get_current_masters(JNIEnv* env, jobject obj, jintArray masterIDs)
{
    jint masterCount = 0;
    w1_master_id masters[MAX_MASTER_COUNT];

    if(!sOneWireInterface)
    {
        LOGE("OneWire(1-Wire or w1) Stub operations not exist!");
    }
    else
    {
    	sOneWireInterface->get_current_masters(masters, &masterCount);
		
    	if(masterCount > 0)
		{
			if(masterCount > MAX_MASTER_COUNT)
				masterCount = MAX_MASTER_COUNT;

			jint* p = env->GetIntArrayElements(masterIDs, NULL);

			memcpy(p, masters, sizeof(w1_master_id) * masterCount);

			env->ReleaseIntArrayElements(masterIDs, p, 0);
		}
    	
    }

    return masterCount;
}



/**
 * Package Name: 	see - register_android_server_onewire_OneWireService()
 * Class Name: 		OneWireService
 * Method Name:  	list_masters
*/
static jint android_onewire_OneWireService_list_masters(JNIEnv* env, jobject obj, jintArray masterIDs)
{
    jint masterCount = 0;
    w1_master_id masters[MAX_MASTER_COUNT];

    if(!sOneWireInterface)
    {
        LOGE("OneWire(1-Wire or w1) Stub operations not exist!");
    }
    else
    {
    	if(sOneWireInterface->list_masters(masters, &masterCount))
    	{
    		if(masterCount > 0)
    		{
    			if(masterCount > MAX_MASTER_COUNT)
    				masterCount = MAX_MASTER_COUNT;

    			jint* p = env->GetIntArrayElements(masterIDs, NULL);

    			memcpy(p, masters, sizeof(w1_master_id) * masterCount);

    			env->ReleaseIntArrayElements(masterIDs, p, 0);
    		}
    	}
    	else
    	{
            LOGE("OneWire(1-Wire or w1) Stub error: list_masters failed!");
    	}
    }

    return masterCount;
}





/**
 * Package Name: 	see - register_android_server_onewire_OneWireService()
 * Class Name: 		OneWireService
 * Method Name:  	search_slaves
*/
static jint android_onewire_OneWireService_search_slaves(JNIEnv* env, jobject obj,
        jint masterId, jlongArray slaveRNs)
{
    jint slaveCount = 0;
    w1_slave_rn slaves[MAX_SLAVE_COUNT];

    w1_master_id mid;
    convert_jint_to_master_id(&masterId, &mid);

    if(!sOneWireInterface)
    {
        LOGE("OneWire(1-Wire or w1) Stub operations not exist!");
    }
    else
    {
        if(sOneWireInterface->search_slaves(mid, slaves, &slaveCount))
        {
            if(slaveCount > 0)
            {
                if(slaveCount > MAX_SLAVE_COUNT)
                    slaveCount = MAX_SLAVE_COUNT;

                jlong* p = env->GetLongArrayElements(slaveRNs, NULL);

                memcpy(p, slaves, sizeof(w1_slave_rn) * slaveCount);

                env->ReleaseLongArrayElements(slaveRNs, p, 0);
            }
        }
        else
    	{
            LOGE("OneWire(1-Wire or w1) Stub error: search_slaves failed!");
    	}
    }

    return slaveCount;
}

/**
 * Package Name: 	see - register_android_server_onewire_OneWireService()
 * Class Name: 		OneWireService
 * Method Name:  	master_reset
*/
static jboolean android_onewire_OneWireService_master_reset(JNIEnv* env, jobject obj, jint masterId)
{
    jboolean result = JNI_FALSE;

    w1_master_id mid;
    convert_jint_to_master_id(&masterId, &mid);

    if(!sOneWireInterface)
    {
        LOGE("OneWire(1-Wire or w1) Stub operations not exist!");
    }
    else
    {
        result = sOneWireInterface->master_reset(mid);
    }
    return result;
}

/**
 * Package Name: 	see - register_android_server_onewire_OneWireService()
 * Class Name: 		OneWireService
 * Method Name:  	master_touch
*/
static jboolean android_onewire_OneWireService_master_touch(JNIEnv* env, jobject obj,
        jint masterId, jbyteArray dataIn, jint dataInLen, jbyteArray dataOut)
{
    jboolean result = JNI_FALSE;
    jint dataOutLen = 0;

    w1_master_id idOut;
    convert_jint_to_master_id(&masterId, &idOut);

    if(!sOneWireInterface)
    {
        LOGE("OneWire(1-Wire or w1) Stub operations not exist!");
    }
    else
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
 * Package Name: 	see - register_android_server_onewire_OneWireService()
 * Class Name: 		OneWireService
 * Method Name:  	master_read
*/
static jboolean android_onewire_OneWireService_master_read(JNIEnv* env, jobject obj,
        jint masterId, jint readLen, jbyteArray dataReadOut)
{
    jboolean result = JNI_FALSE;

    w1_master_id idOut;
    convert_jint_to_master_id(&masterId, &idOut);

    if(!sOneWireInterface)
    {
        LOGE("OneWire(1-Wire or w1) Stub operations not exist!");
    }
    else
    {
        jbyte* b = env->GetByteArrayElements(dataReadOut, NULL);

        result = sOneWireInterface->master_read(idOut, readLen, (BYTE *)b);

        env->ReleaseByteArrayElements(dataReadOut, b, 0);
    }
    return result;
}

/**
 * Package Name: 	see - register_android_server_onewire_OneWireService()
 * Class Name: 		OneWireService
 * Method Name:  	master_write
*/
static jboolean android_onewire_OneWireService_master_write(JNIEnv* env, jobject obj,
        jint masterId, jint writeLen, jbyteArray dataWriteIn)
{
    jboolean result = JNI_FALSE;

    w1_master_id idOut;
    convert_jint_to_master_id(&masterId, &idOut);

    if(!sOneWireInterface)
    {
        LOGE("OneWire(1-Wire or w1) Stub operations not exist!");
    }
    else
    {
        jbyte* b = env->GetByteArrayElements(dataWriteIn, NULL);

        result = sOneWireInterface->master_write(idOut, writeLen, (BYTE *)b);

        env->ReleaseByteArrayElements(dataWriteIn, b, 0);
    }
    return result;
}


// JNI native methods table ---------------------------------------------------------


static JNINativeMethod sMethods[] =
{
    /* name, 						signature, 				funcPtr */
    {"class_init_native", 			"()V", 					(void*)android_onewire_OneWireService_class_init_native},
    {"native_is_supported", 		"()Z", 					(void*)android_onewire_OneWireService_is_supported},
    {"native_start", 				"()Z", 					(void*)android_onewire_OneWireService_start},
    {"native_stop", 				"()V", 					(void*)android_onewire_OneWireService_stop},

    {"native_is_debug_enabled", 	"()Z", 					(void*)android_onewire_OneWireService_is_debug_enabled},
    {"native_set_debug_enabled", 	"(Z)V", 				(void*)android_onewire_OneWireService_set_debug_enabled},
    
    //{"native_begin_exclusive", 		"(I)Z", 					(void*)android_onewire_OneWireService_begin_exclusive},
    //{"native_end_exclusive", 		"(I)V", 					(void*)android_onewire_OneWireService_end_exclusive},
    {"native_begin_exclusive", 		"()Z", 					(void*)android_onewire_OneWireService_begin_exclusive},
    {"native_end_exclusive", 		"()V", 					(void*)android_onewire_OneWireService_end_exclusive},
    
	{"native_get_current_masters", 	"([I)I", 				(void*)android_onewire_OneWireService_get_current_masters},
	
    {"native_list_masters", 		"([I)I", 				(void*)android_onewire_OneWireService_list_masters},
    {"native_search_slaves", 		"(I[J)I", 				(void*)android_onewire_OneWireService_search_slaves},
    {"native_master_reset", 		"(I)Z", 				(void*)android_onewire_OneWireService_master_reset},
    {"native_master_touch", 		"(I[BI[B)Z", 			(void*)android_onewire_OneWireService_master_touch},
    {"native_master_read", 			"(II[B)Z", 				(void*)android_onewire_OneWireService_master_read},
    {"native_master_write", 		"(II[B)Z", 				(void*)android_onewire_OneWireService_master_write},

};




int register_android_server_onewire_OneWireService(JNIEnv* env)
{
    return jniRegisterNativeMethods(env, "com/android/server/OneWireService", sMethods, NELEM(sMethods));
    //return jniRegisterNativeMethods(env, "net/sh/android/onewire/legacy/OneWireService", sMethods, NELEM(sMethods));
}


} /* namespace android */
