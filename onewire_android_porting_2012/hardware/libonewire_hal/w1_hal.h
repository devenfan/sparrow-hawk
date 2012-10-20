/*
 *
 * Copyright (C) 2011 Deven Fan <deven.fan@gmail.com>
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



/* The most important header */
//#include <hardware/hardware.h>

//#include <fcntl.h>
//#include <errno.h>

//#include <cutils/log.h>
//#include <cutils/atomic.h>

//#include "sh_types.h"

//#include "w1_netlink_userspace.h"
//#include "w1_netlink_userservice.h"


/***************************************************************************/

struct w1hal_module_t {

    /* struct hw_module_t must come first, and named common */
    struct hw_module_t common;

    /* Place attributes here. */

    /* Place methods here. */

};





typedef struct w1hal_device_operations {

    /** set to sizeof(w1hal_interface) */
    size_t size;

    void (*init)(w1_user_callbacks * w1UserCallbacks);

    BOOL (*start)();

    void (*stop)();

    /** Get current master. */
    w1_master_id (*get_current_master)();

    /** Get current slaves. */
    void (*get_current_slaves)(w1_slave_rn * slaveIDs, int * slaveCount);

    /** Begin exclusive action. */
    BOOL (*begin_exclusive)(w1_master_id masterId);

    /** End exclusive action. */
    void (*end_exclusive)(w1_master_id masterId);

    /** List all the Masters
    BOOL (*list_masters)(w1_master_id * masters, int * pMasterCount);*/

    /** Search Slaves by Master Id */
    BOOL (*search_slaves)(w1_master_id masterId, w1_slave_rn * slaves, int * pSlaveCount);

    /** Reset the Master */
    BOOL (*master_reset)(w1_master_id masterId);

    /** Rouch data on the Master */
    BOOL (*master_touch)(w1_master_id masterId,
                    BYTE * dataIn, int dataInLen, BYTE * dataOut, int * pDataOutLen);

    /** Read data from the Master */
    BOOL (*master_read)(w1_master_id masterId, int readLen, BYTE * dataReadOut);

    /** Write data to the Master */
    BOOL (*master_write)(w1_master_id masterId, int writeLen, BYTE * dataWriteIn);

}w1hal_interface;




struct w1hal_device_t {

    /* struct hw_device_t must come first, and named common */
    struct hw_device_t common;

    /* attributes */
    //int fd;

    /* Useful APIs for upper layer */
    const w1hal_interface* (*get_w1_interface)(struct w1hal_device_t* dev);
};



#ifdef __cplusplus
extern "C" {
#endif

/** ONEWIRE_LEGACY_MODE */
const w1hal_interface* hw_get_w1_interface();

#ifdef __cplusplus
}
#endif

/***************************************************************************/

/* Module ID must be claimed. */
//#define W1HAL_HARDWARE_MODULE_ID "w1hal"
#define ONEWIRE_HARDWARE_MODULE_ID "OneWire"
