
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



#define MASTER_MAX_COUNT   3
#define SLAVE_MAX_COUNT   10

static w1_master_id m_masterId;    //current master id

static w1_slave_rn m_slaveIDs[SLAVE_MAX_COUNT];
static int m_slaveCount;

static w1_user_callbacks m_userCallbacks;

struct w1_user_service * m_userService;


/* ====================================================================== */
/* ============================ log ralated ============================= */
/* ====================================================================== */

//#define ANDROID_NDK

#define LOG_TAG   "ibutton_linux_app"
#include "sh_log.h"

#define Debug(format, args...)    android_debug(format, ##args)

/* ====================================================================== */
/* ============================== utilities ============================= */
/* ====================================================================== */

void print_master(void)
{
    Debug("w1(1-wire) Master: %d\n", m_masterId);
}

void print_all_slaves(void)
{
    char buf[SLAVE_MAX_COUNT * 30];
    char * position = NULL;
    int index = 0;

    memset(buf, 0, SLAVE_MAX_COUNT * 30);
    for(index = 0, position = buf; index < m_slaveCount; index++)
    {
        sprintf(position, "\tSlave[%d]: %02X.%012llX.%02X\n", index,
                m_slaveIDs[index].family, (long long unsigned int)m_slaveIDs[index].id, m_slaveIDs[index].crc);
        position += strlen(position);
    }
    Debug("Total %d w1(1-wire) Slaves: \n%s\n", m_slaveCount, buf);
}


/* ====================================================================== */
/* =========================== user callbacks =========================== */
/* ====================================================================== */

static void on_master_added(w1_master_id master_id)
{
    Debug("on_master_added\n");

    m_masterId = master_id;

    print_master();
}

static void on_master_removed(w1_master_id master_id)
{
    Debug("on_master_removed\n");

    if(m_masterId == master_id)
        m_masterId = 0;

    print_master();
}


static void on_slave_added(w1_slave_rn slave_id)
{
    int index = 0;
    BOOL found = FALSE;

    Debug("on_slave_added\n");

    if(!w1_slave_rn__is_empty(slave_id))
    {

        for(index = 0; index < m_slaveCount; index++)
        {
            if(w1_slave_rn__are_equal(m_slaveIDs[index], slave_id))
            {
                found = TRUE;
                break;
            }
        }

        if(!found)
        {
            m_slaveIDs[m_slaveCount] = slave_id;
            m_slaveCount++;
            //m_slaveCurrentIndex = 0;
        }
    }

    print_all_slaves();
}

static void on_slave_removed(w1_slave_rn slave_id)
{
    int index = 0;
    BOOL found = FALSE;

    Debug("on_slave_removed\n");

    if(!w1_slave_rn__is_empty(slave_id))
    {
        for(index = 0; index < m_slaveCount; index++)
        {
            if(w1_slave_rn__are_equal(m_slaveIDs[index], slave_id))
            {
                found = TRUE;
                break;
            }
        }

        if(found)
        {
            m_slaveIDs[index] = m_slaveIDs[m_slaveCount - 1];
            //m_slaveIDs[m_slaveCount - 1] = W1_EMPTY_REG_NUM;
            memset(&m_slaveIDs[m_slaveCount - 1], 0, sizeof(w1_slave_rn));
            m_slaveCount--;
            //m_slaveCurrentIndex = (m_slaveCount > 0) ? 0 : -1;
        }
    }

    print_all_slaves();
}

/* ====================================================================== */
/* ============================ Test Method ============================= */
/* ====================================================================== */



BOOL Test_ResetMaster()
{
    BOOL succeed;

    succeed = m_userService->master_reset(m_masterId);

    if(succeed)
    {
        Debug("master_reset Succeed!\n");
    }
    else
    {
        Debug("master_reset Failed!\n");
    }
    return succeed;
}

