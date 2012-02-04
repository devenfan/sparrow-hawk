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
#include "sh_error.h"
#include "sh_util.h"
#include "sh_thread.h"
#include "kernel_connector.h"
#include "w1_netlink_userspace.h"
#include "w1_netlink_util.h"
#include "w1_netlink_userservice.h"


#include "w1_hal.h" //<hardware/w1_hal.h>



static BOOL w1hal_int_start(w1_user_callbacks * w1UserCallbacks);

static BOOL w1hal_int_stop(void);

static BOOL w1hal_int_list_masters(w1_master_id * masters, int * pMasterCount);

static BOOL w1hal_int_search_slaves(w1_master_id masterId, BOOL isSearchAlarm,
                      w1_slave_rn * slaves, int * pSlaveCount);

static BOOL w1hal_int_master_reset(w1_master_id masterId);

/*
static BOOL w1hal_int_touch(BYTE * slaveId, int idLen,
                    void * dataIn, int dataInLen, void ** pDataOut, int * pDataOutLen);

static BOOL w1hal_int_slave_read(BYTE * slaveId, int idLen,
                    void * dataIn, int dataInLen, void ** pDataOut, int * pDataOutLen);

static BOOL w1hal_int_slave_write(BYTE * slaveId, int idLen,
                    void * dataIn, int dataInLen);
*/

static BOOL w1hal_int_touch_data(w1_master_id masterId,
                    BYTE * dataIn, int dataInLen, BYTE * dataOut, int * pDataOutLen);

static BOOL w1hal_int_read_data(w1_master_id masterId,
                    BYTE * dataIn, int dataInLen, BYTE * dataOut, int * pDataOutLen);

static BOOL w1hal_int_write_data(w1_master_id masterId, BYTE * dataIn, int dataInLen);




// Defines the w1hal_interface in w1_hal.h
static const w1hal_interface sW1HalInterface =
{
    sizeof(w1hal_interface),
    w1hal_int_start,
    w1hal_int_stop,
    w1hal_int_list_masters,
    w1hal_int_search_slaves,
    w1hal_int_master_reset,
    w1hal_int_touch_data,
    w1hal_int_read_data,
    w1hal_int_write_data
};






static BOOL w1hal_int_start(w1_user_callbacks * w1UserCallbacks)
{
    return w1_netlink_userservice_start(w1UserCallbacks);
}

static BOOL w1hal_int_stop(void)
{
    return w1_netlink_userservice_stop();
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


static BOOL w1hal_int_touch_data(w1_master_id masterId,
                    BYTE * dataIn, int dataInLen, BYTE * dataOut, int * pDataOutLen)
{
    return w1_process_cmd((BYTE *)&masterId, sizeof(w1_master_id), W1_CMD_TOUCH,
                          dataIn, dataInLen, dataOut, pDataOutLen);
}

static BOOL w1hal_int_read_data(w1_master_id masterId,
                    BYTE * dataIn, int dataInLen, BYTE * dataOut, int * pDataOutLen)
{
    return w1_process_cmd((BYTE *)&masterId, sizeof(w1_master_id), W1_CMD_READ,
                          dataIn, dataInLen, dataOut, pDataOutLen);
}

static BOOL w1hal_int_write_data(w1_master_id masterId, BYTE * dataIn, int dataInLen)
{
    int dataOutLen = 0;
    BYTE dataOut[128];
    memset(dataOut, 0, sizeof(BYTE) * 128);

    return w1_process_cmd((BYTE *)&masterId, sizeof(w1_master_id), W1_CMD_WRITE,
                          dataIn, dataInLen, dataOut, &dataOutLen);
}






const w1hal_interface* w1_get_hardware_interface()
{
    char propBuf[PROPERTY_VALUE_MAX];

    // check to see if GPS should be disabled
    property_get("w1.disable", propBuf, "");
    if (propBuf[0] == '1')
    {
        LOGD("w1_get_interface returning NULL because w1.disable=1\n");
        return NULL;
    }

    return &sW1HalInterface;
}





// for w1_hal.c
extern "C" const w1hal_interface* get_w1_interface()
{
    return &sW1HalInterface;
}


