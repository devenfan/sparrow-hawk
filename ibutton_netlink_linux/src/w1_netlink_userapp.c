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
#include "w1_netlink_userspace.h"
#include "w1_netlink_userservice.h"

#include <android/log.h>    //android log support

#define MASTER_MAX_COUNT   3
#define SLAVE_MAX_COUNT   10


w1_master_id m_masterId;    //current master id

w1_slave_rn m_slaveIDs[SLAVE_MAX_COUNT];
int m_slaveCount;
int m_slaveCurrentIndex;

w1_user_callbacks m_userCallbacks;

#define LOG_TAG   "w1_netlink_userservice"

//logLevel: DEBUG, INFO, WARN, ERROR, FATAL
#define logging(logLevel, format, args...)              \
{                                                       \
    memset(g_logBuf, 0, MAX_LOG_SIZE * sizeof(char));   \
    sprintf(g_logBuf, format, ##args);                  \
    __android_log_write(ANDROID_LOG_##logLevel, LOG_TAG, g_logBuf);   \
}

//print
//#define DebugLine(input)   printf(">>>>>>>>>> w1_netlink_userapp.c : %s  \n", (input))

//logcat
#define DebugLine(input)     __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, (input));

/* ====================================================================== */
/* ============================== utilities ============================= */
/* ====================================================================== */

static void print_master(void)
{
    printf("Master: %d\n", m_masterId);
}

static void print_all_slaves(void)
{
    char buf[SLAVE_MAX_COUNT * 30];
    char * position;
    int index = 0;

    memset(buf, 0, SLAVE_MAX_COUNT * 30);
    for(index = 0, position = buf; index < m_slaveCount; index++)
    {
        sprintf(position, "\tSlave[%d]: %02X.%012llX.%02X\n", index,
                m_slaveIDs[index].family, (long long unsigned int)m_slaveIDs[index].id, m_slaveIDs[index].crc);
        position += strlen(position);
    }
    printf("Total %d Slaves: \n%s\n", m_slaveCount, buf);
}


/* ====================================================================== */
/* =========================== user callbacks =========================== */
/* ====================================================================== */

static void on_master_added(int master_id)
{
    DebugLine("on_master_added");

    m_masterId = master_id;

    print_master();
}

static void on_master_removed(int master_id)
{
    DebugLine("on_master_removed");

    if(m_masterId == master_id)
        m_masterId = 0;

    print_master();
}


static void on_slave_added(w1_slave_rn slave_id)
{
    int index = 0;
    BOOL found = FALSE;

    DebugLine("on_slave_added");

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

    DebugLine("on_slave_removed");

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



int main(void)
{
	int sleepSecond = 3;

    char useless[50];

    w1_master_id masters[MASTER_MAX_COUNT];
    w1_slave_rn slaves[SLAVE_MAX_COUNT];

    int masterCount = 0;
    int slaveCount = 0;
    int index = 0;

    memset(masters, 0, sizeof(w1_master_id) * MASTER_MAX_COUNT);
    memset(slaves, 0, sizeof(w1_slave_rn) * SLAVE_MAX_COUNT);

    BOOL succeed = FALSE;

    initialize();


	if(!w1_netlink_userservice_start(&m_userCallbacks))
	{
	    printf("Cannot start w1 netlink userspace service...\n");
	    goto GameOver;
	}

    //sleep a while...
    sleep(sleepSecond);
    printf("Main thread wake up after %d seconds...\n", sleepSecond);

    succeed = w1_list_masters(masters, &masterCount);
    if(succeed)
    {
        m_masterId = (masterCount > 0) ? masters[0] : 0;
        DebugLine("w1_list_masters Succeed!");
    }
    else
    {
        m_masterId = 0;
        DebugLine("w1_list_masters Failed!");
    }
    print_master();

    //sleep a while...
    sleep(sleepSecond);
    printf("Main thread wake up after %d seconds...\n", sleepSecond);

    succeed = w1_master_search(m_masterId, FALSE, slaves, &slaveCount);
    if(succeed)
    {
        DebugLine("w1_master_search Succeed!");

        m_slaveCount = slaveCount;
        m_slaveCurrentIndex = (slaveCount > 0) ? 0 : -1;
        memcpy(m_slaveIDs, slaves, sizeof(w1_slave_rn) * m_slaveCount);
    }
    else
    {
        DebugLine("w1_master_search Failed!");
    }
    print_all_slaves();

    printf("Type something to quit: \n");
    scanf("%s", useless);
    printf("OK: %s\n", useless);

    w1_netlink_userservice_stop();


GameOver:

	printf("Main thread Game Over...\n");
	return 0;
}




