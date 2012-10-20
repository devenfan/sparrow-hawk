/*
 *
 * Copyright (c) 2011 Deven Fan <deven.fan@gmail.com>
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */


#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include <string.h>
#include <unistd.h>
#include <pthread.h>


#include "sh_types.h"
#include "sh_error.h"
#include "sh_util.h"
#include "sh_thread.h"

#include "w1_userspace.h"
#include "w1_userservice.h"
#include "w1_sysfs_userservice.h"

/* ====================================================================== */
/* ============================ log ralated ============================= */
/* ====================================================================== */

//#define ANDROID_NDK

#define  LOG_TAG   "w1_sysfs_userservice"
#include "sh_log.h"

#define Debug(format, args...)    android_debug(format, ##args)

/* ====================================================================== */
/* ============================== members =============================== */
/* ====================================================================== */

#define FILE_PATH_DATA_RW           "/sys/bus/w1/devices/w1_master_device/data"
#define FILE_PATH_MASTER_ID         "/sys/bus/w1/devices/w1_master_device/w1_master_id"
#define FILE_PATH_RESET_BUS         "/sys/bus/w1/devices/w1_master_device/w1_master_reset_bus"
#define FILE_PATH_SEARCH_SLAVES     "/sys/bus/w1/devices/w1_master_device/w1_master_search_slaves"
#define FILE_PATH_LIST_SLAVES       "/sys/bus/w1/devices/w1_master_device/w1_master_list_slaves_ids"
#define FILE_PATH_SLAVE_COUNT       "/sys/bus/w1/devices/w1_master_device/w1_master_slave_count"

static pthread_mutex_t g_globalLocker;

static w1_user_callbacks * g_userCallbacks = NULL;

#define MAX_SLAVE_COUNT   10
static w1_master_id g_masterId;    //current master id
static w1_slave_rn g_slavesIDs[MAX_SLAVE_COUNT];
static int g_slavesCount;

static pthread_t        g_w1SearchingThread;
static int              g_w1SearchingThreadStopFlag = 0;
static int              g_w1SearchingThreadPauseFalg = 0;
static sh_signal_ctrl   g_w1SearchingThreadStopSignal;
static int              g_w1SearchingInterval = 1000; //by millisecond


BOOL w1_sysfs_list_masters(w1_master_id * masters, int * pMasterCount);

BOOL w1_sysfs_master_search(w1_master_id masterId, w1_slave_rn * slaves, int * pSlaveCount);

// utilities ======================================================================

static w1_master_id read_master_id()
{
    w1_master_id masterId = 0;
    int fdMasterId = open(FILE_PATH_MASTER_ID, O_RDONLY);

    char dataRead[10];
    memset(dataRead, 0, sizeof(char) * 10);

    if(fdMasterId != -1)
    {
        if(read(fdMasterId, dataRead, 8) > 0)
        {
            Debug("read_master_id: %s\n", dataRead);
            masterId = atoi(dataRead);
        }
    }

    close(fdMasterId);

    return masterId;
}


static BOOL search_slaves()
{
    BOOL succeed = FALSE;
    int fdSearchSlaves = open(FILE_PATH_SEARCH_SLAVES, O_WRONLY);

    char dataWrite[2];
    memset(dataWrite, 0, sizeof(char) * 2);

    if(fdSearchSlaves != -1)
    {
        if(write(fdSearchSlaves, dataWrite, 2) > 0)
        {
            succeed = TRUE;
        }
    }

    close(fdSearchSlaves);

    return succeed;
}


static int read_slave_count()
{
    int slave_count = 0;
    int fdSlaveCount = open(FILE_PATH_SLAVE_COUNT, O_RDONLY);

    char dataRead[4];
    memset(dataRead, 0, sizeof(char) * 4);

    if(fdSlaveCount != -1)
    {
        if(read(fdSlaveCount, dataRead, 2) > 0)
        {
            slave_count = atoi(dataRead);
            Debug("read_slave_count: %d\n", slave_count);
        }
    }

    close(fdSlaveCount);

    return slave_count;
}


