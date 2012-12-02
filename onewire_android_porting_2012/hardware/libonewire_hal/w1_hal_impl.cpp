#define LOG_NDDEBUG 0

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <math.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <cutils/properties.h>
#include <cutils/sched_policy.h>
#include <utils/SystemClock.h>

#define  LOG_NDEBUG 0
#define  LOG_TAG "OneWireHALStubImpl"
#include <utils/Log.h>


#include <hardware/hardware.h>


#include "libonewire/sh_types.h"
#include "libonewire/w1_userspace.h"
#include "libonewire/w1_userservice.h"

#include "libonewire_hal/w1_hal.h"



//#define W1_SYSFS

#ifdef W1_SYSFS
    #include "libonewire/w1_sysfs_userservice.h"
    static w1_user_service * w1UserService = &w1_sysfs_userservice;
#else
	#include "libonewire/w1_netlink_userspace.h"
	#include "libonewire/w1_netlink_userservice.h"
	static w1_user_service * w1UserService = &w1_netlink_userservice;
#endif




static void w1hal_int_init(w1_user_callbacks * w1UserCallbacks);

static BOOL w1hal_int_start();

static void w1hal_int_stop();


static w1_master_id w1hal_int_get_current_master();

static void w1hal_int_get_current_slaves(w1_slave_rn * slaveIDs, int * slaveCount);

static BOOL w1hal_int_begin_exclusive_action(w1_master_id masterId);

static void w1hal_int_end_exclusive_action(w1_master_id masterId);


static BOOL w1hal_int_list_masters(w1_master_id * masters, int * pMasterCount);

static BOOL w1hal_int_search_slaves(w1_master_id masterId,
                      w1_slave_rn * slaves, int * pSlaveCount);

static BOOL w1hal_int_master_reset(w1_master_id masterId);


static BOOL w1hal_int_master_touch(w1_master_id masterId,
                    BYTE * dataIn, int dataInLen, BYTE * dataOut, int * pDataOutLen);

static BOOL w1hal_int_master_read(w1_master_id masterId, int readLen, BYTE * dataReadOut);

static BOOL w1hal_int_master_write(w1_master_id masterId, int writeLen, BYTE * dataWriteIn);



// Defines the onewire_interface in w1_hal.h
static const onewire_interface sW1HalInterface =
{
    sizeof(onewire_interface),
    w1hal_int_init,
	w1hal_int_start,
    w1hal_int_stop,

	//w1hal_int_get_current_master,
	//w1hal_int_get_current_slaves,
	w1hal_int_begin_exclusive_action,
	w1hal_int_end_exclusive_action,

    w1hal_int_search_slaves,
    w1hal_int_master_reset,

    w1hal_int_master_touch,
    w1hal_int_master_read,
    w1hal_int_master_write
};





static void w1hal_int_init(w1_user_callbacks * w1UserCallbacks)
{
    w1UserService->init(w1UserCallbacks);
}

static BOOL w1hal_int_start()
{
    return w1UserService->start();
}

static void w1hal_int_stop()
{
    w1UserService->stop();
}
/*
static w1_master_id w1hal_int_get_current_master()
{
    return w1UserService->get_current_master();
}

static void w1hal_int_get_current_slaves(w1_slave_rn * slaveIDs, int * slaveCount)
{
	w1UserService->get_current_slaves(slaveIDs, slaveCount);
}
*/

static BOOL w1hal_int_begin_exclusive_action(w1_master_id masterId)
{
	return w1UserService->begin_exclusive(masterId);
}

static void w1hal_int_end_exclusive_action(w1_master_id masterId)
{
	w1UserService->end_exclusive(masterId);
}



static BOOL w1hal_int_list_masters(w1_master_id * masters, int * pMasterCount)
{
    return w1UserService->list_masters(masters, pMasterCount);
}

static BOOL w1hal_int_search_slaves(w1_master_id masterId,
                      w1_slave_rn * slaves, int * pSlaveCount)
{
    return w1UserService->search_slaves(masterId, slaves, pSlaveCount);
}

static BOOL w1hal_int_master_reset(w1_master_id masterId)
{
    return w1UserService->master_reset(masterId);
}



static BOOL w1hal_int_master_touch(w1_master_id masterId,
                    BYTE * dataIn, int dataInLen, BYTE * dataOut, int * pDataOutLen)
{
    return w1UserService->master_touch(masterId, dataIn, dataInLen, dataOut, pDataOutLen);
}

static BOOL w1hal_int_master_read(w1_master_id masterId, int readLen, BYTE * dataReadOut)
{
    return w1UserService->master_read(masterId, readLen, dataReadOut);
}

static BOOL w1hal_int_master_write(w1_master_id masterId, int writeLen, BYTE * dataWriteIn)
{
    return w1UserService->master_write(masterId, writeLen, dataWriteIn);
}






const onewire_interface* w1_get_hardware_interface()
{
    char propBuf[PROPERTY_VALUE_MAX];

    // check to see if OneWire should be disabled
    property_get("onewire.disable", propBuf, "");
    if (propBuf[0] == '1')
    {
        LOGD("w1_get_hardware_interface returning NULL because onewire.disable=1\n");
        return NULL;
    }

    return &sW1HalInterface;
}



/**
 *  implement the extern interface inside w1_hal.c:
 *  extern const onewire_interface* ex_get_onewire_interface();
*/
extern "C" const onewire_interface* ex_get_onewire_interface()
{
    LOGD("[ONEWIRE_STUB_MODE] ex_get_onewire_interface called!");
    return &sW1HalInterface;
}





/**
 * ONEWIRE_LEGACY_MODE
 * implement the interface in w1_hal.h
 */
const onewire_interface* hw_get_onewire_interface()
{
    LOGD("[ONEWIRE_LEGACY_MODE] hw_get_onewire_interface called!");
    return &sW1HalInterface;
}

