

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


static void init_w1_forkmsg(struct cn_msg * cnmsg)
{
	struct w1_netlink_msg * w1msg = NULL;
	struct w1_netlink_cmd * w1cmd = NULL;
	u_int64_t * slave_rn_fork = NULL;
	u_int64_t temp_rn;
	int slave_count = 10;
	int index = 0;

	w1msg = (struct w1_netlink_msg *) (cnmsg + 1);
	w1cmd = (struct w1_netlink_cmd *) (w1msg + 1);
	slave_rn_fork = (u_int64_t *) (w1cmd + 1);

	for(index = 0; index < slave_count; index++)
	{
		//you must cast to the same type, then calc...
		temp_rn = 0x12345678L + (u_int64_t)index;
		*(slave_rn_fork + index) = temp_rn;
		printf("(index,temp_rn): (%d, %lx)\n", index, temp_rn);
	}

	w1cmd->len = sizeof(u_int64_t) * slave_count;
	w1cmd->cmd = W1_CMD_SEARCH;

	w1msg->len = sizeof(struct w1_netlink_cmd) + w1cmd->len;
	w1msg->type = W1_MASTER_CMD;

	cnmsg->len = sizeof(struct w1_netlink_msg) + w1msg->len;
}




static int send_w1_forkmsg(void)
{
	int ret = 0;
	struct cn_msg * cnmsg = NULL;

	cnmsg = malloc_w1_netlinkmsg();

	if(NULL == cnmsg)
	{
		printf("cannot malloc_w1_netlinkmsg\n");
		return E_OUT_OF_MEM;
	}

	init_w1_forkmsg(cnmsg);

	ret = send_w1_netlinkmsg(cnmsg);

	free_w1_netlinkmsg(cnmsg);

	if(-1 == ret)
	{
		return E_SOCKET_CANNOT_SEND;
	}
	else
	{
		return E_OK;
	}
}


int main(void)
{
	int sleepSecond = 3;

	if(0 == w1_netlink_userservice_start())
    {
        /*send_w1_forkmsg();*/

        sleep(sleepSecond);

        printf("Main thread sleep well after %d seconds...\n", sleepSecond);

        w1_netlink_userservice_stop();
    }
    else
    {
        printf("Cannot start w1 netlink userspace service...\n");
    }

	printf("Main thread Game Over...\n");
	return 0;
}


