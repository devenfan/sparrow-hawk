/*
 *
 * Copyright (c) 2011 Deven Fan <deven@sparrow-hawk.net>
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

/* ====================================================================== */
/* ============================= include ================================ */
/* ====================================================================== */

#include <stdio.h>
#include <stdlib.h>

#include <sys/cdefs.h>
#include <sys/types.h>		//must
#include <sys/socket.h>		//must

#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "w1_netlink_userservice.h"
#include "sh_thread.h"
#include "sh_error.h"

/* ====================================================================== */
/* ============================ Constants =============================== */
/* ====================================================================== */

#define MAX_MSG_SIZE 1024
#define MAX_CNMSG_SIZE 768


/* ====================================================================== */
/* ========================= static variables =========================== */
/* ====================================================================== */


static const int group = W1_GROUP;

static int globalSeq = 1;
static pthread_mutex_t globalSeqMutex;

static int w1Socket;					//SOCKET
static struct sockaddr_nl bindAddr;		//socket bind address
static struct sockaddr_nl dataAddr;		//socket data address

static struct msghdr socketMsgSend;			//socket message header, for sending
static struct msghdr socketMsgRecv;			//socket message header, for receiving
static struct iovec iovSend;				//data storage structure for I/O using uio(Userspace I/O)
static struct iovec iovRecv;				//data storage structure for I/O using uio(Userspace I/O)
static struct nlmsghdr * nlMsgSend = NULL;	//netlink message header, for sending
static struct nlmsghdr * nlMsgRecv = NULL;	//netlink message header, for receiving

//static struct cn_msg * cnmsgBuf = NULL;		//it's a buffer, only for sending purpose

static pthread_t receivingThread;
static int receivingThreadStopFlag = 0;
static sh_signal_ctrl recevingThreadStopSignal;


/* ====================================================================== */
/* ===================== w1 msg message handler ========================= */
/* ====================================================================== */

static void on_w1_netlinkmsg_received(struct cn_msg * cnmsg)
{
	struct w1_netlink_msg * w1msg = (struct w1_netlink_msg *)(cnmsg->data);
	struct w1_netlink_cmd * w1cmd = (struct w1_netlink_cmd *)(w1msg->data);

	u_int64_t * slave_rn;
	int slave_count;
	int index;

	printf("received w1msg type is %s\n", describe_w1_msg_type(w1msg->type));
	printf("received w1cmd type is %s\n", describe_w1_cmd_type(w1cmd->cmd));

	switch(w1cmd->cmd)
	{
		case W1_CMD_ALARM_SEARCH:
		case W1_CMD_SEARCH:

			slave_rn = (u_int64_t *) w1cmd->data;

			slave_count = w1cmd->len / sizeof(u_int64_t);
			index = 0;

			for(index = 0; index < slave_count; index++)
			{
				printf("w1 salve device[%d] found: %lx \n", index, *slave_rn);
				slave_rn++;
			}
			break;
	}

}


/* ====================================================================== */
/* =================== socket msg receiving thread ====================== */
/* ====================================================================== */


static int retrieve_socket_msg(void)
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


static void * socketmsg_receiving_loop(void * param)
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


static void start_receiving_thread(void)
{
	receivingThreadStopFlag = 0;

	sh_signal_init(&recevingThreadStopSignal);

	//Unless we need to use the 4rd argument in the callback, the 4th argument can be NULL
	pthread_create(&receivingThread, NULL, socketmsg_receiving_loop, NULL);

	//printf("Receiving thread created...\n");
}


static void stop_receiving_thread(void)
{
	int ret = 0;

	receivingThreadStopFlag = 1;

	{
		//printf("come into sh_signal_wait...\n");

		ret = sh_signal_wait(&recevingThreadStopSignal);

		//printf("back from sh_signal_wait: %d\n", ret);
	}

	sh_signal_destroy(&recevingThreadStopSignal);

}


/* ====================================================================== */
/* =========================== Public functions ========================= */
/* ====================================================================== */