static BOOL read_slaves(int slaveCount, w1_slave_rn * slaves)
{
    BOOL succeed = FALSE;
    int fdListSlaves = open(FILE_PATH_LIST_SLAVES, O_RDONLY);
    int index;

    int dataReadLen = 0;
    char dataRead[slaveCount * 19]; //2+1+12+1+2+1, the last char is '\n'
    memset(dataRead, 0, sizeof(char) * slaveCount * 19);

    if(fdListSlaves != -1)
    {
        dataReadLen = read(fdListSlaves, dataRead, slaveCount * 19);

        Debug("read_slaves:\n%s\n", dataRead);

        if(slaveCount * 19 == dataReadLen)
        {
            for(index = 0; index < slaveCount; index++)
            {
                w1_reg_num__from_string((dataRead + index * 19), slaves + index);
            }
            succeed = TRUE;
        }
    }

    close(fdListSlaves);

    return succeed;
}



static BOOL reset_bus()
{
    BOOL succeed = FALSE;
    int fdResetBus = open(FILE_PATH_RESET_BUS, O_WRONLY);

    char dataWrite[2];
    memset(dataWrite, 0, sizeof(char) * 2);

    if(fdResetBus != -1)
    {
        if(write(fdResetBus, dataWrite, 2) > 0)
        {
            succeed = TRUE;
        }
    }

    close(fdResetBus);

    return succeed;
}


static BOOL read_data(int dataReadLen, BYTE * dataReadOut)
{
    BOOL succeed = FALSE;
    int fdDataRW = open(FILE_PATH_DATA_RW, O_RDWR);

    if(fdDataRW != -1)
    {
        if(read(fdDataRW, dataReadOut, dataReadLen) == dataReadLen)
        {
            succeed = TRUE;
        }
    }

    close(fdDataRW);

    return succeed;
}

static BOOL write_data(int dataWriteLen, BYTE * dataWriteIn)
{
    BOOL succeed = FALSE;
    int fdDataRW = open(FILE_PATH_DATA_RW, O_RDWR);

    if(fdDataRW != -1)
    {
        if(write(fdDataRW, dataWriteIn, dataWriteLen) == dataWriteLen)
        {
            succeed = TRUE;
        }
    }

    close(fdDataRW);

    return succeed;
}


// threading ======================================================================


static void w1_compare_slaves(w1_slave_rn * slavesOld, int slavesOldCount,
                              w1_slave_rn * slavesNew, int slavesNewCount,
                              w1_slave_rn * slavesAdded, int * slavesAddedCount,
                              w1_slave_rn * slavesRemoved, int * slavesRemovedCount,
                              w1_slave_rn * slavesKept, int * slavesKeptCount)
{
    //we suspect all input parameters are legal, no NULL input...
    int i, j, added, removed, kept;

    //all empty...
    if((0 == slavesOldCount && 0 == slavesNewCount))
    {
        *slavesAddedCount = 0;
        *slavesRemovedCount = 0;
        return;
    }

    //only old empty
    if(slavesOldCount == 0)
    {
        *slavesAddedCount = slavesNewCount;
        for(i = 0; i < slavesNewCount; i++)
        {
            slavesAdded[i] = slavesNew[i];
        }
        return;
    }

    //only new empty
    if(slavesNewCount == 0)
    {
        *slavesRemovedCount = slavesOldCount;
        for(i = 0; i < slavesOldCount; i++)
        {
            slavesRemoved[i] = slavesOld[i];
        }
        return;
    }

    //compare both
    added = 0;
    removed = 0;
    kept = 0;

    for(i = 0; i < slavesNewCount; i++)
    {
        for(j = 0; j < slavesOldCount; j++)
        {
            if(w1_slave_rn__are_equal(slavesNew[i], slavesOld[j]))
            {
                slavesKept[kept++] = slavesNew[i];
                break;
            }
        }

        if(j == slavesOldCount)
        {
            //not found in slavesOld, means slavesNew[i] is newly added
            slavesAdded[added++] = slavesNew[i];
        }
    }

    if(slavesOldCount > kept)
    {
        for(i = 0; i < slavesOldCount; i++)
        {
            for(j = 0; j < kept; j++)
            {
                if(w1_slave_rn__are_equal(slavesOld[i], slavesKept[j]))
                {
                    break;
                }
            }

            if(j == kept)
            {
                //not found in slavesSame, means slavesOld[i] is removed
                slavesRemoved[removed++] = slavesOld[i];
            }
        }
    }

    *slavesAddedCount = added;
    *slavesRemovedCount = removed;
    *slavesKeptCount = kept;
}


