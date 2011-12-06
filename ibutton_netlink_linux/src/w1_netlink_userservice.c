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
#include "sh_util.h"
#include "sh_error.h"

/* ====================================================================== */
/* ============================ Constants =============================== */
/* ====================================================================== */

#define MAX_MSG_SIZE    768
#define MAX_CNMSG_SIZE  512


/* ====================================================================== */
/* ========================= static variables =========================== */
/* ====================================================================== */


static const int g_group = W1_GROUP;

static int g_globalSeq = 1;
static pthread_mutex_t g_globalLocker;

static int w1Socket;					//SOCKET
static struct sockaddr_nl g_bindAddr;		//socket bind address
static struct sockaddr_nl g_dataAddr;		//socket data address

static struct msghdr socketMsgSend;			//socket message header, for sending
static struct msghdr socketMsgRecv;			//socket message header, for receiving
static struct iovec iovSend;				//data storage structure for I/O using uio(Userspace I/O)
static struct iovec iovRecv;				//data storage structure for I/O using uio(Userspace I/O)
static struct nlmsghdr * nlMsgSend = NULL;	//netlink message header, for sending
static struct nlmsghdr * nlMsgRecv = NULL;	//netlink message header, for receiving

//static struct cn_msg * cnmsgBuf = NULL;		//it's a buffer, only for sending purpose

static w1_user_callbacks * g_userCallbacks = NULL;

static pthread_t receivingThread;
static int receivingThreadStopFlag = 0;
static sh_signal_ctrl recevingThreadStopSignal;


static BOOL g_isWaitingAckMsg;  //indicate if it's waiting ack from w1 kernel now!
static sh_signal_ctrl g_waitAckMsgSignal;
struct cn_msg * g_ackMsg;       //the ack message

/* ====================================================================== */
/* ========================== private methods =========================== */
/* ====================================================================== */

#define DebugLine(input)   printf(">>>>>>>>>>>>>>>>>>>> %s <<<<<<<<<<<<<<<<<<<<<<<<< \n", (input))


int generate_w1_global_sequence(void);

/**
 * Allocate the fix-size momory to cn_msg.
 * The size is defined inside w1_netlink_userservice.c
 */
struct cn_msg * malloc_w1_netlinkmsg(void);

/**
 * You must free the memory once you finish of using it
 */
void free_w1_netlinkmsg(struct cn_msg * cnmsg);

/**
 * You can re-use the message if you want to save the memory
 */
void refresh_w1_netlinkmsg(struct cn_msg * cnmsg);

/**
 * You can also recycle(free or reuse) it after you send it
 */
BOOL send_w1_netlinkmsg(struct cn_msg * cnmsg);


/* ====================================================================== */
/* ===================== w1 msg message handler ========================= */
/* ====================================================================== */

