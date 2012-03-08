#include <stdio.h>
#include <stdlib.h>

#include <sys/cdefs.h>
#include <sys/types.h>		//must
#include <sys/socket.h>		//must

#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "sh_types.h"
#include "sh_error.h"
#include "sh_util.h"
#include "sh_thread.h"

#include "w1_userspace.h"
#include "w1_userservice.h"
#include "w1_netlink_userspace.h"
#include "w1_netlink_userservice.h"

#define MASTER_MAX_COUNT   3
#define SLAVE_MAX_COUNT   10

static w1_master_id m_masterId;    //current master id

static w1_slave_rn m_slaveIDs[SLAVE_MAX_COUNT];
static int m_slaveCount;
//static int m_slaveCurrentIndex;

static w1_user_callbacks m_userCallbacks;

/* ====================================================================== */
/* ============================ log ralated ============================= */
/* ====================================================================== */


//#define ANDROID_NDK

#define LOG_TAG   "w1_netlink_userapp"
#include "sh_log.h"

#define Debug(format, args...)    android_debug(format, ##args)

/* ====================================================================== */
/* ============================== utilities ============================= */
/* ====================================================================== */

static void print_master(void)
{
    Debug("w1(1-wire) Master: %d\n", m_masterId);
}

static void print_all_slaves(void)
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

    if(!is_w1_slave_rn_empty(slave_id))
    {

        for(index = 0; index < m_slaveCount; index++)
        {
            if(are_w1_slave_rn_equal(m_slaveIDs[index], slave_id))
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

    if(!is_w1_slave_rn_empty(slave_id))
    {
        for(index = 0; index < m_slaveCount; index++)
        {
            if(are_w1_slave_rn_equal(m_slaveIDs[index], slave_id))
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
/* ============================ main method ============================= */
/* ====================================================================== */

static void initialize()
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
}





static BOOL Test_ListMasters()
{
    BOOL succeed;

    w1_master_id masters[MASTER_MAX_COUNT];
    int masterCount = 0;

    memset(masters, 0, sizeof(w1_master_id) * MASTER_MAX_COUNT);

    succeed = w1_list_masters(masters, &masterCount);
    if(succeed)
    {
        m_masterId = (masterCount > 0) ? masters[0] : 0;
        Debug("w1_list_masters Succeed!\n");
    }
    else
    {
        m_masterId = 0;
        Debug("w1_list_masters Failed!\n");
    }
    print_master();
    return succeed;
}

static BOOL Test_SearchSlaves()
{
    BOOL succeed;

    w1_slave_rn slaves[SLAVE_MAX_COUNT];
    int slaveCount = 0;
    int index = 0;

    memset(slaves, 0, sizeof(w1_slave_rn) * SLAVE_MAX_COUNT);

    succeed = w1_master_search(m_masterId, FALSE, slaves, &slaveCount);
    if(succeed)
    {
        Debug("w1_master_search Succeed!\n");

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
        Debug("w1_master_search Failed!\n");
    }
    print_all_slaves();
    return succeed;
}

static BOOL Test_ResetMaster()
{
    BOOL succeed;

    succeed = w1_master_reset(m_masterId);

    if(succeed)
    {
        Debug("w1_master_reset Succeed!\n");
    }
    else
    {
        Debug("w1_master_reset Failed!\n");
    }
    return succeed;
}


//It works...
static BOOL Test_1904ReadRTC()
{
    BOOL succeed;

    int dataSendLen1 = 9;
    BYTE dataSend1[9] = { 0x55, 0x24, 0x94, 0x01, 0x37, 0x00, 0x00, 0x00, 0x75};

    int dataSendLen2 = 1;
    BYTE dataSend2[1] = { 0x66};

    int dataSendLen3 = 5;
    BYTE dataSend3[5] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    int dataRecvLen = 0;
    BYTE dataRecv[128];
    memset(dataRecv, 0, sizeof(BYTE) * 128);

    //0. reset
    succeed = w1_master_reset(m_masterId);
    if(!succeed)
    {
        Debug("w1_master_reset Failed!\n");
        return FALSE;
    }

    //1.
    succeed = w1_process_cmd((BYTE *)&m_masterId, sizeof(w1_master_id), W1_CMD_WRITE,
                            dataSend1, dataSendLen1, (void *)dataRecv, &dataRecvLen);
    if(!succeed)
    {
        Debug("w1_process_cmd[W1_CMD_WRITE]-1 Failed!\n");
        return FALSE;
    }

    Debug("w1_process_cmd[W1_CMD_WRITE]-1 Succeed!\n");
    print_bytes(dataRecv, 0, dataRecvLen);

    //2.
    succeed = w1_process_cmd((BYTE *)&m_masterId, sizeof(w1_master_id), W1_CMD_WRITE,
                            dataSend2, dataSendLen2, (void *)dataRecv, &dataRecvLen);
    if(!succeed)
    {
        Debug("w1_process_cmd[W1_CMD_WRITE]-2 Failed!\n");
        return FALSE;
    }

    Debug("w1_process_cmd[W1_CMD_WRITE]-2 Succeed!\n");
    print_bytes(dataRecv, 0, dataRecvLen);

    //3.
    succeed = w1_process_cmd((BYTE *)&m_masterId, sizeof(w1_master_id), W1_CMD_READ,
                        dataSend3, dataSendLen3, (void *)dataRecv, &dataRecvLen);
    if(!succeed)
    {
        Debug("w1_process_cmd[W1_CMD_READ]-3 Failed!\n");
        return FALSE;
    }

    Debug("w1_process_cmd[W1_CMD_READ]-3 Succeed!\n");
    print_bytes(dataRecv, 0, dataRecvLen);

    return TRUE;
}



static BOOL Test_1904WriteRTC()
{
    BOOL succeed;

    int dataSendLen1 = 9;
    BYTE dataSend1[9] = { 0x55, 0x24, 0x94, 0x01, 0x37, 0x00, 0x00, 0x00, 0x75};

    int dataSendLen2 = 6;
    //BYTE dataSend2[6] = { 0x99, 0x5C, 0x33, 0xF1, 0x2C, 0x4F};
    BYTE dataSend2[6] = { 0x99, 0x5C, 0xA0, 0xF2, 0x2D, 0x4F};

    int dataRecvLen = 0;
    BYTE dataRecv[128];
    memset(dataRecv, 0, sizeof(BYTE) * 128);

    //0. reset
    succeed = w1_master_reset(m_masterId);
    if(!succeed)
    {
        Debug("w1_master_reset Failed!\n");
        return FALSE;
    }

    //1.
    succeed = w1_process_cmd((BYTE *)&m_masterId, sizeof(w1_master_id), W1_CMD_WRITE,
                           dataSend1, dataSendLen1, (void *)dataRecv, &dataRecvLen);
    if(!succeed)
    {
        Debug("w1_process_cmd[W1_CMD_WRITE]-1 Failed!\n");
        return FALSE;
    }

    Debug("w1_process_cmd[W1_CMD_WRITE]-1 Succeed!\n");
    print_bytes(dataRecv, 0, dataRecvLen);

    //2.
    succeed = w1_process_cmd((BYTE *)&m_masterId, sizeof(w1_master_id), W1_CMD_TOUCH,
                            dataSend2, dataSendLen2, (void *)dataRecv, &dataRecvLen);
    if(!succeed)
    {
        Debug("w1_process_cmd[W1_CMD_TOUCH]-2 Failed!\n");
        return FALSE;
    }

    Debug("w1_process_cmd[W1_CMD_TOUCH]-2 Succeed!\n");
    print_bytes(dataRecv, 0, dataRecvLen);

    return TRUE;
}


static BOOL Test_1972ReadMemory(int startAddress, int length)
{
    BOOL succeed;

    int dataSendLen1 = 9;
    BYTE dataSend1[9] = { 0x55, 0x2D, 0x04, 0x74, 0xE1, 0x02, 0x00, 0x00, 0xF0};

    int dataSendLen2 = 3 + length;
    BYTE dataSend2[dataSendLen2];
    dataSend2[0] = 0xF0;
    dataSend2[1] = (BYTE)(startAddress & 0x000000FF);
    dataSend2[2] = (BYTE)((startAddress & 0x0000FF00) >> 8);
    memset(dataSend2 + 3, 0xFF, sizeof(BYTE) * length);

    int dataRecvLen = 0;
    BYTE dataRecv[256];
    memset(dataRecv, 0, sizeof(BYTE) * 256);

    //0. reset
    succeed = w1_master_reset(m_masterId);
    if(!succeed)
    {
        Debug("w1_master_reset Failed!\n");
        return FALSE;
    }

    //1. MatchROM
    succeed = w1_process_cmd((BYTE *)&m_masterId, sizeof(w1_master_id), W1_CMD_WRITE,
                           dataSend1, dataSendLen1, (void *)dataRecv, &dataRecvLen);
    if(!succeed)
    {
        Debug("w1_process_cmd[MatchROM] Failed!\n");
        return FALSE;
    }

    Debug("w1_process_cmd[MatchROM] Succeed!\n");
    print_bytes(dataRecv, 0, dataRecvLen);

    //2. ReadMemory
    succeed = w1_process_cmd((BYTE *)&m_masterId, sizeof(w1_master_id), W1_CMD_TOUCH,
                            dataSend2, dataSendLen2, (void *)dataRecv, &dataRecvLen);
    if(!succeed)
    {
        Debug("w1_process_cmd[ReadMemory] Failed!\n");
        return FALSE;
    }

    Debug("w1_process_cmd[ReadMemory] Succeed!\n");
    print_bytes(dataRecv, 0, dataRecvLen);

    return TRUE;
}


static BOOL Test_1972WriteAndCopyScratchpad(int startAddress, BYTE * inputBytes, int offset, int length)
{
    BOOL succeed;

    if(0 != (startAddress % 8) || NULL == inputBytes || 0 != (length % 8))
        return FALSE;

    int dataSendLen1 = 9; //Match ROM
    BYTE dataSend1[9] = { 0x55, 0x2D, 0x04, 0x74, 0xE1, 0x02, 0x00, 0x00, 0xF0};

    int dataSendLen2 = 1 + 2 + 8 + 2;
    BYTE dataSend2[dataSendLen2];
    dataSend2[0] = 0x0F; //Write Scratchpad
    dataSend2[1] = (BYTE)(startAddress & 0x000000FF);
    dataSend2[2] = (BYTE)((startAddress & 0x0000FF00) >> 8);
    memcpy(dataSend2 + 3, inputBytes, 8);
    memset(dataSend2 + 3 + 8, 0xFF, 2); //copy CRC16 out...

    int dataSendLen3 = 3 + 1;
    BYTE dataSend3[dataSendLen3];
    dataSend3[0] = 0x55; //Copy Scratchpad
    dataSend3[1] = (BYTE)(startAddress & 0x000000FF);
    dataSend3[2] = (BYTE)((startAddress & 0x0000FF00) >> 8);
    dataSend3[3] = 0x07; //ES
    //dataSend3[4] = 0xFF; //useless

    int dataSendLen4 = 1 + 3 + 8 + 2;
    BYTE dataSend4[dataSendLen4];
    memset(dataSend4, 0xFF, dataSendLen4);
    dataSend4[0] = 0xAA; //Read Scratchpad

    int dataRecvLen = 0;
    BYTE dataRecv[256];
    memset(dataRecv, 0, sizeof(BYTE) * 256);

    //-----------------------------------------------------------
    //0. Reset
    succeed = w1_master_reset(m_masterId);
    if(!succeed)
    {
        Debug("w1_master_reset Failed!\n");
        return FALSE;
    }
    //1. MatchROM
    succeed = w1_process_cmd((BYTE *)&m_masterId, sizeof(w1_master_id), W1_CMD_TOUCH,
                           dataSend1, dataSendLen1, (void *)dataRecv, &dataRecvLen);
    if(!succeed)
    {
        Debug("w1_process_cmd[MatchROM]-1 Failed!\n");
        return FALSE;
    }
    Debug("w1_process_cmd[MatchROM]-1 Succeed!\n");
    print_bytes(dataRecv, 0, dataRecvLen);
    //2. WriteScratchpad
    succeed = w1_process_cmd((BYTE *)&m_masterId, sizeof(w1_master_id), W1_CMD_TOUCH,
                            dataSend2, dataSendLen2, (void *)dataRecv, &dataRecvLen);
    if(!succeed)
    {
        Debug("w1_process_cmd[WriteScratchpad] Failed!\n");
        return FALSE;
    }
    Debug("w1_process_cmd[WriteScratchpad] Succeed!\n");
    print_bytes(dataRecv, 0, dataRecvLen);

    //-----------------------------------------------------------
    //0. Reset
    succeed = w1_master_reset(m_masterId);
    if(!succeed)
    {
        Debug("w1_master_reset Failed!\n");
        return FALSE;
    }
    //1. MatchROM
    succeed = w1_process_cmd((BYTE *)&m_masterId, sizeof(w1_master_id), W1_CMD_TOUCH,
                           dataSend1, dataSendLen1, (void *)dataRecv, &dataRecvLen);
    if(!succeed)
    {
        Debug("w1_process_cmd[MatchROM]-2 Failed!\n");
        return FALSE;
    }
    Debug("w1_process_cmd[MatchROM]-2 Succeed!\n");
    print_bytes(dataRecv, 0, dataRecvLen);
    //2. ReadScratchpad
    succeed = w1_process_cmd((BYTE *)&m_masterId, sizeof(w1_master_id), W1_CMD_TOUCH,
                            dataSend4, dataSendLen4, (void *)dataRecv, &dataRecvLen);
    if(!succeed)
    {
        Debug("w1_process_cmd[ReadScratchpad]-1 Failed!\n");
        return FALSE;
    }
    Debug("w1_process_cmd[ReadScratchpad]-1 Succeed!\n");
    print_bytes(dataRecv, 0, dataRecvLen);

    //-----------------------------------------------------------
    //0. Reset
    succeed = w1_master_reset(m_masterId);
    if(!succeed)
    {
        Debug("w1_master_reset Failed!\n");
        return FALSE;
    }
    //1. MatchROM
    succeed = w1_process_cmd((BYTE *)&m_masterId, sizeof(w1_master_id), W1_CMD_WRITE,
                           dataSend1, dataSendLen1, (void *)dataRecv, &dataRecvLen);
    if(!succeed)
    {
        Debug("w1_process_cmd[MatchROM]-3 Failed!\n");
        return FALSE;
    }
    Debug("w1_process_cmd[MatchROM]-3 Succeed!\n");
    print_bytes(dataRecv, 0, dataRecvLen);
    //2. CopyScratchpad
    succeed = w1_process_cmd((BYTE *)&m_masterId, sizeof(w1_master_id), W1_CMD_TOUCH,
                            dataSend3, dataSendLen3, (void *)dataRecv, &dataRecvLen);
    if(!succeed)
    {
        Debug("w1_process_cmd[CopyScratchpad] Failed!\n");
        return FALSE;
    }
    Debug("w1_process_cmd[CopyScratchpad] Succeed!\n");
    print_bytes(dataRecv, 0, dataRecvLen);

    //2. Read Result
    //Wait tPROGMAX for the copy function to complete
    sleep(1); //tPROGMAX = 10ms, here we use 1s instead...

    succeed = w1_master_read(m_masterId, 1, dataRecv);
    if(!succeed)
    {
        Debug("w1_master_read Failed!\n");
        return FALSE;
    }
    Debug("w1_master_read Succeed!\n");
    print_bytes(dataRecv, 0, 1);

    return (0xAA == dataRecv[0]) ? TRUE : FALSE;

    //return (0xAA == dataRecv[4]) ? TRUE : FALSE;
}



static BOOL Test_1920Temperature()
{
    BOOL succeed;

    //Match ROM
    int dataSendLen1 = 9;
    BYTE dataSend1[9] = { 0x55, 0x10, 0x55, 0x69, 0x82, 0x00, 0x08, 0x00, 0x74};

    //Convert Temperature
    int dataSendLen2 = 1;
    BYTE dataSend2[1] = { 0x44 };

    //Read Scratchpad
    int dataSendLen3 = 1 + 9;
    BYTE dataSend3[dataSendLen3];
    memset(dataSend3, 0xFF, dataSendLen3);
    dataSend3[0] = 0xBE;

    int dataRecvLen = 0;
    BYTE dataRecv[256];
    memset(dataRecv, 0xFF, 256);

    //-----------------------------------------------------------
    //0. Reset
    succeed = w1_master_reset(m_masterId);
    if(!succeed)
    {
        Debug("w1_master_reset Failed!\n");
        return FALSE;
    }
    //1. MatchROM
    succeed = w1_master_write(m_masterId, dataSendLen1, dataSend1);
    if(!succeed)
    {
        Debug("w1_master_write[MatchROM]-1 Failed!\n");
        return FALSE;
    }
    Debug("w1_master_write[MatchROM]-1 Succeed!\n");
    print_bytes(dataRecv, 0, dataRecvLen);
    //2. ConvertTemperature
    pause_w1_searching_thread();
    succeed = w1_master_write(m_masterId, dataSendLen2, dataSend2);
    if(!succeed)
    {
        Debug("w1_master_write[ConvertTemperature] Failed!\n");
        return FALSE;
    }
    sleep(1); //Data line is held high for at least 0.75 seconds by bus
              //master to allow conversion to complete. Here use 1s instead.
    Debug("w1_master_write[ConvertTemperature] Succeed!\n");
    print_bytes(dataRecv, 0, dataRecvLen);
    wakeup_w1_searching_thread();
    //-----------------------------------------------------------
    //0. Reset
    succeed = w1_master_reset(m_masterId);
    if(!succeed)
    {
        Debug("w1_master_reset Failed!\n");
        return FALSE;
    }
    //1. MatchROM
    succeed = w1_master_write(m_masterId, dataSendLen1, dataSend1);
    if(!succeed)
    {
        Debug("w1_master_write[MatchROM]-2 Failed!\n");
        return FALSE;
    }
    Debug("w1_master_write[MatchROM]-2 Succeed!\n");
    print_bytes(dataRecv, 0, dataRecvLen);
    //2. ReadScratchpad
    succeed = w1_process_cmd((BYTE *)&m_masterId, sizeof(w1_master_id), W1_CMD_TOUCH,
                            dataSend3, dataSendLen3, (void *)dataRecv, &dataRecvLen);
    if(!succeed)
    {
        Debug("w1_process_cmd[ReadScratchpad]-1 Failed!\n");
        return FALSE;
    }
    Debug("w1_process_cmd[ReadScratchpad]-1 Succeed!\n");
    print_bytes(dataRecv, 0, dataRecvLen);

    return succeed;
}




int main(void)
{
	int sleepSecond = 3;

	int i, j;

    char useless[50];

    initialize();

    Debug("======================================================\n");

	if(!w1_netlink_userservice_start(&m_userCallbacks))
	{
	    Debug("Cannot start w1 netlink userspace service...\n");
	    goto GameOver;
	}

    Debug("======================================================\n");

    m_masterId = get_w1_master_id();

    print_master();

    Debug("======================================================\n");

    get_w1_slave_ids(m_slaveIDs, &m_slaveCount);

    print_all_slaves();

    Debug("======================================================\n");

    //Test_ListMasters(); //Must be the first...

    for(i = 0; i < 10; i++)
        Test_ResetMaster();

    Debug("======================================================\n");
    /*
    Test_SearchSlaves();

    sleep(sleepSecond);

	Test_ResetMaster();

    Test_SearchSlaves();

    Debug("======================================================\n");

    sleep(sleepSecond);

    Test_1904ReadRTC();

    Debug("======================================================\n");

    sleep(sleepSecond);

    Test_1904WriteRTC();

    Debug("======================================================\n");

    sleep(sleepSecond);

    Test_1904ReadRTC();
    */

    /*
    Debug("======================================================\n");
    Debug("===============  Test_1972ReadMemory =================\n");
    Debug("======================================================\n");

    Test_1972ReadMemory(0x80, 8);

    sleep(sleepSecond);

    Test_1972ReadMemory(0x20, 8);
    */

    /*
    Debug("======================================================\n");
    Debug("==========  Test_1972WriteAndCopyScratchpad ==========\n");
    Debug("======================================================\n");

    BYTE eightBytes[8] = {0x28, 0x27, 0x26, 0x25, 0x24, 0x23, 0x22, 0x21};

    Test_1972WriteAndCopyScratchpad(0x20, eightBytes, 0, 8);
    */

    /*
    Debug("======================================================\n");
    Debug("===============  Test_1972ReadMemory =================\n");
    Debug("======================================================\n");

    Test_1972ReadMemory(0x20, 8);

    Debug("======================================================\n");
    */

    /**/
    Debug("======================================================\n");
    Debug("===============  Test_1920Temperature ================\n");
    Debug("======================================================\n");

    for(j = 0; j < 10; j++)
        Test_1920Temperature();

    Debug("======================================================\n");

    sleep(sleepSecond);

    Debug("Type something to quit: \n");
    scanf("%s", useless);
    Debug("OK: %s\n", useless);

    w1_netlink_userservice_stop();


GameOver:

	Debug("Main thread Game Over...\n");
	return 0;
}