static void * w1_searching_loop(void * param)
{
    w1_master_id masterSearched;

    w1_slave_rn slavesSearched[MAX_SLAVE_COUNT];
    w1_slave_rn slavesAdded[MAX_SLAVE_COUNT];
    w1_slave_rn slavesRemoved[MAX_SLAVE_COUNT];
    w1_slave_rn slavesKept[MAX_SLAVE_COUNT];

    int slavesSearchedCount = 0;
    int slavesAddedCount = 0;
    int slavesRemovedCount = 0;
    int slavesKeptCount = 0;

    int i, j;

    char idString[20];
    memset(idString, 0, 20);

    Debug("w1(1-wire) searching thread started!\n");

    while(!g_w1SearchingThreadStopFlag)
    {
        if(!g_w1SearchingThreadPauseFalg)
        {
            if(g_masterId > 0)
            {
                masterSearched = read_master_id();
                if(masterSearched == 0)
                {
                    Debug("w1(1-wire) master[%d] removed during searching...\n", g_masterId);
                    g_masterId = masterSearched;

                    if(g_userCallbacks != NULL && g_userCallbacks->master_removed_callback != NULL)
                        g_userCallbacks->master_removed_callback(masterSearched);

                    continue;
                }

                slavesSearchedCount = 0;
                slavesAddedCount = 0;
                slavesRemovedCount = 0;
                slavesKeptCount = 0;

                if(w1_sysfs_master_search(g_masterId, slavesSearched, &slavesSearchedCount))
                {
                    pthread_mutex_lock(&g_globalLocker);

                    w1_compare_slaves(g_slavesIDs, g_slavesCount, slavesSearched, slavesSearchedCount,
                                      slavesAdded, &slavesAddedCount, slavesRemoved, &slavesRemovedCount,
                                      slavesKept, &slavesKeptCount);

                    g_slavesCount = slavesKeptCount + slavesAddedCount;
                    memcpy(g_slavesIDs, slavesKept, sizeof(w1_slave_rn) * slavesKeptCount);
                    memcpy(g_slavesIDs + slavesKeptCount, slavesAdded, sizeof(w1_slave_rn) * slavesAddedCount);

                    pthread_mutex_unlock(&g_globalLocker);

                    if(slavesAddedCount > 0)
                    {
                        for(i = 0; i < slavesAddedCount; i++)
                        {
                            w1_reg_num__to_string(slavesAdded + i, idString);
                            Debug("w1(1-wire) slave[%s] added during searching...\n", idString);

                            if(g_userCallbacks != NULL && g_userCallbacks->slave_added_callback != NULL)
                                g_userCallbacks->slave_added_callback(slavesAdded[i]);
                        }
                    }
                    if(slavesRemovedCount > 0)
                    {
                        for(j = 0; j < slavesRemovedCount; j++)
                        {
                            w1_reg_num__to_string(slavesRemoved + j, idString);
                            Debug("w1(1-wire) slave[%s] removed during searching...\n", idString);

                            if(g_userCallbacks != NULL && g_userCallbacks->slave_removed_callback != NULL)
                                g_userCallbacks->slave_removed_callback(slavesRemoved[j]);
                        }
                    }
                }
                else
                {
                    Debug("w1 searching failed on master[%d]...\n", g_masterId);
                }
            }
            else
            {
                masterSearched = read_master_id();
                if(masterSearched > 0)
                {
                    Debug("w1(1-wire) master[%d] added during searching...\n", masterSearched);

                    g_masterId = masterSearched;

                    if(g_userCallbacks != NULL && g_userCallbacks->master_added_callback != NULL)
                        g_userCallbacks->master_added_callback(masterSearched);

                    continue;
                }
            }
        }

        usleep(g_w1SearchingInterval * 1000);   //by microsecond
    }

    Debug("w1(1-wire) searching thread stopped!\n");

    sh_signal_notify(&g_w1SearchingThreadStopSignal);

    return 0;
}