static void on_w1_netlinkmsg_received(struct cn_msg * cnmsg)
{
	struct w1_netlink_msg * w1msg = (struct w1_netlink_msg *)(cnmsg->data);
	struct w1_netlink_cmd * w1cmd = NULL;

    int master_id;
    int master_count;
    int master_index;
    w1_slave_rn * slave_rn;
    int slave_count;
    int slave_index;

	int idSize = 20;
	char idDescribe[20];
	char msgTypeStr[20];
	char cmdTypeStr[20];

    memset(idDescribe, 0, 20);
    memset(msgTypeStr, 0, 20);
    memset(cmdTypeStr, 0, 20);

    describe_w1_msg_type(w1msg->type, msgTypeStr);

    //Attention: DO NOT mistake any of %d, %s... Or, you will get "Segmentation fault".
	printf("RECV: cnmsg seq[%d], ack[%d], dataLen[%d]\n",
        cnmsg->seq, cnmsg->ack, cnmsg->len);

	printf("RECV: w1msg type[%s], dataLen[%d], status[%d]\n",
        msgTypeStr, w1msg->len, w1msg->status);

    if(W1_SLAVE_ADD == w1msg->type || W1_SLAVE_REMOVE == w1msg->type)
    {
        //Only when Slave Device Found or removed, the slave id(w1msg->id.id) contains 64 bits
        //memcpy(slave_rn, w1msg->id.id, sizeof(w1_slave_rn)); //8 bytes
        slave_rn = w1msg->id.id;

        describe_w1_reg_num(slave_rn, idDescribe);

        if(W1_SLAVE_ADD == w1msg->type)
        {
            printf("w1(1-wire) slave[%s] added\n", idDescribe);
            if(g_userCallbacks != NULL && g_userCallbacks->slave_added_callback != NULL)
                g_userCallbacks->slave_added_callback(*slave_rn);
        }
        else
        {
            printf("w1(1-wire) slave[%s] removed\n", idDescribe);
            if(g_userCallbacks != NULL && g_userCallbacks->slave_removed_callback != NULL)
                g_userCallbacks->slave_removed_callback(*slave_rn);
        }
        return;
    }
    else if(W1_MASTER_ADD == w1msg->type || W1_MASTER_REMOVE == w1msg->type)
    {
        //Only when Master Device Found or Removed, the master id[w1msg->id.mst.id] contains 32 bits
        master_id = w1msg->id.mst.id;
        convert_bytes_to_hexstr(&(w1msg->id.mst.id), 0, sizeof(w1msg->id.mst.id), idDescribe, &idSize);

        if(W1_MASTER_ADD == w1msg->type)
        {
            printf("w1(1-wire) master[%d] added\n", master_id);
            if(g_userCallbacks != NULL && g_userCallbacks->master_added_callback != NULL)
                g_userCallbacks->master_added_callback(master_id);
        }
        else
        {
            printf("w1(1-wire) master[%d] removed\n", master_id);
            if(g_userCallbacks != NULL && g_userCallbacks->master_removed_callback != NULL)
                g_userCallbacks->master_removed_callback(master_id);
        }
        return;
    }
    else if(W1_LIST_MASTERS == w1msg->type)
    {
        //It's the ack of the sending command, it will return the w1msg with master IDs (w1msg->data)
        //It will be processed inside [w1_process_command_root] of w1_netlink.c
        //If w1msg sent back one by one, then the ack will begin with 1, plus 1 by 1, and end with 0
        //Here we consider it will send all IDs back inside one w1msg.
        master_count = w1msg->len / sizeof(master_id);
        for(master_index = 0; master_count < master_count; master_count++)
        {
            master_id = *((int *)(w1msg->data));
            convert_bytes_to_hexstr(&master_id, 0, sizeof(master_id), idDescribe, &idSize);

            printf("w1(1-wire) master[%s] listed\n", idDescribe);
        }
        if(g_userCallbacks != NULL && g_userCallbacks->master_listed_callback != NULL)
                g_userCallbacks->master_listed_callback(w1msg->data, master_count);
        return;
    }
    else if(W1_MASTER_CMD == w1msg->type)
    {
        //It's the ack of the command: SEARCH, ALARM_SEARCH, RESET
        //It will be processed inside w1_netlink.c:
        //    1. w1_search_master_id
        //    2. w1_process_command_master OR w1_process_command_slave
        w1cmd = (struct w1_netlink_cmd *)(w1msg->data);
        describe_w1_cmd_type(w1cmd->cmd, cmdTypeStr);
        printf("RECV: w1cmd type[%s], dataLen[%d]\n", cmdTypeStr,  w1cmd->len);

        if(W1_CMD_SEARCH == w1cmd->cmd || W1_CMD_ALARM_SEARCH == w1cmd->cmd)
        {
            slave_count = w1cmd->len / sizeof(w1_slave_rn);

			slave_rn = (w1_slave_rn *) w1cmd->data;

			for(slave_index = 0; slave_index < slave_count; slave_index++)
			{
			    describe_w1_reg_num(slave_rn + slave_index, idDescribe);
				printf("w1(1-wire) salve[%d] searched: %s \n", slave_index, idDescribe);
			}
        }

        g_ackMsg = cnmsg;

        sh_signal_notify(&g_waitAckMsgSignal);
    }
    else if(W1_SLAVE_CMD == w1msg->type)
    {
        //It's the ack of the command: READ, WRITE, TOUCH
        //It will be processed inside w1_netlink.c:
        //    1. w1_search_slave
        //    2. w1_process_command_master OR w1_process_command_slave
        w1cmd = (struct w1_netlink_cmd *)(w1msg->data);
        describe_w1_cmd_type(w1cmd->cmd, cmdTypeStr);
        printf("RECV: w1cmd type[%s], dataLen[%d]\n", cmdTypeStr,  w1cmd->len);

        g_ackMsg = cnmsg;

        sh_signal_notify(&g_waitAckMsgSignal);
    }

    printf("w1 netlinkmsg process done!\n");

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

	socketMsgRecv.msg_name = (void *)&g_dataAddr;
	socketMsgRecv.msg_namelen = sizeof(g_dataAddr);
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
			printf("RECV: socketmsg received, which size is %d ........................\n", ret);
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

    {
        //Unless we need to use the 4rd argument in the callback, the 4th argument can be NULL
        pthread_create(&receivingThread, NULL, socketmsg_receiving_loop, NULL);
    }

    g_isWaitingAckMsg = FALSE;
	sh_signal_init(&g_waitAckMsgSignal);
}


