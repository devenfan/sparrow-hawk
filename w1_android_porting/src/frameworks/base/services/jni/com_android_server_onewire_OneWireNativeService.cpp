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

#include "sh_types.h"
#include "sh_error.h"
#include "sh_util.h"
#include "sh_thread.h"
#include "kernel_connector.h"
#include "w1_netlink_userspace.h"
#include "w1_netlink_util.h"
#include "w1_netlink_userservice.h"

#include "hardware/w1_hal.h"

static jobject mCallbacksObj = NULL;


static jmethodID method_masterAdded;
static jmethodID method_masterRemoved;
static jmethodID method_slaveAdded;
static jmethodID method_slaveRemoved;


static const w1hal_interface * sOneWireInterface = NULL;


//#define WAKE_LOCK_NAME  "OneWireWakeLock"

#define MAX_SLAVE_COUNT  10
#define MAX_MASTER_COUNT  3

namespace android {


static void checkAndClearExceptionFromCallback(JNIEnv* env, const char* methodName) {
    if (env->ExceptionCheck()) {
        LOGE("An exception was thrown by callback '%s'.", methodName);
        //LOGE_EX(env);
        env->ExceptionClear();
    }
}



static void master_added_callback(w1_master_id masterId)
{
    JNIEnv* env = AndroidRuntime::getJNIEnv();
	env->CallVoidMethod(mCallbacksObj, method_masterAdded, *((jint*)&masterId));
    checkAndClearExceptionFromCallback(env, __FUNCTION__);
}

static void master_removed_callback(w1_master_id masterId)
{
	JNIEnv* env = AndroidRuntime::getJNIEnv();
	env->CallVoidMethod(mCallbacksObj, method_masterRemoved, *((jint*)&masterId));
    checkAndClearExceptionFromCallback(env, __FUNCTION__);
}

static void slave_added_callback(w1_slave_rn slaveRN)
{
	JNIEnv* env = AndroidRuntime::getJNIEnv();
    env->CallVoidMethod(mCallbacksObj, method_slaveAdded, *((jlong*)&slaveRN));
    checkAndClearExceptionFromCallback(env, __FUNCTION__);
}

static void slave_removed_callback(w1_slave_rn slaveRN)
{
	JNIEnv* env = AndroidRuntime::getJNIEnv();
    env->CallVoidMethod(mCallbacksObj, method_slaveRemoved, *((jlong*)&slaveRN));
    checkAndClearExceptionFromCallback(env, __FUNCTION__);
}



w1_user_callbacks sW1UserCallbacks = {

    master_added_callback,
    master_removed_callback,
    slave_added_callback,
    slave_removed_callback,
};


/*
static void slave_added_callback(w1_master_id masterId)
{
    JNIEnv* env = AndroidRuntime::getJNIEnv();
    env->CallVoidMethod(mCallbacksObj, method_reportLocation,
                        location->flags,
            (jdouble)location->latitude, (jdouble)location->longitude,
            (jdouble)location->altitude,
            (jfloat)location->speed, (jfloat)location->bearing,
            (jfloat)location->accuracy, (jlong)location->timestamp);
    checkAndClearExceptionFromCallback(env, __FUNCTION__);
}

static void status_callback(GpsStatus* status)
{
    JNIEnv* env = AndroidRuntime::getJNIEnv();
    env->CallVoidMethod(mCallbacksObj, method_reportStatus, status->status);
    checkAndClearExceptionFromCallback(env, __FUNCTION__);
}

static void sv_status_callback(GpsSvStatus* sv_status)
{
    JNIEnv* env = AndroidRuntime::getJNIEnv();
    memcpy(&sGpsSvStatus, sv_status, sizeof(sGpsSvStatus));
    env->CallVoidMethod(mCallbacksObj, method_reportSvStatus);
    checkAndClearExceptionFromCallback(env, __FUNCTION__);
}

static void nmea_callback(GpsUtcTime timestamp, const char* nmea, int length)
{
    JNIEnv* env = AndroidRuntime::getJNIEnv();
    // The Java code will call back to read these values
    // We do this to avoid creating unnecessary String objects
    sNmeaString = nmea;
    sNmeaStringLength = length;
    env->CallVoidMethod(mCallbacksObj, method_reportNmea, timestamp);
    checkAndClearExceptionFromCallback(env, __FUNCTION__);
}

static void set_capabilities_callback(uint32_t capabilities)
{
    LOGD("set_capabilities_callback: %ld\n", capabilities);
    JNIEnv* env = AndroidRuntime::getJNIEnv();
    env->CallVoidMethod(mCallbacksObj, method_setEngineCapabilities, capabilities);
    checkAndClearExceptionFromCallback(env, __FUNCTION__);
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
    return (pthread_t)AndroidRuntime::createJavaThread(name, start, arg);
}

GpsCallbacks sGpsCallbacks = {
    sizeof(GpsCallbacks),
    location_callback,
    status_callback,
    sv_status_callback,
    nmea_callback,
    set_capabilities_callback,
    acquire_wakelock_callback,
    release_wakelock_callback,
    create_thread_callback,
};
*/

/*
static void xtra_download_request_callback()
{
    JNIEnv* env = AndroidRuntime::getJNIEnv();
    env->CallVoidMethod(mCallbacksObj, method_xtraDownloadRequest);
    checkAndClearExceptionFromCallback(env, __FUNCTION__);
}

GpsXtraCallbacks sGpsXtraCallbacks = {
    xtra_download_request_callback,
    create_thread_callback,
};
*/

/*
static void agps_status_callback(AGpsStatus* agps_status)
{
    JNIEnv* env = AndroidRuntime::getJNIEnv();

    uint32_t ipaddr;
    // ipaddr field was not included in original AGpsStatus
    if (agps_status->size >= sizeof(AGpsStatus))
        ipaddr = agps_status->ipaddr;
    else
        ipaddr = 0xFFFFFFFF;
    env->CallVoidMethod(mCallbacksObj, method_reportAGpsStatus,
                        agps_status->type, agps_status->status, ipaddr);
    checkAndClearExceptionFromCallback(env, __FUNCTION__);
}

AGpsCallbacks sAGpsCallbacks = {
    agps_status_callback,
    create_thread_callback,
};

static void gps_ni_notify_callback(GpsNiNotification *notification)
{
    LOGD("gps_ni_notify_callback\n");
    JNIEnv* env = AndroidRuntime::getJNIEnv();
    jstring requestor_id = env->NewStringUTF(notification->requestor_id);
    jstring text = env->NewStringUTF(notification->text);
    jstring extras = env->NewStringUTF(notification->extras);

    if (requestor_id && text && extras) {
        env->CallVoidMethod(mCallbacksObj, method_reportNiNotification,
            notification->notification_id, notification->ni_type,
            notification->notify_flags, notification->timeout,
            notification->default_response, requestor_id, text,
            notification->requestor_id_encoding,
            notification->text_encoding, extras);
    } else {
        LOGE("out of memory in gps_ni_notify_callback\n");
    }

    if (requestor_id)
        env->DeleteLocalRef(requestor_id);
    if (text)
        env->DeleteLocalRef(text);
    if (extras)
        env->DeleteLocalRef(extras);
    checkAndClearExceptionFromCallback(env, __FUNCTION__);
}

GpsNiCallbacks sGpsNiCallbacks = {
    gps_ni_notify_callback,
    create_thread_callback,
};
*/

/*
static void agps_request_set_id(uint32_t flags)
{
    JNIEnv* env = AndroidRuntime::getJNIEnv();
    env->CallVoidMethod(mCallbacksObj, method_requestSetID, flags);
    checkAndClearExceptionFromCallback(env, __FUNCTION__);
}

static void agps_request_ref_location(uint32_t flags)
{
    JNIEnv* env = AndroidRuntime::getJNIEnv();
    env->CallVoidMethod(mCallbacksObj, method_requestRefLocation, flags);
    checkAndClearExceptionFromCallback(env, __FUNCTION__);
}

AGpsRilCallbacks sAGpsRilCallbacks = {
    agps_request_set_id,
    agps_request_ref_location,
    create_thread_callback,
};
*/


static void android_onewire_OneWireNativeService_class_init_native(JNIEnv* env, jclass clazz) {
    int err;
    hw_module_t* module;

    method_masterAdded      = env->GetMethodID(clazz, "masterAdded", "(I)V");
    method_masterRemoved    = env->GetMethodID(clazz, "masterRemoved", "(I)V");
    method_slaveAdded       = env->GetMethodID(clazz, "slaveAdded", "(J)V");
    method_slaveRemoved     = env->GetMethodID(clazz, "slaveRemoved", "(J)V");

	/*
    method_reportLocation = env->GetMethodID(clazz, "reportLocation", "(IDDDFFFJ)V");
    method_reportStatus = env->GetMethodID(clazz, "reportStatus", "(I)V");
    method_reportSvStatus = env->GetMethodID(clazz, "reportSvStatus", "()V");
    method_reportAGpsStatus = env->GetMethodID(clazz, "reportAGpsStatus", "(III)V");
    method_reportNmea = env->GetMethodID(clazz, "reportNmea", "(J)V");
    method_setEngineCapabilities = env->GetMethodID(clazz, "setEngineCapabilities", "(I)V");
    method_xtraDownloadRequest = env->GetMethodID(clazz, "xtraDownloadRequest", "()V");
    method_reportNiNotification = env->GetMethodID(clazz, "reportNiNotification",
            "(IIIIILjava/lang/String;Ljava/lang/String;IILjava/lang/String;)V");
    method_requestRefLocation = env->GetMethodID(clazz,"requestRefLocation","(I)V");
    method_requestSetID = env->GetMethodID(clazz,"requestSetID","(I)V");
	*/

    //err = hw_get_module(GPS_HARDWARE_MODULE_ID, (hw_module_t const**)&module);
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
    }

}


