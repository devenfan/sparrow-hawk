

#include <stdio.h>
#include <stdlib.h>

#include <sys/cdefs.h>
#include <sys/types.h>		//must
#include <sys/socket.h>		//must

//#include <linux/types.h>

#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "w1_netlink_userspace.h"
#include "sh_error.h"
#include "sh_thread.h"


#include "w1_netlink_userservice.h"

#define MASTER_MAX_COUNT   3
#define SLAVE_MAX_COUNT   10

static int s_masterIDs[MASTER_MAX_COUNT];
static int s_masterCount;
static int s_masterCurrentIndex;

static w1_slave_rn s_slaveIDs[SLAVE_MAX_COUNT];
static int s_slaveCount;
static int s_slaveCurrentIndex;

static w1_user_callbacks s_userCallbacks;


static void on_master_added(int master_id)
{
    int index = 0;
    BOOL found = FALSE;

    if(master_id > 0)
    {
        for(index = 0; index < s_masterCount; index++)
        {
            if(s_masterIDs[index] == master_id)
            {
                found = TRUE;
                break;
            }
        }

        if(!found)
        {
            s_masterIDs[s_masterCount] = master_id;
            s_masterCount++;
            s_masterCurrentIndex = 0;
        }
    }
}

static void on_master_removed(int master_id)
{
    int index = 0;
    BOOL found = FALSE;

    if(master_id > 0)
    {
        for(index = 0; index < s_masterCount; index++)
        {
            if(s_masterIDs[index] == master_id)
            {
                found = TRUE;
                break;
            }
        }

        if(found)
        {
            s_masterIDs[index] = s_masterIDs[s_masterCount - 1];
            s_masterIDs[s_masterCount - 1] = 0;
            s_masterCount--;
            s_masterCurrentIndex = (s_masterCount > 0) ? 0 : -1;
        }
    }
}

static void on_master_listed(int * master_ids, int master_count)
{
    int index = 0;

    memset( s_masterIDs, 0, sizeof(int) * MASTER_MAX_COUNT);
    s_masterCount = master_count;
    s_masterCurrentIndex = (master_count > 0) ? 0 : -1;

    for(index = 0; index < master_count; index++)
    {
        s_masterIDs[index] = *(master_ids + index);
    }
}



static void on_slave_added(w1_slave_rn slave_id)
{
    int index = 0;
    BOOL found = FALSE;

    if(!is_w1_slave_rn_empty(slave_id))
    {
        for(index = 0; index < s_slaveCount; index++)
        {
            if(are_w1_slave_rn_equal(s_slaveIDs[index], slave_id))
            {
                found = TRUE;
                break;
            }
        }

        if(!found)
        {
            s_slaveIDs[s_slaveCount] = slave_id;
            s_slaveCount++;
            s_slaveCurrentIndex = 0;
        }
    }
}

static void on_slave_removed(w1_slave_rn slave_id)
{
    int index = 0;
    BOOL found = FALSE;

    if(!is_w1_slave_rn_empty(slave_id))
    {
        for(index = 0; index < s_slaveCount; index++)
        {
            if(are_w1_slave_rn_equal(s_slaveIDs[index], slave_id))
            {
                found = TRUE;
                break;
            }
        }

        if(found)
        {
            s_slaveIDs[index] = s_slaveIDs[s_slaveCount - 1];
            //s_slaveIDs[s_slaveCount - 1] = W1_EMPTY_REG_NUM;
            memset(&s_slaveIDs[s_slaveCount - 1], 0, sizeof(w1_slave_rn));
            s_slaveCount--;
            s_slaveCurrentIndex = (s_slaveCount > 0) ? 0 : -1;
        }
    }
}

static void on_salve_found(w1_slave_rn * slave_ids, int slave_count)
{
    int index = 0;

    memset( s_slaveIDs, 0, sizeof(w1_slave_rn) * SLAVE_MAX_COUNT);
    s_slaveCount = slave_count;
    s_slaveCurrentIndex = (slave_count > 0) ? 0 : -1;

    for(index = 0; index < slave_count; index++)
    {
        s_slaveIDs[index] = *(slave_ids + index);
    }
}


static void initialize()
{
    memset( s_masterIDs, 0, sizeof(int) * MASTER_MAX_COUNT );
    s_masterCount = 0;
    s_masterCurrentIndex = -1;

    memset( s_masterCurrentIndex, 0, sizeof(w1_slave_rn) * SLAVE_MAX_COUNT );
    s_slaveCount = 0;
    s_slaveCurrentIndex = -1;

    s_userCallbacks.master_added_callback = on_master_added;
    s_userCallbacks.master_removed_callback = on_master_removed;
    s_userCallbacks.master_listed_callback = on_master_listed;
    s_userCallbacks.slave_added_callback = on_slave_added;
    s_userCallbacks.slave_removed_callback = on_slave_removed;
    s_userCallbacks.slave_found_callback = on_salve_found;
}



int main(void)
{
	int sleepSecond = 10;

	char choice;
    int msgType;
    int cmdType;
    char useless[50];

    initialize();

	if(!w1_netlink_userservice_start(&s_userCallbacks))
	{
	    printf("Cannot start w1 netlink userspace service...\n");
	    goto GameOver;
	}

    /*
	printf("Continue(C) or Quit(Q): \n");
    scanf("%c", &choice);

    if('Q' == choice) goto GameOver;

    printf("Please input w1 msg type: \n");
    scanf("%d", &msgType);
    memset(useless, 0, 50);
    describe_w1_msg_type(msgType, useless);
    printf("Your input w1 msg type: %s\n", useless);

    printf("Please input w1 cmd type: \n");
    scanf("%d", &cmdType);
    memset(useless, 0, 50);
    describe_w1_cmd_type(cmdType, useless);
    printf("Your input w1 cmd type: %s\n", useless);


    printf("Type something to quit: \n");
    scanf("%s", useless);
    printf("OK: %s\n", useless);

    */

	/*
	send_w1_forkmsg();
    */

    sleep(sleepSecond);

    printf("Main thread wake up after %d seconds...\n", sleepSecond);

    w1_netlink_userservice_stop();


GameOver:

	printf("Main thread Game Over...\n");
	return 0;
}