static void stop_receiving_thread(void)
{
	int ret = 0;

	receivingThreadStopFlag = 1;

	{
		ret = sh_signal_wait(&recevingThreadStopSignal);
	}

	sh_signal_destroy(&recevingThreadStopSignal);

	sh_signal_destroy(&g_waitAckMsgSignal);
}


/* ====================================================================== */
/* =========================== Public functions ========================= */
/* ====================================================================== */


BOOL w1_netlink_userservice_start(w1_user_callbacks * w1UserCallbacks)
{
	pthread_mutex_init(&g_globalLocker, NULL);

	//open socket
	w1Socket = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_CONNECTOR);
	if(-1 == w1Socket)
	{
        perror("socket open");
        return FALSE;
	}

	g_bindAddr.nl_family = AF_NETLINK;
	g_bindAddr.nl_groups = g_group;
	g_bindAddr.nl_pid = getpid();

	g_dataAddr.nl_family = AF_NETLINK;
	g_dataAddr.nl_groups = g_group;
	g_dataAddr.nl_pid = 0;

	//bind socket
	if (bind(w1Socket, (struct sockaddr *)&g_bindAddr, sizeof(struct sockaddr_nl)) == -1)
	{
		perror("socket bind");

		//close socket
		close(w1Socket);
        return FALSE;
	}

	//Add membership to W1 Group. Or, you cannot send any message.
	if (setsockopt(w1Socket, SOL_NETLINK, NETLINK_ADD_MEMBERSHIP, &g_group, sizeof(g_group)) < 0)
	{
		perror("socket setsockopt");
        return FALSE;
	}

	//init socket messages
	nlMsgSend = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_MSG_SIZE));
	nlMsgRecv = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_MSG_SIZE));
	if (nlMsgSend == NULL || nlMsgRecv == NULL)
	{
		printf("Cannot allocate memory for netlink message header\n");
        return FALSE;
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

	g_userCallbacks = w1UserCallbacks;

	start_receiving_thread();

	printf("w1 netlink userspace service started!\n");

	return TRUE;
}


BOOL w1_netlink_userservice_stop(void)
{
    //Attesntion:
    //if the thread is blocked inside [recvmsg], then this method will be blocked here forever!!!
    //TODO: We should send something initiatively, so that we can get ack later...
    request_to_list_w1_masters();

	stop_receiving_thread();

	//Remove membership when you don't want to use netlink
	setsockopt(w1Socket, SOL_NETLINK, NETLINK_DROP_MEMBERSHIP, &g_group, sizeof(g_group));

	//close socket
	close(w1Socket);

	free(nlMsgSend);
	free(nlMsgRecv);
	//free(cnmsgBuf);

	pthread_mutex_destroy(&g_globalLocker);

	printf("w1 netlink userspace service stopped!\n");

	return TRUE;
}



/* ====================================================================== */
/* ========================== private methods =========================== */
/* ====================================================================== */

int generate_w1_global_sequence(void)
{
    int ret = 0;
    pthread_mutex_lock(&g_globalLocker);
    ret = g_globalSeq++;
    pthread_mutex_unlock(&g_globalLocker);
    return ret;
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

	memset(cnmsg, 0, MAX_CNMSG_SIZE);

    cnmsg->id.idx = CN_W1_IDX;
    cnmsg->id.val = CN_W1_VAL;
    cnmsg->seq = generate_w1_global_sequence();
}


