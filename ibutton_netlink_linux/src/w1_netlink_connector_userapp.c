

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

#define MAX_MSG_SIZE 1024
#define MAX_CNMSG_SIZE 512


static int w1Socket;						//SOCKET
struct sockaddr_nl bindAddr, dataAddr;		//socket address

static struct msghdr socketMsgSend;			//socket message header, for sending
static struct msghdr socketMsgRecv;			//socket message header, for receiving
static struct iovec iovSend;				//data storage structure for I/O using uio(Userspace I/O)
static struct iovec iovRecv;				//data storage structure for I/O using uio(Userspace I/O)
static struct nlmsghdr * nlMsgSend = NULL;	//netlink message header, for sending
static struct nlmsghdr * nlMsgRecv = NULL;	//netlink message header, for receiving


static pthread_t receivingThread;
static int receivingThreadStopFlag = 0;
static sh_signal_ctrl recevingThreadStopSignal;


struct cn_msg * malloc_w1_netlinkmsg(void)
{
	void * buffer = NULL;
	struct cn_msg * cnmsg = NULL;
	struct w1_netlink_msg * w1msg = NULL;
	struct w1_netlink_cmd * w1cmd = NULL;
	u_int64_t * slave_rn_fork = NULL;

	buffer = malloc(MAX_CNMSG_SIZE);

	if(NULL == buffer) return NULL;

	cnmsg = (struct cn_msg *) buffer;
	w1msg = (struct w1_netlink_msg *) (cnmsg + 1);
	w1cmd = (struct w1_netlink_cmd *) (w1msg + 1);
	slave_rn_fork = (u_int64_t *) (w1cmd + 1);

	*slave_rn_fork = 0x12345678L;

	w1cmd->len = sizeof(u_int64_t);
	w1cmd->cmd = W1_CMD_SEARCH;

	int len1 = sizeof(w1cmd) + w1cmd->len;
	int len2 = sizeof(struct w1_netlink_cmd) + w1cmd->len;
	if(len1 != len2)
		printf("Not the same length!\n");

	w1msg->len = sizeof(struct w1_netlink_cmd) + w1cmd->len;

	cnmsg->len = sizeof(struct w1_netlink_msg) + w1msg->len;

	return cnmsg;
}


void free_w1_netlinkmsg(struct cn_msg * cnmsg)
{
	struct w1_netlink_msg * w1msg = (struct w1_netlink_msg *)(cnmsg->data);
	struct w1_netlink_cmd * w1cmd = (struct w1_netlink_cmd *)(w1msg->data);

	free(w1cmd->data);
	free(w1cmd);
	free(w1msg);
	free(cnmsg);
}


void init_w1_netlinkmsg(struct cn_msg * cnmsg)
{
	//struct cb_id w1_id = {.idx = CN_W1_IDX, .val = CN_W1_VAL};

    cnmsg->id.idx = CN_W1_IDX;
    cnmsg->id.val = CN_W1_VAL;
    cnmsg->seq = 0;
    cnmsg->ack = 0;
}


int send_w1_netlinkmsg(struct cn_msg * cnmsg)
{
	struct w1_netlink_msg * w1msg = (struct w1_netlink_msg *)(cnmsg + 1);
	struct w1_netlink_cmd * w1cmd = (struct w1_netlink_cmd *)(w1msg + 1);

	int realSize = sizeof(struct nlmsghdr) + sizeof(struct cn_msg)
			+ sizeof(struct w1_netlink_msg) + sizeof(struct w1_netlink_cmd) + w1cmd->len;

	memset(nlMsgSend, 0, sizeof(NLMSG_SPACE(MAX_MSG_SIZE)));
	memset(&iovSend, 0, sizeof(struct iovec));
	memset(&socketMsgSend, 0, sizeof(struct msghdr));

	memcpy(NLMSG_DATA(nlMsgSend), cnmsg, realSize);

	nlMsgSend->nlmsg_len = realSize;
	nlMsgSend->nlmsg_pid = getpid();
	nlMsgSend->nlmsg_flags = 0;
	nlMsgSend->nlmsg_type = NLMSG_DONE;
	nlMsgSend->nlmsg_seq = 0;

//	cnmsg->id.idx = CN_IDX_PROC;
//	cnmsg->id.val = CN_VAL_PROC;
//	cnmsg->seq = 0;
//	cnmsg->ack = 0;
//	cnmsg->len = sizeof(enum proc_cn_mcast_op);

	iovSend.iov_base = (void *)nlMsgSend;
	iovSend.iov_len = nlMsgSend->nlmsg_len;

	socketMsgSend.msg_name = (void *)&dataAddr;
	socketMsgSend.msg_namelen = sizeof(dataAddr);
	socketMsgSend.msg_iov = &iovSend;
	socketMsgSend.msg_iovlen = 1;

	return sendmsg(w1Socket, &socketMsgSend, 0);
}