int w1_netlink_userservice_start(void)
{
    int ret = E_ERROR;

	pthread_mutex_init(&globalSeqMutex, NULL);

	//sh_signal_init(&recevingThreadStopSignal);

	//open socket
	w1Socket = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_CONNECTOR);
	if(-1 == w1Socket)
	{
        perror("socket open");
        return -1;
	}

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
		return -1;
	}

	//init socket messages
	nlMsgSend = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_MSG_SIZE));
	nlMsgRecv = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_MSG_SIZE));
	if (nlMsgSend == NULL || nlMsgRecv == NULL)
	{
		printf("Cannot allocate memory for netlink message header\n");
		return -1;
	}
	memset(nlMsgSend, 0, NLMSG_SPACE(MAX_MSG_SIZE));
	memset(nlMsgRecv, 0, NLMSG_SPACE(MAX_MSG_SIZE));

	/*
	cnmsgBuf = (struct cn_msg *)malloc(MAX_CNMSG_SIZE);
	if(NULL == cnmsgBuf)
	{
		printf("Cannot allocate memory for netlink message \n");
		return -1;
	}
	memset(cnmsgBuf, 0, MAX_CNMSG_SIZE);
	*/

	start_receiving_thread();

	printf("w1 netlink userspace service started!\n");

	return E_OK;
}


int w1_netlink_userservice_stop(void)
{

	stop_receiving_thread();

	//Remove membership when you don't want to use netlink
	setsockopt(w1Socket, SOL_NETLINK, NETLINK_DROP_MEMBERSHIP, &group, sizeof(group));

	//close socket
	close(w1Socket);

	free(nlMsgSend);
	free(nlMsgRecv);
	//free(cnmsgBuf);

	//sh_signal_destroy(&recevingThreadStopSignal);

	pthread_mutex_destroy(&globalSeqMutex);

	printf("w1 netlink userspace service stopped!\n");

	return 0;
}



struct cn_msg * malloc_w1_netlinkmsg(void)
{
	void * buffer = NULL;
	struct cn_msg * cnmsg = NULL;
	//struct w1_netlink_msg * w1msg = NULL;

	buffer = malloc(MAX_CNMSG_SIZE);

	if(NULL == buffer) return NULL;

	cnmsg = (struct cn_msg *) buffer;

	refresh_w1_netlinkmsg(cnmsg);

	//we cannot take "__u8	data[0]" as "void *", because former one don't take any mem.
	//printf("sizeof(struct w1_netlink_msg): %d bytes\n", sizeof(struct w1_netlink_msg));

	return cnmsg;
}


void free_w1_netlinkmsg(struct cn_msg * cnmsg)
{
	/*
	struct w1_netlink_msg * w1msg = (struct w1_netlink_msg *)(cnmsg->data);
	struct w1_netlink_cmd * w1cmd = (struct w1_netlink_cmd *)(w1msg->data);

	free(w1cmd->data);
	free(w1cmd);
	free(w1msg);
	*/

	free(cnmsg);
}

void refresh_w1_netlinkmsg(struct cn_msg * cnmsg)
{
	//struct cb_id w1_id = {.idx = CN_W1_IDX, .val = CN_W1_VAL};

	struct w1_netlink_msg * w1msg;

	memset(cnmsg, 0, MAX_CNMSG_SIZE);

    cnmsg->id.idx = CN_W1_IDX;
    cnmsg->id.val = CN_W1_VAL;

    pthread_mutex_lock(&globalSeqMutex);
    cnmsg->seq = globalSeq++;
    pthread_mutex_unlock(&globalSeqMutex);

	w1msg = (struct w1_netlink_msg *)(cnmsg->data);
	//w1msg->type = w1msgType;
}


int send_w1_netlinkmsg(struct cn_msg * cnmsg)
{
	int ret = -1;

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

	iovSend.iov_base = (void *)nlMsgSend;
	iovSend.iov_len = nlMsgSend->nlmsg_len;

	socketMsgSend.msg_name = (void *)&dataAddr;
	socketMsgSend.msg_namelen = sizeof(dataAddr);
	socketMsgSend.msg_iov = &iovSend;
	socketMsgSend.msg_iovlen = 1;

	//On success, this call return the number of characters sent.
	//On error, -1 is returned, and errno is set appropriately.
	ret = sendmsg(w1Socket, &socketMsgSend, 0);

	if(ret != -1)
	{
		printf("w1 socketmsg sent, which size is " + ret);
	}

	return ret;
}