BOOL send_w1_netlinkmsg(struct cn_msg * cnmsg)
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

	socketMsgSend.msg_name = (void *)&g_dataAddr;
	socketMsgSend.msg_namelen = sizeof(g_dataAddr);
	socketMsgSend.msg_iov = &iovSend;
	socketMsgSend.msg_iovlen = 1;

	//On success, this call return the number of characters sent.
	//On error, -1 is returned, and errno is set appropriately.
	ret = sendmsg(w1Socket, &socketMsgSend, 0);

	if(ret != -1)
	{
		printf("SEND: w1 socketmsg sent, which size is " + ret);
        return TRUE;
	}

    return FALSE;
}

/* ====================================================================== */
/* =========================== public methods =========================== */
/* ====================================================================== */


BOOL process_w1_master_cmd(int masterId, struct w1_netlink_cmd * cmd)
{
    if(0 == masterId) return FALSE;   //no master id

    if(NULL == cmd) return FALSE;     //no cmd

    BOOL isBusy = FALSE;

    pthread_mutex_lock(&g_globalLocker);
    if(g_isWaitingAckMsg)
        isBusy = TRUE;              //busy now
    else
        g_isWaitingAckMsg = TRUE;   //mark busy
    pthread_mutex_unlock(&g_globalLocker);

    if(isBusy) return FALSE;        //busy

    if(W1_CMD_SEARCH == cmd->cmd || W1_CMD_ALARM_SEARCH == cmd->cmd || W1_CMD_RESET == cmd->cmd)
    {
        BOOL ret = FALSE;
        struct cn_msg * cnmsg = NULL;
        struct w1_netlink_msg * w1msg = NULL;
        struct w1_netlink_cmd * w1cmd = NULL;
        struct w1_netlink_msg * ackW1Msg = NULL;
        struct w1_netlink_cmd * ackW1Cmd = NULL;

        if(g_isWaitingAckMsg) return FALSE;    //busy

        cnmsg = malloc_w1_netlinkmsg();
        w1msg = (struct w1_netlink_msg *) (cnmsg + 1);
        w1cmd = (struct w1_netlink_cmd *) (w1msg + 1);

        memset(w1cmd, cmd, sizeof(struct w1_netlink_cmd) + cmd->len);

        memset(w1msg->id.id, &masterId, sizeof(masterId));
        w1msg->type = W1_MASTER_CMD;
        w1msg->len = sizeof(struct w1_netlink_cmd) + w1cmd->len;

        cnmsg->len = sizeof(struct w1_netlink_msg) + w1msg->len;

        ret = send_w1_netlinkmsg(cnmsg);

        sh_signal_wait(&g_waitAckMsgSignal);

        if(g_ackMsg != NULL)
        {
            ackW1Msg = (struct w1_netlink_msg *) (g_ackMsg + 1);
            ackW1Cmd = (struct w1_netlink_cmd *) (ackW1Msg + 1);

            memset(cmd, ackW1Cmd, sizeof(struct w1_netlink_cmd) + ackW1Cmd->len);

            ret = (0 == ackW1Msg->status) ? TRUE : FALSE;

            g_ackMsg = NULL;
        }

        free_w1_netlinkmsg(cnmsg);

        return ret;
    }

    pthread_mutex_lock(&g_globalLocker);
    g_isWaitingAckMsg = FALSE;   //mark idle
    pthread_mutex_unlock(&g_globalLocker);

    return FALSE;
}


