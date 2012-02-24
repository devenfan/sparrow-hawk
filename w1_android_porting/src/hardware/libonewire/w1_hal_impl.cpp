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

#define LOG_TAG "lib_w1hal"
#include <utils/Log.h>


#include <hardware/hardware.h>


#include "sh_types.h"
//#include "sh_error.h"
//#include "sh_util.h"
//#include "sh_thread.h"
//#include "kernel_connector.h"
#include "w1_netlink_userspace.h"
//#include "w1_netlink_util.h"
#include "w1_netlink_userservice.h"


#include <hardware/w1_hal.h>



static BOOL w1hal_int_start(w1_user_callbacks * w1UserCallbacks);

static BOOL w1hal_int_stop(void);


static w1_master_id w1hal_int_get_master_id(void);

static void w1hal_int_get_slave_ids(w1_slave_rn * slaveIDs, int * slaveCount);

static void w1hal_int_begin_exclusive_action(void);

static void w1hal_int_end_exclusive_action(void);


static BOOL w1hal_int_list_masters(w1_master_id * masters, int * pMasterCount);

static BOOL w1hal_int_search_slaves(w1_master_id masterId, BOOL isSearchAlarm,
                      w1_slave_rn * slaves, int * pSlaveCount);

static BOOL w1hal_int_master_reset(w1_master_id masterId);


static BOOL w1hal_int_master_touch(w1_master_id masterId,
                    BYTE * dataIn, int dataInLen, BYTE * dataOut, int * pDataOutLen);

static BOOL w1hal_int_master_read(w1_master_id masterId, int readLen, BYTE * dataReadOut);

static BOOL w1hal_int_master_write(w1_master_id masterId, int writeLen, BYTE * dataWriteIn);



// Defines the w1hal_interface in w1_hal.h
static const w1hal_interface sW1HalInterface =
{
    sizeof(w1hal_interface),
    w1hal_int_start,
    w1hal_int_stop,

	w1hal_int_get_master_id,
	w1hal_int_get_slave_ids,
	w1hal_int_begin_exclusive_action,
	w1hal_int_end_exclusive_action,

    w1hal_int_list_masters,
    w1hal_int_search_slaves,
    w1hal_int_master_reset,

    w1hal_int_master_touch,
    w1hal_int_master_read,
    w1hal_int_master_write
};






static BOOL w1hal_int_start(w1_user_callbacks * w1UserCallbacks)
{
    return w1_netlink_userservice_start(w1UserCallbacks);
}

static BOOL w1hal_int_stop(void)
{
    return w1_netlink_userservice_stop();
}

static w1_master_id w1hal_int_get_master_id(void)
{
    return get_w1_master_id();
}

static void w1hal_int_get_slave_ids(w1_slave_rn * slaveIDs, int * slaveCount)
{
	get_w1_slave_ids(slaveIDs, slaveCount);
}

static void w1hal_int_begin_exclusive_action(void)
{
	pause_w1_searching_thread();
}

static void w1hal_int_end_exclusive_action(void)
{
	wakeup_w1_searching_thread();
}



static BOOL w1hal_int_list_masters(w1_master_id * masters, int * pMasterCount)
{
    return w1_list_masters(masters, pMasterCount);
}

static BOOL w1hal_int_search_slaves(w1_master_id masterId, BOOL isSearchAlarm,
                      w1_slave_rn * slaves, int * pSlaveCount)
{
    return w1_master_search(masterId, isSearchAlarm, slaves, pSlaveCount);
}

static BOOL w1hal_int_master_reset(w1_master_id masterId)
{
    return w1_master_reset(masterId);
}



static BOOL w1hal_int_master_touch(w1_master_id masterId,
                    BYTE * dataIn, int dataInLen, BYTE * dataOut, int * pDataOutLen)
{
    return w1_master_touch(masterId, dataIn, dataInLen, dataOut, pDataOutLen);
}

static BOOL w1hal_int_master_read(w1_master_id masterId, int readLen, BYTE * dataReadOut)
{
    return w1_master_read(masterId, readLen, dataReadOut);
}

static BOOL w1hal_int_master_write(w1_master_id masterId, int writeLen, BYTE * dataWriteIn)
{
    return w1_master_write(masterId, writeLen, dataWriteIn);
}






const w1hal_interface* w1_get_hardware_interface()
{
    char propBuf[PROPERTY_VALUE_MAX];

    // check to see if OneWire should be disabled
    property_get("onewire.disable", propBuf, "");
    if (propBuf[0] == '1')
    {
        LOGD("w1_get_interface returning NULL because onewire.disable=1\n");
        return NULL;
    }

    return &sW1HalInterface;
}





// for w1_hal.c
extern "C" const w1hal_interface* get_w1_interface()
{
    return &sW1HalInterface;
}



