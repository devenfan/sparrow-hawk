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


/***************************************************************************/


struct onewire_module_t {

    /* struct hw_module_t must come first, and named common */
    struct hw_module_t common;

    /* Place attributes here. */

    /* Place methods here. */

};




typedef struct onewire_device_operations {

    /** set to sizeof(onewire_interface) */
    size_t size;

    void (*init)(w1_user_callbacks * onewireUserCallbacks);

    BOOL (*start)();

    void (*stop)();

	BOOL (*is_debug_enabled)();

	void (*set_debug_enabled)(BOOL enableOrDisable);

    /** Begin exclusive action. */
    BOOL (*begin_exclusive)();

    /** End exclusive action. */
    void (*end_exclusive)();

	/** Get all the Masters in the memory */
	void (*get_current_masters)(w1_master_id * masters, int * pMasterCount);

    /** List all the Masters */
    BOOL (*list_masters)(w1_master_id * masters, int * pMasterCount);

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

}onewire_interface;




struct onewire_device_t {

    /* struct hw_device_t must come first, and named common */
    struct hw_device_t common;

    /* attributes */
    //int fd;

    /* Useful APIs for upper layer: NEW STUB MODEL */
    const onewire_interface* (*get_onewire_interface)(struct onewire_device_t* dev);
};



#ifdef __cplusplus
extern "C" {
#endif

/** ONEWIRE_LEGACY_MODE */
const onewire_interface* hw_get_onewire_interface();

#ifdef __cplusplus
}
#endif

/***************************************************************************/

/* Module ID must be claimed. */
#define ONEWIRE_HARDWARE_MODULE_ID "OneWire"