static void w1_searching_loop2(void * param)
{
    w1_searching_loop(param);
}

static void start_searching_thread(void)
{
    g_w1SearchingThreadStopFlag = 0;
    g_w1SearchingThreadPauseFalg = 0;

    sh_signal_init(&g_w1SearchingThreadStopSignal);

    if(NULL == g_userCallbacks->create_thread_cb)
    {
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

        //Unless we need to use the 4rd argument in the callback, the 4th argument can be NULL
        pthread_create(&g_w1SearchingThread, &attr, w1_searching_loop, NULL);
    }
    else
    {
        //g_w1SearchingThread = sh_create_thread("w1_sys_userservice", w1_searching_loop, NULL);
        g_w1SearchingThread = g_userCallbacks->create_thread_cb("w1_sys_searching", w1_searching_loop2, NULL);
    }
}


static void stop_searching_thread(void)
{
    g_w1SearchingThreadStopFlag = 1;
    g_w1SearchingThreadPauseFalg = 1;

    {
        sh_signal_timedwait(&g_w1SearchingThreadStopSignal, 5000);
    }

    sh_signal_destroy(&g_w1SearchingThreadStopSignal);
}


// implement ======================================================================

/**
 * Inject callbacks...
**/
void w1_sysfs_userservice_init(w1_user_callbacks * w1UserCallbacks)
{
    g_userCallbacks = w1UserCallbacks;
}


/**
 * The other functions cannot be used before started.
**/
BOOL w1_sysfs_userservice_start()
{

    //List Masters...
    w1_master_id mastersListed[3];   //usually 1
    int mastersListedCount = 0;

    //Search masters...
    w1_sysfs_list_masters(mastersListed, &mastersListedCount);
    if(mastersListedCount > 0)
    {
        g_masterId = mastersListed[0];

        if(g_masterId > 0)
        {
            //Search Slaves...
            w1_sysfs_master_search(g_masterId, g_slavesIDs, &g_slavesCount);
        }
    }

    start_searching_thread();

    Debug("w1(1-wire) sysfs userspace service started!\n");

    return TRUE;
}

/**
 * Please stop userspace service once you finish of using it.
**/
void w1_sysfs_userservice_stop()
{
    stop_searching_thread();

    Debug("w1(1-wire) sysfs userspace service stopped!\n");
}


/**
 * DONOT invoke this method unless userspace service is started...
**/
static w1_master_id get_current_w1_master()
{
    return g_masterId;  //needs locker???
}

/**
 * DONOT invoke this method unless userspace service is started...
**/
static void get_current_w1_slaves(w1_slave_rn * slaveIDs, int * slaveCount)
{
    if(g_slavesCount > 0)
    {
        *slaveCount = g_slavesCount;
        memcpy(slaveIDs, g_slavesIDs, sizeof(w1_slave_rn) * g_slavesCount);
    }
}