/**
 * Package Name: 	see - register_android_server_onewire_OneWireNativeService()
 * Class Name: 		OneWireNativeService
 * Method Name:  	is_supported
*/
static jboolean android_onewire_OneWireNativeService_is_supported(JNIEnv* env, jclass clazz) {
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
        mCallbacksObj = env->NewGlobalRef(obj);

	if(!sOneWireInterface)
		return JNI_FALSE;

	return (sOneWireInterface->start(&sW1UserCallbacks));
}

/**
 * Package Name: 	see - register_android_server_onewire_OneWireNativeService()
 * Class Name: 		OneWireNativeService
 * Method Name:  	stop
*/
static void android_onewire_OneWireNativeService_stop(JNIEnv* env, jobject obj)
{
	if(sOneWireInterface)
		sOneWireInterface->stop();
}

/**
 * Package Name: 	see - register_android_server_onewire_OneWireNativeService()
 * Class Name: 		OneWireNativeService
 * Method Name:  	get_current_master
*/
static jint android_onewire_OneWireNativeService_get_current_master(JNIEnv* env, jobject obj)
{
	if(sOneWireInterface)
		return sOneWireInterface->get_master_id();
	else
		return 0;
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

    if(sOneWireInterface)
    {
    	sOneWireInterface->get_slave_ids(slaves, &slaveCount);
    	if(slaveCount > 0)
		{
			if(slaveCount > MAX_SLAVE_COUNT)
				slaveCount = MAX_SLAVE_COUNT;

			jlong* l = env->GetLongArrayElements(slaveRNs, NULL);

			memcpy(l, slaves, sizeof(w1_slave_rn) * slaveCount);

			env->ReleaseLongArrayElements(slaveRNs, l, 0);
		}
    }

	return slaveCount;
}