int send_w1_forkmsg(void)
{
	int ret = 0;
	struct cn_msg * cnmsg = NULL;

	cnmsg = malloc_w1_netlinkmsg();

	if(NULL == cnmsg)
	{
		printf("cannot malloc_w1_netlinkmsg\n");
		return E_OUT_OF_MEM;
	}

	init_w1_netlinkmsg(cnmsg);

	ret = send_w1_netlinkmsg(cnmsg);
	if(-1 == ret)
	{
		perror("send_w1_netlinkmsg");
		return E_SOCKET_CANNOT_SEND;
	}
	else
	{
		printf("send_w1_netlinkmsg ok with %d bytes\n", ret);
		return E_OK;
	}
}


void on_w1_netlinkmsg_received(struct cn_msg * cnmsg)
{
	struct w1_netlink_msg * w1msg = (struct w1_netlink_msg *)(cnmsg->data);
	struct w1_netlink_cmd * w1cmd = (struct w1_netlink_cmd *)(w1msg->data);

	u_int64_t * slave_rn;
	int slave_count;

	printf("w1 netlink cmd: %d\n", w1cmd->cmd);

	switch(w1cmd->cmd)
	{
		case W1_CMD_ALARM_SEARCH:
		case W1_CMD_SEARCH:

			slave_count = w1cmd->len / sizeof(u_int64_t);

			while(slave_count-- > 0)
			{
				slave_rn = (u_int64_t *) w1cmd->data;
				printf("A w1 salve device found: %lx \n", *slave_rn);
				slave_rn++;
			}
			break;
	}

}



int retrieve_socket_msg(void)
{
	memset(nlMsgRecv, 0, NLMSG_SPACE(MAX_MSG_SIZE));
	memset(&iovRecv, 0, sizeof(struct iovec));
	memset(&socketMsgRecv, 0, sizeof(struct msghdr));

	iovRecv.iov_base = (void *)nlMsgRecv;
	iovRecv.iov_len = NLMSG_SPACE(MAX_MSG_SIZE);

	socketMsgRecv.msg_name = (void *)&dataAddr;
	socketMsgRecv.msg_namelen = sizeof(dataAddr);
	socketMsgRecv.msg_iov = &iovRecv;
	socketMsgRecv.msg_iovlen = 1;

	//This call return the number of bytes received, or -1 if an error occurred.
	//The return value will be 0 when the peer has performed an orderly shutdown.

	//If no messages are available at the socket, the receive calls wait for a message to arrive,
	//unless the socket is nonblocking (see fcntl(2)), in which case the value -1 is returned
	//and the external variable errno is set to EAGAIN or EWOULDBLOCK.
	//The receive calls normally return any data available, up to the requested amount,
	//rather than waiting for receipt of the full amount requested.

	int ret = recvmsg(w1Socket, &socketMsgRecv, 0);
	if(0 == ret)
		return E_SOCKET_PEER_GONE;
	else if(-1 == ret)
		return E_SOCKET_CANNOT_RECV;
	else
		return ret;	//return ssize_t
}