BOOL Test_ListMasters()
{
    BOOL succeed = FALSE;

    w1_master_id masters[MASTER_MAX_COUNT];
    int masterCount = 0;

    memset(masters, 0, sizeof(w1_master_id) * MASTER_MAX_COUNT);

    succeed = m_userService->list_masters(masters, &masterCount);
    if(succeed)
    {
        m_masterId = (masterCount > 0) ? masters[0] : 0;
        Debug("list_masters Succeed!\n");
    }
    else
    {
        m_masterId = 0;
        Debug("list_masters Failed!\n");
    }
    print_master();

    return succeed;
}


BOOL Test_SearchSlaves()
{
    BOOL succeed;

    w1_slave_rn slaves[SLAVE_MAX_COUNT];
    int slaveCount = 0;
    int index = 0;

    memset(slaves, 0, sizeof(w1_slave_rn) * SLAVE_MAX_COUNT);

    succeed = m_userService->search_slaves(m_masterId, slaves, &slaveCount);
    if(succeed)
    {
        Debug("search_slaves Succeed!\n");

        m_slaveCount = slaveCount;
        //m_slaveCurrentIndex = (slaveCount > 0) ? 0 : -1;

        memset(m_slaveIDs, 0, sizeof(w1_slave_rn) * SLAVE_MAX_COUNT);
        for(index = 0; index < slaveCount; index++)
        {
            m_slaveIDs[index] = slaves[index];
        }

        //memcpy(m_slaveIDs, slaves, sizeof(w1_slave_rn) * m_slaveCount);
    }
    else
    {
        Debug("search_slaves Failed!\n");
    }
    print_all_slaves();
    return succeed;
}




BOOL Test_1904ReadRTC()
{
    BOOL succeed;

    int dataLen1 = 9;
    BYTE data1[9] = { 0x55, 0x24, 0x94, 0x01, 0x37, 0x00, 0x00, 0x00, 0x75};

    int dataLen2 = 1;
    BYTE data2[1] = { 0x66};

    int dataLen3 = 5;
    BYTE data3[5] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};


    //0. reset
    succeed = m_userService->master_reset(m_masterId);
    if(!succeed)
    {
        Debug("[0]master_reset Failed!\n");
        return FALSE;
    }
    Debug("[0]master_reset Succeed!\n");

    //1.
    succeed = m_userService->master_write(m_masterId, dataLen1, data1);
    if(!succeed)
    {
        Debug("[1]master_write Failed!\n");
        return FALSE;
    }
    Debug("[1]master_write Succeed!\n");

    //2.
    succeed = m_userService->master_write(m_masterId, dataLen2, data2);
    if(!succeed)
    {
        Debug("[2]master_write Failed!\n");
        return FALSE;
    }
    Debug("[2]master_write Succeed!\n");

    //3.
    succeed = m_userService->master_read(m_masterId, dataLen3, data3);
    if(!succeed)
    {
        Debug("[3]master_read Failed!\n");
        return FALSE;
    }
    Debug("[3]master_read Succeed!\n");
    print_bytes(data3, 0, dataLen3);

    return TRUE;
}



BOOL Test_1904WriteRTC()
{
    BOOL succeed;

    int dataLen1 = 9;
    BYTE data1[9] = { 0x55, 0x24, 0x94, 0x01, 0x37, 0x00, 0x00, 0x00, 0x75};

    int dataLen2 = 6;
    //BYTE data2[6] = { 0x99, 0x5C, 0x33, 0xF1, 0x2C, 0x4F};
    BYTE data2[6] = { 0x99, 0x5C, 0xA0, 0xF2, 0x2D, 0x4F};

    //0. reset
    succeed = m_userService->master_reset(m_masterId);
    if(!succeed)
    {
        Debug("[0]master_reset Failed!\n");
        return FALSE;
    }
    Debug("[0]master_reset Succeed!\n");

    //1.
    succeed = m_userService->master_write(m_masterId, dataLen1, data1);
    if(!succeed)
    {
        Debug("[1]master_write Failed!\n");
        return FALSE;
    }
    Debug("[1]master_write Succeed!\n");

    //2.
    succeed = m_userService->master_write(m_masterId, dataLen2, data2);
    if(!succeed)
    {
        Debug("[2]master_write Failed!\n");
        return FALSE;
    }
    Debug("[2]master_write Succeed!\n");


    //3. reset
    succeed = m_userService->master_reset(m_masterId);
    if(!succeed)
    {
        Debug("[3]master_reset Failed!\n");
        return FALSE;
    }
    Debug("[3]master_reset Succeed!\n");

    return TRUE;
}