BOOL process_w1_slave_cmd(w1_slave_rn * slaveId, struct w1_netlink_cmd * cmd)
{
    if(NULL == slaveId) return FALSE;   //no slave id

    if(NULL == cmd) return FALSE;     //no cmd

    BOOL isBusy = FALSE;

    pthread_mutex_lock(&g_globalLocker);
    if(g_isWaitingAckMsg)
        isBusy = TRUE;              //busy now
    else
        g_isWaitingAckMsg = TRUE;   //mark busy
    pthread_mutex_unlock(&g_globalLocker);

    if(isBusy) return FALSE;        //busy

    if(W1_CMD_READ == cmd->cmd || W1_CMD_WRITE == cmd->cmd || W1_CMD_TOUCH == cmd->cmd)
    {
        BOOL ret = FALSE;
        struct cn_msg * cnmsg = NULL;
        struct w1_netlink_msg * w1msg = NULL;
        struct w1_netlink_cmd * w1cmd = NULL;
        struct w1_netlink_msg * ackW1Msg = NULL;
        struct w1_netlink_cmd * ackW1Cmd = NULL;

        if(g_isWaitingAckMsg) return FALSE;    //busy

        cnmsg = malloc_w1_netlinkmsg();
        w1msg = (struct w1_netlink_msg *) (cnmsg + 1);
        w1cmd = (struct w1_netlink_cmd *) (w1msg + 1);

        memset(w1cmd, cmd, sizeof(struct w1_netlink_cmd) + cmd->len);

        memset(w1msg->id.id, slaveId, sizeof(w1_slave_rn));
        w1msg->type = W1_SLAVE_CMD;
        w1msg->len = sizeof(struct w1_netlink_cmd) + w1cmd->len;

        cnmsg->len = sizeof(struct w1_netlink_msg) + w1msg->len;

        ret = send_w1_netlinkmsg(cnmsg);

        sh_signal_wait(&g_waitAckMsgSignal);

        if(g_ackMsg != NULL)
        {
            ackW1Msg = (struct w1_netlink_msg *) (g_ackMsg + 1);
            ackW1Cmd = (struct w1_netlink_cmd *) (ackW1Msg + 1);

            memset(cmd, ackW1Cmd, sizeof(struct w1_netlink_cmd) + ackW1Cmd->len);

            ret = (0 == ackW1Msg->status) ? TRUE : FALSE;

            g_ackMsg = NULL;
        }

        free_w1_netlinkmsg(cnmsg);

        return ret;
    }

    pthread_mutex_lock(&g_globalLocker);
    g_isWaitingAckMsg = FALSE;   //mark idle
    pthread_mutex_unlock(&g_globalLocker);

    return FALSE;
}


BOOL request_to_list_w1_masters(void)
{
    BOOL ret = FALSE;
    struct cn_msg * cnmsg = NULL;
    struct w1_netlink_msg * w1msg = NULL;
	//struct w1_netlink_cmd * w1cmd = NULL;

    if(g_isWaitingAckMsg) return FALSE;    //busy

	cnmsg = malloc_w1_netlinkmsg();
	w1msg = (struct w1_netlink_msg *) (cnmsg + 1);
	//w1cmd = (struct w1_netlink_cmd *) (w1msg + 1);

	//w1cmd->len = sizeof(u_int64_t) * slave_count;
	//w1cmd->cmd = W1_CMD_SEARCH;

	//w1msg->len = sizeof(struct w1_netlink_cmd) + w1cmd->len;
	w1msg->len = 0;
	w1msg->type = W1_LIST_MASTERS;

	cnmsg->len = sizeof(struct w1_netlink_msg) + w1msg->len;

	ret = send_w1_netlinkmsg(cnmsg);

	free_w1_netlinkmsg(cnmsg);

	return ret;
}

/*
BOOL request_to_search_w1_slaves(BOOL alarmSearch)
{
    BOOL ret = FALSE;
    struct cn_msg * cnmsg = NULL;
    struct w1_netlink_msg * w1msg = NULL;
	struct w1_netlink_cmd * w1cmd = NULL;

    if(g_isWaitingAckMsg) return FALSE;    //busy

	cnmsg = malloc_w1_netlinkmsg();
	w1msg = (struct w1_netlink_msg *) (cnmsg + 1);
	w1cmd = (struct w1_netlink_cmd *) (w1msg + 1);

	w1cmd->len = 0;
	w1cmd->cmd = alarmSearch ? W1_CMD_ALARM_SEARCH : W1_CMD_SEARCH;

	w1msg->len = sizeof(struct w1_netlink_cmd) + w1cmd->len;
	w1msg->len = 0;
	w1msg->type = W1_MASTER_CMD;
	//TODO: w1-msg->id = master_id...

	cnmsg->len = sizeof(struct w1_netlink_msg) + w1msg->len;

	ret = send_w1_netlinkmsg(cnmsg);

	free_w1_netlinkmsg(cnmsg);

	return ret;
}
*/