void * socketmsg_receiving_loop(void * param)
{
	int ret;

	printf("w1 socketmsg receiving thread started!\n");

	while(!receivingThreadStopFlag)
	{
		ret = retrieve_socket_msg();

		if (E_SOCKET_PEER_GONE == ret) {
			printf("System error, socket peer is gone, application exit.\n");
			exit(0);
		}
		else if (E_SOCKET_CANNOT_RECV == ret) {
			perror("recvmsg error...");
			exit(1);
		}
		else
		{
			printf("w1 socketmsg received, which size is %d\n", ret);
			//base on the ret size...
			on_w1_netlinkmsg_received((struct cn_msg *)NLMSG_DATA(nlMsgRecv));
		}
	}

	printf("w1 socketmsg receiving thread stopped!\n");

	sh_signal_notify(&recevingThreadStopSignal);

	return 0;
}


void start_receiving_thread(void)
{
	receivingThreadStopFlag = 0;

	if(E_OK == send_w1_forkmsg())
	{
		printf("send_w1_forkmsg before thread created...\n");
	}

	//recevingThreadStopSignal = malloc(sizeof(sh_signal_ctrl));

	sh_signal_init(&recevingThreadStopSignal);

	//Unless we need to use the 4rd argument in the callback, the 4th argument can be NULL
	pthread_create(&receivingThread, NULL, socketmsg_receiving_loop, NULL);

	//printf("Receiving thread created...\n");
}


void stop_receiving_thread(void)
{
	int ret = 0;

	receivingThreadStopFlag = 1;

	//if(E_OK == send_w1_forkmsg())
	{
		printf("we're going to sh_signal_wait...\n");

		ret = sh_signal_wait(&recevingThreadStopSignal);

		printf("back from sh_signal_wait: %d\n", ret);
	}

	sh_signal_destroy(&recevingThreadStopSignal);

	//free(recevingThreadStopSignal);
}


int main(void)
{
	const int group = W1_GROUP;

	//open socket
	w1Socket = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_CONNECTOR);

	bindAddr.nl_family = AF_NETLINK;
	bindAddr.nl_groups = group;
	bindAddr.nl_pid = getpid();

	dataAddr.nl_family = AF_NETLINK;
	dataAddr.nl_groups = group;
	dataAddr.nl_pid = 0;

	//bind socket
	if (bind(w1Socket, (struct sockaddr *)&bindAddr, sizeof(struct sockaddr_nl)) == -1)
	{
		perror("socket bind");

		//close socket
		close(w1Socket);
		return -1;
	}

	//Add membership to W1 Group. Or, you cannot send any message.
	if (setsockopt(w1Socket, SOL_NETLINK, NETLINK_ADD_MEMBERSHIP, &group, sizeof(group)) < 0)
	{
		perror("socket setsockopt");
		exit(-1);
	}

	//Remove membership when you don't want to use netlink
	//setsockopt(w1Socket, SOL_NETLINK, NETLINK_DROP_MEMBERSHIP, &group, sizeof(group));


	//init socket messages
	nlMsgSend = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_MSG_SIZE));
	nlMsgRecv = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_MSG_SIZE));
	if (nlMsgSend == NULL || nlMsgRecv == NULL)
	{
		perror("Cannot allocate memory for netlink message header!");
		exit(-1);
	}

	memset(nlMsgSend, 0, NLMSG_SPACE(MAX_MSG_SIZE));
	memset(nlMsgRecv, 0, NLMSG_SPACE(MAX_MSG_SIZE));


	start_receiving_thread();

	sleep(3);

	printf("main thread sleep well...\n");

	stop_receiving_thread();

	//Remove membership when you don't want to use netlink
	setsockopt(w1Socket, SOL_NETLINK, NETLINK_DROP_MEMBERSHIP, &group, sizeof(group));

	//close socket
	close(w1Socket);

	free(nlMsgSend);
	free(nlMsgRecv);

	printf("Game Over...\n");
	return 0;
}