BOOL Test_1920Temperature()
{
    BOOL succeed;

    //Match ROM
    int dataLen1 = 9;
    BYTE data1[9] = { 0x55, 0x10, 0x55, 0x69, 0x82, 0x00, 0x08, 0x00, 0x74};

    //Convert Temperature
    int dataLen2 = 1;
    BYTE data2[1] = { 0x44 };

    //Read Scratchpad
    int dataLen3 = 1;
    BYTE data3[] = { 0xBE };

    //Data Out
    int dataLen4 = 9;
    BYTE data4[dataLen4];
    memset(data4, 0xFF, dataLen4);

    //-----------------------------------------------------------
    //0. MasterReset
    succeed = m_userService->master_reset(m_masterId);
    if(!succeed)
    {
        Debug("[0]MasterReset Failed!\n");
        return FALSE;
    }
    Debug("[0]MasterReset Succeed!\n");

    //1. MatchROM
    succeed = m_userService->master_write(m_masterId, dataLen1, data1);
    if(!succeed)
    {
        Debug("[1]MatchROM Failed!\n");
        return FALSE;
    }
    Debug("[1]MatchROM Succeed!\n");

    //2. ConvertTemperature
    m_userService->master_begin_exclusive(m_masterId);

    succeed = m_userService->master_write(m_masterId, dataLen2, data2);
    if(!succeed)
    {
        m_userService->master_end_exclusive(m_masterId);
        Debug("[2]ConvertTemperature Failed!\n");
        return FALSE;
    }
    sleep(1); //Data line is held high for at least 0.75 seconds by bus
              //master to allow conversion to complete. Here use 1s instead.
    m_userService->master_end_exclusive(m_masterId);
    Debug("[2]ConvertTemperature Succeed!\n");


    //-----------------------------------------------------------
    //0. MasterReset
    succeed = m_userService->master_reset(m_masterId);
    if(!succeed)
    {
        Debug("[0]MasterReset Failed!\n");
        return FALSE;
    }
    Debug("[0]MasterReset Succeed!\n");

    //1. MatchROM
    succeed = m_userService->master_write(m_masterId, dataLen1, data1);
    if(!succeed)
    {
        Debug("[1]MatchROM Failed!\n");
        return FALSE;
    }
    Debug("[1]MatchROM Succeed!\n");

    //2. ReadScratchpad
    succeed = m_userService->master_write(m_masterId, dataLen3, data3);
    if(!succeed)
    {
        Debug("[2]ReadScratchpad Failed!\n");
        return FALSE;
    }
    Debug("[2]ReadScratchpad Succeed!\n");

    //3. DataOut
    succeed = m_userService->master_read(m_masterId, dataLen4, data4);
    if(!succeed)
    {
        Debug("[3]master_read Failed!\n");
        return FALSE;
    }
    Debug("[3]master_read Succeed!\n");
    print_bytes(data4, 0, dataLen4);

    return succeed;
}





BOOL ibutton_test_setup(struct w1_user_service * w1UserService)
{

    /*
    memset( m_masterIDs, 0, sizeof(int) * MASTER_MAX_COUNT );
    m_masterCount = 0;
    m_masterCurrentIndex = -1;
    */

    m_masterId = 0;

    memset(m_slaveIDs, 0, sizeof(w1_slave_rn) * SLAVE_MAX_COUNT );
    m_slaveCount = 0;

    m_userCallbacks.master_added_callback = on_master_added;
    m_userCallbacks.master_removed_callback = on_master_removed;
    m_userCallbacks.slave_added_callback = on_slave_added;
    m_userCallbacks.slave_removed_callback = on_slave_removed;

    m_userService = w1UserService;

    m_userService->init(&m_userCallbacks);

    return m_userService->start();
}


void ibutton_test_teardown()
{
    m_userService->stop();
}