/**
 *
 * Attention: the "masters" & "pMasterCount" are used as output parameters.
**/
BOOL w1_sysfs_list_masters(w1_master_id * masters, int * pMasterCount)
{
    w1_master_id id = read_master_id();
    if(0 == id)
    {
        *pMasterCount = 0;
    }
    else
    {
        *pMasterCount = 1;
        masters[0] = id;
    }
    return TRUE;
}


/**
 * reset this master.
**/
BOOL w1_sysfs_master_reset(w1_master_id masterId)
{
    if(0 == masterId) return FALSE;
    if(g_masterId != masterId) return FALSE;

    return reset_bus();
}

/**
 * search all the slaves on this master.
**/
BOOL w1_sysfs_master_search(w1_master_id masterId, w1_slave_rn * slaves, int * pSlaveCount)
{
    if(0 == masterId) return FALSE;
    if(g_masterId != masterId) return FALSE;
    if(NULL == slaves) return FALSE;
    if(NULL == pSlaveCount) return FALSE;

    int count = 0;

    if(search_slaves())
    {
        count = read_slave_count();
        if(count > 0)
        {
            if(read_slaves(count, slaves))
            {
                *pSlaveCount = count;
                return TRUE;
            }
        }
    }

    return FALSE;
}

/**
 *
 * Input Parameters: masterId, readLen
 * Output Parameters: dataOut
**/
BOOL w1_sysfs_master_read(w1_master_id masterId, int readLen, void * dataOut)
{
    if(0 == masterId) return FALSE;
    if(g_masterId != masterId) return FALSE;

    if(readLen <= 0 || NULL == dataOut) return FALSE;

    return read_data(readLen, dataOut);
}

/**
 *
 * Input Parameters: masterId, writeLen
 * Output Parameters: dataIn
**/
BOOL w1_sysfs_master_write(w1_master_id masterId, int writeLen, void * dataIn)
{
    if(0 == masterId) return FALSE;
    if(g_masterId != masterId) return FALSE;

    if(writeLen <= 0 || NULL == dataIn) return FALSE;

    return write_data(writeLen, dataIn);
}

/**
 *
 * Input Parameters: masterId, dataIn, dataInLen
 * Output Parameters: dataOut, pDataOutLen
**/
BOOL w1_sysfs_master_touch(w1_master_id masterId, void * dataIn, int dataInLen, void * dataOut, int * pDataOutLen)

{
    return FALSE;
}


static BOOL w1_master_begin_exclusive(w1_master_id masterId)
{
    if(0 == masterId) return FALSE;
    if(g_masterId != masterId) return FALSE;

    if(1 == g_w1SearchingThreadPauseFalg)
        return FALSE;

    pthread_mutex_lock(&g_globalLocker);
    g_w1SearchingThreadPauseFalg = 1;   //needs locker???
    pthread_mutex_unlock(&g_globalLocker);

    return TRUE;
}

static void w1_master_end_exclusive(w1_master_id masterId)
{
    pthread_mutex_lock(&g_globalLocker);
    g_w1SearchingThreadPauseFalg = 0;   //needs locker???
    pthread_mutex_unlock(&g_globalLocker);
}



struct w1_user_service w1_sysfs_userservice =
{
    .init = w1_sysfs_userservice_init,
    .start = w1_sysfs_userservice_start,
    .stop = w1_sysfs_userservice_stop,
    //.list_masters = w1_sysfs_list_masters,

    .get_current_master = get_current_w1_master,
    .get_current_slaves = get_current_w1_slaves,
    .begin_exclusive = w1_master_begin_exclusive,
    .end_exclusive = w1_master_end_exclusive,

    .search_slaves = w1_sysfs_master_search,
    .master_reset = w1_sysfs_master_reset,
    .master_read = w1_sysfs_master_read,
    .master_write = w1_sysfs_master_write,
    .master_touch = w1_sysfs_master_touch,
};