/**
 * Package Name: 	see - register_android_server_onewire_OneWireNativeService()
 * Class Name: 		OneWireNativeService
 * Method Name:  	begin_exclusive
*/
static void android_onewire_OneWireNativeService_begin_exclusive(JNIEnv* env, jobject obj)
{
	if(sOneWireInterface)
		sOneWireInterface->begin_exclusive();
}

/**
 * Package Name: 	see - register_android_server_onewire_OneWireNativeService()
 * Class Name: 		OneWireNativeService
 * Method Name:  	end_exclusive
*/
static void android_onewire_OneWireNativeService_end_exclusive(JNIEnv* env, jobject obj)
{
	if(sOneWireInterface)
		sOneWireInterface->end_exclusive();
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

	return masterCount;
}

/**
 * Package Name: 	see - register_android_server_onewire_OneWireNativeService()
 * Class Name: 		OneWireNativeService
 * Method Name:  	search_slaves
*/
static jint android_onewire_OneWireNativeService_search_slaves(JNIEnv* env, jobject obj,
                jint masterId, jboolean isSearchAlarm, jlongArray slaveRNs)
{
	jint slaveCount = 0;
	w1_slave_rn slaves[MAX_SLAVE_COUNT];

    if(sOneWireInterface)
    {
    	if(sOneWireInterface->search_slaves(*((w1_master_id*)&masterId), (BOOL)isSearchAlarm, slaves, &slaveCount))
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
	if(sOneWireInterface)
	{
		result = sOneWireInterface->master_reset(*((w1_master_id*)&masterId));
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

    if(sOneWireInterface)
    {
    	jbyte* b1 = env->GetByteArrayElements(dataIn, NULL);
    	jbyte* b2 = env->GetByteArrayElements(dataOut, NULL);

    	result = sOneWireInterface->master_touch(*((w1_master_id*)&masterId),
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
    if(sOneWireInterface)
    {
    	jbyte* b = env->GetByteArrayElements(dataReadOut, NULL);

		result = sOneWireInterface->master_read(*((w1_master_id*)&masterId), readLen, (BYTE *)b);

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
    if(sOneWireInterface)
    {
    	jbyte* b = env->GetByteArrayElements(dataWriteIn, NULL);

		result = sOneWireInterface->master_write(*((w1_master_id*)&masterId), writeLen, (BYTE *)b);

		env->ReleaseByteArrayElements(dataWriteIn, b, 0);
    }
    return result;
}


/*
static jboolean android_location_GpsLocationProvider_set_position_mode(JNIEnv* env, jobject obj,
        jint mode, jint recurrence, jint min_interval, jint preferred_accuracy, jint preferred_time)
{
    if (sGpsInterface)
        return (sGpsInterface->set_position_mode(mode, recurrence, min_interval, preferred_accuracy,
                preferred_time) == 0);
    else
        return false;
}

static jboolean android_location_GpsLocationProvider_start(JNIEnv* env, jobject obj)
{
    if (sGpsInterface)
        return (sGpsInterface->start() == 0);
    else
        return false;
}

static jboolean android_location_GpsLocationProvider_stop(JNIEnv* env, jobject obj)
{
    if (sGpsInterface)
        return (sGpsInterface->stop() == 0);
    else
        return false;
}

static void android_location_GpsLocationProvider_delete_aiding_data(JNIEnv* env, jobject obj, jint flags)
{
    if (sGpsInterface)
        sGpsInterface->delete_aiding_data(flags);
}

static jint android_location_GpsLocationProvider_read_sv_status(JNIEnv* env, jobject obj,
        jintArray prnArray, jfloatArray snrArray, jfloatArray elevArray, jfloatArray azumArray,
        jintArray maskArray)
{
    // this should only be called from within a call to reportSvStatus

    jint* prns = env->GetIntArrayElements(prnArray, 0);
    jfloat* snrs = env->GetFloatArrayElements(snrArray, 0);
    jfloat* elev = env->GetFloatArrayElements(elevArray, 0);
    jfloat* azim = env->GetFloatArrayElements(azumArray, 0);
    jint* mask = env->GetIntArrayElements(maskArray, 0);

    int num_svs = sGpsSvStatus.num_svs;
    for (int i = 0; i < num_svs; i++) {
        prns[i] = sGpsSvStatus.sv_list[i].prn;
        snrs[i] = sGpsSvStatus.sv_list[i].snr;
        elev[i] = sGpsSvStatus.sv_list[i].elevation;
        azim[i] = sGpsSvStatus.sv_list[i].azimuth;
    }
    mask[0] = sGpsSvStatus.ephemeris_mask;
    mask[1] = sGpsSvStatus.almanac_mask;
    mask[2] = sGpsSvStatus.used_in_fix_mask;

    env->ReleaseIntArrayElements(prnArray, prns, 0);
    env->ReleaseFloatArrayElements(snrArray, snrs, 0);
    env->ReleaseFloatArrayElements(elevArray, elev, 0);
    env->ReleaseFloatArrayElements(azumArray, azim, 0);
    env->ReleaseIntArrayElements(maskArray, mask, 0);
    return num_svs;
}

static void android_location_GpsLocationProvider_agps_set_reference_location_cellid(JNIEnv* env,
        jobject obj, jint type, jint mcc, jint mnc, jint lac, jint cid)
{
    AGpsRefLocation location;

    if (!sAGpsRilInterface) {
        LOGE("no AGPS RIL interface in agps_set_reference_location_cellid");
        return;
    }

    switch(type) {
        case AGPS_REF_LOCATION_TYPE_GSM_CELLID:
        case AGPS_REF_LOCATION_TYPE_UMTS_CELLID:
            location.type = type;
            location.u.cellID.mcc = mcc;
            location.u.cellID.mnc = mnc;
            location.u.cellID.lac = lac;
            location.u.cellID.cid = cid;
            break;
        default:
            LOGE("Neither a GSM nor a UMTS cellid (%s:%d).",__FUNCTION__,__LINE__);
            return;
            break;
    }
    sAGpsRilInterface->set_ref_location(&location, sizeof(location));
}

static void android_location_GpsLocationProvider_agps_send_ni_message(JNIEnv* env,
        jobject obj, jbyteArray ni_msg, jint size)
{
    size_t sz;

    if (!sAGpsRilInterface) {
        LOGE("no AGPS RIL interface in send_ni_message");
        return;
    }
    if (size < 0)
        return;
    sz = (size_t)size;
    jbyte* b = env->GetByteArrayElements(ni_msg, 0);
    sAGpsRilInterface->ni_message((uint8_t *)b,sz);
    env->ReleaseByteArrayElements(ni_msg,b,0);
}

static void android_location_GpsLocationProvider_agps_set_id(JNIEnv *env,
        jobject obj, jint type, jstring  setid_string)
{
    if (!sAGpsRilInterface) {
        LOGE("no AGPS RIL interface in agps_set_id");
        return;
    }

    const char *setid = env->GetStringUTFChars(setid_string, NULL);
    sAGpsRilInterface->set_set_id(type, setid);
    env->ReleaseStringUTFChars(setid_string, setid);
}

static jint android_location_GpsLocationProvider_read_nmea(JNIEnv* env, jobject obj,
                                            jbyteArray nmeaArray, jint buffer_size)
{
    // this should only be called from within a call to reportNmea
    jbyte* nmea = (jbyte *)env->GetPrimitiveArrayCritical(nmeaArray, 0);
    int length = sNmeaStringLength;
    if (length > buffer_size)
        length = buffer_size;
    memcpy(nmea, sNmeaString, length);
    env->ReleasePrimitiveArrayCritical(nmeaArray, nmea, JNI_ABORT);
    return length;
}

static void android_location_GpsLocationProvider_inject_time(JNIEnv* env, jobject obj,
        jlong time, jlong timeReference, jint uncertainty)
{
    if (sGpsInterface)
        sGpsInterface->inject_time(time, timeReference, uncertainty);
}

static void android_location_GpsLocationProvider_inject_location(JNIEnv* env, jobject obj,
        jdouble latitude, jdouble longitude, jfloat accuracy)
{
    if (sGpsInterface)
        sGpsInterface->inject_location(latitude, longitude, accuracy);
}

static jboolean android_location_GpsLocationProvider_supports_xtra(JNIEnv* env, jobject obj)
{
    return (sGpsXtraInterface != NULL);
}

static void android_location_GpsLocationProvider_inject_xtra_data(JNIEnv* env, jobject obj,
        jbyteArray data, jint length)
{
    if (!sGpsXtraInterface) {
        LOGE("no XTRA interface in inject_xtra_data");
        return;
    }

    jbyte* bytes = (jbyte *)env->GetPrimitiveArrayCritical(data, 0);
    sGpsXtraInterface->inject_xtra_data((char *)bytes, length);
    env->ReleasePrimitiveArrayCritical(data, bytes, JNI_ABORT);
}

static void android_location_GpsLocationProvider_agps_data_conn_open(JNIEnv* env, jobject obj, jstring apn)
{
    if (!sAGpsInterface) {
        LOGE("no AGPS interface in agps_data_conn_open");
        return;
    }
    if (apn == NULL) {
        jniThrowException(env, "java/lang/IllegalArgumentException", NULL);
        return;
    }
    const char *apnStr = env->GetStringUTFChars(apn, NULL);
    sAGpsInterface->data_conn_open(apnStr);
    env->ReleaseStringUTFChars(apn, apnStr);
}

static void android_location_GpsLocationProvider_agps_data_conn_closed(JNIEnv* env, jobject obj)
{
    if (!sAGpsInterface) {
        LOGE("no AGPS interface in agps_data_conn_open");
        return;
    }
    sAGpsInterface->data_conn_closed();
}

static void android_location_GpsLocationProvider_agps_data_conn_failed(JNIEnv* env, jobject obj)
{
    if (!sAGpsInterface) {
        LOGE("no AGPS interface in agps_data_conn_open");
        return;
    }
    sAGpsInterface->data_conn_failed();
}

static void android_location_GpsLocationProvider_set_agps_server(JNIEnv* env, jobject obj,
        jint type, jstring hostname, jint port)
{
    if (!sAGpsInterface) {
        LOGE("no AGPS interface in agps_data_conn_open");
        return;
    }
    const char *c_hostname = env->GetStringUTFChars(hostname, NULL);
    sAGpsInterface->set_server(type, c_hostname, port);
    env->ReleaseStringUTFChars(hostname, c_hostname);
}

static void android_location_GpsLocationProvider_send_ni_response(JNIEnv* env, jobject obj,
      jint notifId, jint response)
{
    if (!sGpsNiInterface) {
        LOGE("no NI interface in send_ni_response");
        return;
    }

    sGpsNiInterface->respond(notifId, response);
}

static jstring android_location_GpsLocationProvider_get_internal_state(JNIEnv* env, jobject obj)
{
    jstring result = NULL;
    if (sGpsDebugInterface) {
        const size_t maxLength = 2047;
        char buffer[maxLength+1];
        size_t length = sGpsDebugInterface->get_internal_state(buffer, maxLength);
        if (length > maxLength) length = maxLength;
        buffer[length] = 0;
        result = env->NewStringUTF(buffer);
    }
    return result;
}

static void android_location_GpsLocationProvider_update_network_state(JNIEnv* env, jobject obj,
        jboolean connected, int type, jboolean roaming, jboolean available, jstring extraInfo, jstring apn)
{

    if (sAGpsRilInterface && sAGpsRilInterface->update_network_state) {
        if (extraInfo) {
            const char *extraInfoStr = env->GetStringUTFChars(extraInfo, NULL);
            sAGpsRilInterface->update_network_state(connected, type, roaming, extraInfoStr);
            env->ReleaseStringUTFChars(extraInfo, extraInfoStr);
        } else {
            sAGpsRilInterface->update_network_state(connected, type, roaming, NULL);
        }

        // update_network_availability callback was not included in original AGpsRilInterface
        if (sAGpsRilInterface->size >= sizeof(AGpsRilInterface)
                && sAGpsRilInterface->update_network_availability) {
            const char *c_apn = env->GetStringUTFChars(apn, NULL);
            sAGpsRilInterface->update_network_availability(available, c_apn);
            env->ReleaseStringUTFChars(apn, c_apn);
        }
    }
}
*/



static JNINativeMethod sMethods[] = {
     /* name, 						signature, 				funcPtr */
    {"class_init_native", 			"()V", 					(void*)android_onewire_OneWireNativeService_class_init_native},
    {"native_is_supported", 		"()Z", 					(void*)android_onewire_OneWireNativeService_is_supported},
    {"native_start", 				"()Z", 					(void*)android_onewire_OneWireNativeService_start},
    {"native_stop", 				"()V", 					(void*)android_onewire_OneWireNativeService_stop},

    {"native_get_current_master", 	"()I", 					(void*)android_onewire_OneWireNativeService_get_current_master},
    {"native_get_current_slaves", 	"([J)I", 				(void*)android_onewire_OneWireNativeService_get_current_slaves},
    {"native_begin_exclusive", 		"()V", 					(void*)android_onewire_OneWireNativeService_begin_exclusive},
    {"native_end_exclusive", 		"()V", 					(void*)android_onewire_OneWireNativeService_end_exclusive},

    {"native_list_masters", 		"([I)I", 				(void*)android_onewire_OneWireNativeService_list_masters},
    {"native_search_slaves", 		"(IZ[J)I", 				(void*)android_onewire_OneWireNativeService_search_slaves},
    {"native_master_reset", 		"(I)Z", 				(void*)android_onewire_OneWireNativeService_master_reset},
    {"native_master_touch", 		"(I[BI[B)Z", 			(void*)android_onewire_OneWireNativeService_master_touch},
    {"native_master_read", 			"(II[B)Z", 				(void*)android_onewire_OneWireNativeService_master_read},
    {"native_master_write", 		"(II[B)Z", 				(void*)android_onewire_OneWireNativeService_master_write},

};

int register_android_server_onewire_OneWireNativeService(JNIEnv* env)
{
    return jniRegisterNativeMethods(env, "com/android/server/onewire/OneWireNativeService", sMethods, NELEM(sMethods));
}


} /* namespace android */
