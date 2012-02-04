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
#include "kernel_connector.h"
#include "w1_netlink_userspace.h"
#include "w1_netlink_util.h"
#include "w1_netlink_userservice.h"


#define MASTER_MAX_COUNT   3
#define SLAVE_MAX_COUNT   10


static w1_master_id m_masterId;    //current master id

static w1_slave_rn m_slaveIDs[SLAVE_MAX_COUNT];
static int m_slaveCount;
static int m_slaveCurrentIndex;

static w1_user_callbacks m_userCallbacks;

/* ====================================================================== */
/* ============================ log ralated ============================= */
/* ====================================================================== */

#define LOG_TAG   "w1_netlink_userapp"

#define ANDROID_NDK

#include "sh_log.h"

#define Debug(format, args...)    android_debug(LOG_TAG, format, ##args)

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
    Debug("on_master_added");

    m_masterId = master_id;

    print_master();
}

static void on_master_removed(w1_master_id master_id)
{
    Debug("on_master_removed");

    if(m_masterId == master_id)
        m_masterId = 0;

    print_master();
}


static void on_slave_added(w1_slave_rn slave_id)
{
    int index = 0;
    BOOL found = FALSE;

    Debug("on_slave_added");

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
            m_slaveCurrentIndex = 0;
        }
    }

    print_all_slaves();
}

static void on_slave_removed(w1_slave_rn slave_id)
{
    int index = 0;
    BOOL found = FALSE;

    Debug("on_slave_removed");

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
            m_slaveCurrentIndex = (m_slaveCount > 0) ? 0 : -1;
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

    memset( m_slaveIDs, 0, sizeof(w1_slave_rn) * SLAVE_MAX_COUNT );
    m_slaveCount = 0;
    m_slaveCurrentIndex = -1;

    m_userCallbacks.master_added_callback = on_master_added;
    m_userCallbacks.master_removed_callback = on_master_removed;
    //m_userCallbacks.master_listed_callback = on_master_listed;
    m_userCallbacks.slave_added_callback = on_slave_added;
    m_userCallbacks.slave_removed_callback = on_slave_removed;
    //m_userCallbacks.slave_found_callback = on_salve_found;
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
        m_slaveCurrentIndex = (slaveCount > 0) ? 0 : -1;

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

/*useless...
static BOOL Test_ReadRom()
{
    BOOL succeed;

    int dataSendLen = 1;
    BYTE dataSend[dataSendLen];

    int dataRecvLen = 0;
    BYTE * dataRecv = NULL;

    int dataReadLen = 8;
    BYTE dataRead[dataReadLen];

    dataSend[0] = 0x33;

    succeed = w1_process_cmd(&m_masterId, sizeof(w1_master_id), W1_CMD_WRITE,
    //succeed = w1_process_cmd(m_slaveIDs + m_slaveCurrentIndex, sizeof(w1_slave_rn), W1_CMD_WRITE,
                            dataSend, dataSendLen, &dataRecv, &dataRecvLen);

    if(succeed)
    {
        Debug("w1_process_cmd[W1_CMD_WRITE] Succeed!\n");

        print_bytes(dataRecv, 0, dataRecvLen);

        memset(dataRead, 0, sizeof(BYTE) * dataReadLen);

        succeed = w1_process_cmd(&m_masterId, sizeof(w1_master_id), W1_CMD_READ,
        //succeed = w1_process_cmd(m_slaveIDs + m_slaveCurrentIndex, sizeof(w1_slave_rn), W1_CMD_READ,
                            dataRead, dataReadLen, &dataRecv, &dataRecvLen);

        if(succeed)
        {
            Debug("w1_process_cmd[W1_CMD_READ] Succeed!\n");

            print_bytes(dataRecv, 0, dataRecvLen);
        }
        else
        {
            Debug("w1_process_cmd[W1_CMD_READ] Failed!\n");
        }
    }
    else
    {
        Debug("w1_process_cmd[W1_CMD_WRITE] Failed!\n");
    }

    return succeed;
}
*/

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


static BOOL Test_1972Read(int startAddress, int length)
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






int main(void)
{
	int sleepSecond = 3;

    char useless[50];

    initialize();

    Debug("======================================================\n");

	if(!w1_netlink_userservice_start(&m_userCallbacks))
	{
	    Debug("Cannot start w1 netlink userspace service...\n");
	    goto GameOver;
	}

    Debug("======================================================\n");

    sleep(sleepSecond);

    Test_ListMasters();

    Debug("======================================================\n");

    sleep(sleepSecond);

	Test_ResetMaster();

    Test_SearchSlaves();

    Debug("======================================================\n");
    /*
    sleep(sleepSecond);

    Test_1904ReadRTC();

    Debug("======================================================\n");

    sleep(sleepSecond);

    Test_1904WriteRTC();

    Debug("======================================================\n");

    sleep(sleepSecond);

    Test_1904ReadRTC();

    Debug("======================================================\n");
    */

    sleep(sleepSecond);

    Test_1972Read(0, 128);

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






