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

#include "sh_types.h"
#include "sh_error.h"
#include "sh_util.h"
#include "sh_thread.h"


#include <linux/netlink.h>		//it will include <linux/socket.h>

//<linux/connector.h> is not contained inside NDK android-5
//Attention: NDK android-5 support android-6 & android 7
#include "kernel_connector.h"

#include "w1_netlink_userspace.h"
#include "w1_netlink_util.h"
#include "w1_netlink_userservice.h"

/* ====================================================================== */
/* ============================ Constants =============================== */
/* ====================================================================== */

#define MAX_MSG_SIZE    256
#define MAX_CNMSG_SIZE  192


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

static BYTE g_currentW1MsgType;
static BYTE g_currentW1CmdType;
static BOOL g_isWaitingAckMsg;             //indicate if it's waiting ack from w1 kernel now
static sh_signal_ctrl g_waitAckMsgSignal;  //the ack signal
static struct cn_msg * g_ackMsg;           //the ack message
//static BYTE g_ackStatus;                   //the ack status: OK, FAILED, TIMEOUT

#define ACK_TIMEOUT    10       //TIMEOUT for waiting ACK, by seconds

/* ====================================================================== */
/* ============================ log ralated ============================= */
/* ====================================================================== */

#define LOG_TAG   "w1_netlink_userservice"

#define ANDROID_NDK

#include "sh_log.h"

#define Debug(format, args...)    android_debug(LOG_TAG, format, ##args)

/* ====================================================================== */
/* ========================== private methods =========================== */
/* ====================================================================== */


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

/*
 * You can re-use the message if you want to save the memory

void refresh_w1_netlinkmsg(struct cn_msg * cnmsg);
 */

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

    w1_master_id master_id;

    w1_slave_rn * slave_rn;

	int idSize = 20;
	char idDescribe[20];
	char msgTypeStr[20];
	char cmdTypeStr[20];

    memset(idDescribe, 0, 20);
    memset(msgTypeStr, 0, 20);
    memset(cmdTypeStr, 0, 20);

    describe_w1_msg_type(w1msg->type, msgTypeStr);

    /*

    //Attention: DO NOT mistake any of %d, %s... Or, you will get "Segmentation fault".
	Debug("RECV: cnmsg seq[%d], ack[%d], dataLen[%d]\n",
        cnmsg->seq, cnmsg->ack, cnmsg->len);

	Debug("RECV: w1msg type[%s], dataLen[%d], status[%d]\n",
        msgTypeStr, w1msg->len, w1msg->status);
    */

    Debug("Print RecvMsg below >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> \n");
    print_cnmsg(cnmsg);
    //print_w1msg(w1msg);

    if(W1_SLAVE_ADD == w1msg->type || W1_SLAVE_REMOVE == w1msg->type)
    {
        //Only when Slave Device Found or removed, the slave id(w1msg->id.id) contains 64 bits
        //memcpy(slave_rn, w1msg->id.id, sizeof(w1_slave_rn)); //8 bytes
        slave_rn = (w1_slave_rn *)w1msg->id.id;

        describe_w1_reg_num(slave_rn, idDescribe);

        if(W1_SLAVE_ADD == w1msg->type)
        {
            Debug("w1(1-wire) slave[%s] added\n", idDescribe);
            if(g_userCallbacks != NULL && g_userCallbacks->slave_added_callback != NULL)
                g_userCallbacks->slave_added_callback(*slave_rn);
        }
        else
        {
            Debug("w1(1-wire) slave[%s] removed\n", idDescribe);
            if(g_userCallbacks != NULL && g_userCallbacks->slave_removed_callback != NULL)
                g_userCallbacks->slave_removed_callback(*slave_rn);
        }
        return;
    }
    else if(W1_MASTER_ADD == w1msg->type || W1_MASTER_REMOVE == w1msg->type)
    {
        //Only when Master Device Found or Removed, the master id[w1msg->id.mst.id] contains 32 bits
        master_id = w1msg->id.mst.id;
        convert_bytes_to_hexstr((BYTE *)&(w1msg->id.mst.id), 0, sizeof(w1msg->id.mst.id), idDescribe, &idSize);

        if(W1_MASTER_ADD == w1msg->type)
        {
            Debug("w1(1-wire) master[%d] added\n", master_id);
            if(g_userCallbacks != NULL && g_userCallbacks->master_added_callback != NULL)
                g_userCallbacks->master_added_callback(master_id);
        }
        else
        {
            Debug("w1(1-wire) master[%d] removed\n", master_id);
            if(g_userCallbacks != NULL && g_userCallbacks->master_removed_callback != NULL)
                g_userCallbacks->master_removed_callback(master_id);
        }
        return;
    }
    else if(W1_LIST_MASTERS == w1msg->type)
    {
        if(g_isWaitingAckMsg)
        {
            memset(g_ackMsg, 0, MAX_CNMSG_SIZE);
            memcpy(g_ackMsg, cnmsg, sizeof(struct cn_msg) + cnmsg->len);
            //Debug("Notify for msgType[%s]!\n", msgTypeStr);
            sh_signal_notify(&g_waitAckMsgSignal);
        }
        return;
    }
    else if(W1_MASTER_CMD == w1msg->type)
    {
        //It's the ack of the command: SEARCH, ALARM_SEARCH, RESET
        //It will be processed inside w1_netlink.c:
        //    1. w1_search_master_id
        //    2. w1_process_command_master OR w1_process_command_slave
        w1cmd = (struct w1_netlink_cmd *)(w1msg->data);

        print_w1cmd(w1cmd);

        /*
        describe_w1_cmd_type(w1cmd->cmd, cmdTypeStr);
        Debug("RECV: w1cmd type[%s], dataLen[%d]\n", cmdTypeStr,  w1cmd->len);

        if(W1_CMD_SEARCH == w1cmd->cmd || W1_CMD_ALARM_SEARCH == w1cmd->cmd)
        {
            slave_count = w1cmd->len / sizeof(w1_slave_rn);

			slave_rn = (w1_slave_rn *) w1cmd->data;

			for(slave_index = 0; slave_index < slave_count; slave_index++)
			{
			    describe_w1_reg_num(slave_rn + slave_index, idDescribe);
				Debug("w1(1-wire) salve[%d] searched: %s \n", slave_index, idDescribe);
			}
        }
        */
        if(g_isWaitingAckMsg)
        {
            memset(g_ackMsg, 0, MAX_CNMSG_SIZE);
            memcpy(g_ackMsg, cnmsg, sizeof(struct cn_msg) + cnmsg->len);
            //Debug("Notify for msgType[%s]!\n", msgTypeStr);
            sh_signal_notify(&g_waitAckMsgSignal);
        }
        return;
    }
    else if(W1_SLAVE_CMD == w1msg->type)
    {
        //It's the ack of the command: READ, WRITE, TOUCH
        //It will be processed inside w1_netlink.c:
        //    1. w1_search_slave
        //    2. w1_process_command_master OR w1_process_command_slave
        w1cmd = (struct w1_netlink_cmd *)(w1msg->data);

        //describe_w1_cmd_type(w1cmd->cmd, cmdTypeStr);
        //Debug("RECV: w1cmd type[%s], dataLen[%d]\n", cmdTypeStr,  w1cmd->len);
        print_w1cmd(w1cmd);

        if(g_isWaitingAckMsg)
        {
            memset(g_ackMsg, 0, MAX_CNMSG_SIZE);
            memcpy(g_ackMsg, cnmsg, sizeof(struct cn_msg) + cnmsg->len);
            //Debug("Notify for msgType[%s]!\n", msgTypeStr);
            sh_signal_notify(&g_waitAckMsgSignal);
        }
        return;
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

	Debug("w1 socketmsg receiving thread started!\n");

	while(!receivingThreadStopFlag)
	{
		ret = retrieve_socket_msg();

		if (E_SOCKET_PEER_GONE == ret) {
			Debug("System error, socket peer is gone, application exit.\n");
			exit(0);
		}
		else if (E_SOCKET_CANNOT_RECV == ret) {
			perror("recvmsg error...");
			exit(1);
		}
		else
		{
			Debug("RECV: socketmsg received, which size is %d ........................\n", ret);
			//base on the ret size...
			on_w1_netlinkmsg_received((struct cn_msg *)NLMSG_DATA(nlMsgRecv));
		}
	}

	Debug("w1 socketmsg receiving thread stopped!\n");

	sh_signal_notify(&recevingThreadStopSignal);

	return 0;
}


static void start_receiving_thread(void)
{
	receivingThreadStopFlag = 0;

	sh_signal_init(&recevingThreadStopSignal);

    {
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

        //Unless we need to use the 4rd argument in the callback, the 4th argument can be NULL
        pthread_create(&receivingThread, &attr, socketmsg_receiving_loop, NULL);
    }

    g_isWaitingAckMsg = FALSE;
	sh_signal_init(&g_waitAckMsgSignal);
}


static void stop_receiving_thread(void)
{
	int ret = 0;

    struct cn_msg * cnmsg = NULL;
    struct w1_netlink_msg * w1msg = NULL;

	cnmsg = malloc_w1_netlinkmsg();
	w1msg = (struct w1_netlink_msg *) (cnmsg + 1);

	w1msg->len = 0;
	w1msg->type = W1_LIST_MASTERS;
	cnmsg->len = sizeof(struct w1_netlink_msg) + w1msg->len;

	receivingThreadStopFlag = 1;

	send_w1_netlinkmsg(cnmsg); //send something to activate the receiving thread
	free_w1_netlinkmsg(cnmsg);

	{
		ret = sh_signal_wait(&recevingThreadStopSignal);
	}

	sh_signal_destroy(&recevingThreadStopSignal);

	sh_signal_destroy(&g_waitAckMsgSignal);
}


static BOOL request_to_list_w1_masters(void)
{
    BOOL ret = FALSE;
    struct cn_msg * cnmsg = NULL;
    struct w1_netlink_msg * w1msg = NULL;

	cnmsg = malloc_w1_netlinkmsg();
	w1msg = (struct w1_netlink_msg *) (cnmsg + 1);

    w1msg->len = 0;
	w1msg->type = W1_LIST_MASTERS;

	cnmsg->len = sizeof(struct w1_netlink_msg) + w1msg->len;

	ret = send_w1_netlinkmsg(cnmsg);

	free_w1_netlinkmsg(cnmsg);

	return ret;
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
		Debug("Cannot allocate memory for netlink message header\n");
        return FALSE;
	}
	memset(nlMsgSend, 0, NLMSG_SPACE(MAX_MSG_SIZE));
	memset(nlMsgRecv, 0, NLMSG_SPACE(MAX_MSG_SIZE));

    //init cnmsg as ack buffer
	g_ackMsg = (struct cn_msg *)malloc(MAX_CNMSG_SIZE);
	if(NULL == g_ackMsg)
	{
		Debug("Cannot allocate memory for ack cnmsg!\n");
		return -1;
	}
	memset(g_ackMsg, 0, MAX_CNMSG_SIZE);


	g_userCallbacks = w1UserCallbacks;

	start_receiving_thread();

	Debug("w1 netlink userspace service started!\n");

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
	free(g_ackMsg);

	pthread_mutex_destroy(&g_globalLocker);

	Debug("w1 netlink userspace service stopped!\n");

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

    //struct cb_id w1_id = {.idx = CN_W1_IDX, .val = CN_W1_VAL};

	memset(cnmsg, 0, MAX_CNMSG_SIZE);

    cnmsg->id.idx = CN_W1_IDX;
    cnmsg->id.val = CN_W1_VAL;
    cnmsg->seq = generate_w1_global_sequence();

	//we cannot take "__u8	data[0]" as "void *", because former one don't take any mem.
	//Debug("sizeof(struct w1_netlink_msg): %d bytes\n", sizeof(struct w1_netlink_msg));

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
		Debug("SEND: w1 socketmsg sent, which size is " + ret);
        return TRUE;
	}

    return FALSE;
}




/* ====================================================================== */
/* ======================== w1 message transact ========================= */
/* ====================================================================== */

BOOL transact_w1_msg(BYTE w1MsgType, BYTE w1CmdType,
                     BYTE * masterOrSlaveId, int idLen,
                     void * data, int dataLen,
                     struct w1_netlink_msg ** ppRecvMsg)
{
    if((idLen < 0) || (idLen > 0 && NULL == masterOrSlaveId)) return FALSE;
    if((dataLen < 0) || (dataLen > 0 && NULL == data))  return FALSE;
    //if(NULL == ppRecvMsg) return FALSE;

    //check busy or not
    BOOL isBusy = FALSE;
    pthread_mutex_lock(&g_globalLocker);
    if(g_isWaitingAckMsg)
        isBusy = TRUE;              //already busy
    else
        g_isWaitingAckMsg = TRUE;   //mark busy
    pthread_mutex_unlock(&g_globalLocker);
    if(isBusy) return FALSE;        //busy

    //declaration
    BOOL succeed = FALSE;
    struct cn_msg * cnmsg = NULL;
    struct w1_netlink_msg * w1msg = NULL;
    struct w1_netlink_cmd * w1cmd = NULL;

    //allocate new message
    cnmsg = malloc_w1_netlinkmsg();
    w1msg = (struct w1_netlink_msg *)(cnmsg + 1);
    w1cmd = (struct w1_netlink_cmd *)(w1msg + 1);

    //assemble w1cmd
    g_currentW1CmdType = w1CmdType;
    w1cmd->cmd = w1CmdType;
    if(dataLen > 0)
    {
        memcpy(w1cmd->data, data, dataLen);
    }
    w1cmd->len = dataLen;

    //assemble w1msg
    g_currentW1MsgType = w1MsgType;
    w1msg->type = w1MsgType;
    if(idLen > 0)
    {
        memcpy(w1msg->id.id, masterOrSlaveId, idLen);
    }
    w1msg->len = sizeof(struct w1_netlink_cmd) + w1cmd->len;

    //assemble cnmsg
	cnmsg->len = sizeof(struct w1_netlink_msg) + w1msg->len;

    Debug("Print OutMsg below >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> \n");
    print_cnmsg(cnmsg);
    print_w1msg(w1msg);
    print_w1cmd(w1cmd);

    //send the message
	succeed = send_w1_netlinkmsg(cnmsg);

	if(!succeed) goto End;

	//Debug("Before sh_signal_wait...\n");

    //waiting for the ack message
    if(sh_signal_wait(&g_waitAckMsgSignal) != 0)
    {
        //Debug("After sh_signal_wait... Failed\n");
        succeed = FALSE;
        goto End;
    }

	//Debug("After sh_signal_wait... OK\n");

    //*ppRecvMsg = (struct w1_netlink_msg *)(g_ackMsg + 1);

    *ppRecvMsg = (struct w1_netlink_msg *)malloc(g_ackMsg->len);
    if(NULL == *ppRecvMsg)
    {
        Debug("Out of memory!\n");
        succeed = FALSE;
        goto End;
    }
    memset(*ppRecvMsg, 0, g_ackMsg->len);
    memcpy(*ppRecvMsg, g_ackMsg->data, g_ackMsg->len);

    Debug("Print AckMsg below >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> \n");
    print_cnmsg(g_ackMsg);
    print_w1msg(*ppRecvMsg);

End:
    g_isWaitingAckMsg = FALSE;
    //free message
    free_w1_netlinkmsg(cnmsg);
    return succeed;
}


/* ====================================================================== */
/* =========================== public methods =========================== */
/* ====================================================================== */



BOOL w1_master_search(w1_master_id masterId, BOOL isSearchAlarm,
                      w1_slave_rn * slaves, int * pSlaveCount)
{
    if(0 == masterId) return FALSE;   //no master id
    if(NULL == slaves) return FALSE;
    if(NULL == pSlaveCount) return FALSE;

    BOOL succeed = FALSE;

    //struct w1_netlink_msg ** ppRecvMsg = malloc(sizeof(struct w1_netlink_msg *));

    struct w1_netlink_msg * w1msgRecv = NULL;
    //Pay attention of all warnings, or you will get unspected result....
    //struct w1_netlink_msg * w1cmdRecv = NULL; //issue here!!!! Wrong declaration!!!!
    struct w1_netlink_cmd * w1cmdRecv = NULL;

	//int index;

    succeed = transact_w1_msg(W1_MASTER_CMD, (isSearchAlarm ? W1_CMD_ALARM_SEARCH : W1_CMD_SEARCH),
                              (BYTE *)&masterId, sizeof(w1_master_id), NULL, 0, &w1msgRecv);

    if(succeed)
        succeed = (0 == w1msgRecv->status) ? TRUE : FALSE;

    if(succeed)
    {
        w1cmdRecv = (struct w1_netlink_cmd *)(w1msgRecv->data);

        //print_bytes((BYTE *)w1cmdRecv, 0, sizeof(w1cmdRecv) + w1cmdRecv->len);

        /*
        Debug("Recv w1msg & w1cmd below................................\n");
        print_w1msg(w1msgRecv);
        print_w1cmd(w1cmdRecv);

        print_bytes(w1cmdRecv, 0, sizeof(struct w1_netlink_cmd) + w1cmdRecv->len);
        print_bytes(w1cmdRecv, 0, sizeof(struct w1_netlink_cmd));
        print_bytes(w1cmdRecv, sizeof(struct w1_netlink_cmd), w1cmdRecv->len);

        print_bytes(bytesOfSlaveRNs, 0, w1cmdRecv->len);

        print_bytes(w1cmdRecv + 1, 0, w1cmdRecv->len);
        print_bytes(w1cmdRecv->data, 0, w1cmdRecv->len);
        print_bytes(w1cmdRecv, 0, sizeof(struct w1_netlink_cmd) + w1cmdRecv->len * 2);
        */

        *pSlaveCount = w1cmdRecv->len / sizeof(w1_slave_rn);

        if(*pSlaveCount > 0)
        {

            memcpy(slaves, w1cmdRecv->data, (*pSlaveCount) * sizeof(w1_slave_rn));

            //print_bytes((BYTE *)slaves, 0, (*pSlaveCount) * sizeof(w1_slave_rn));
        }

    }

    if(!w1msgRecv) free(w1msgRecv);

    return succeed;
}



BOOL w1_master_reset(w1_master_id masterId)
{
    if(0 == masterId) return FALSE;   //no master id

    BOOL succeed = FALSE;

    //struct w1_netlink_msg ** ppRecvMsg = malloc(sizeof(struct w1_netlink_msg *));

    struct w1_netlink_msg * w1msgRecv = NULL;
    //struct w1_netlink_msg * w1cmdRecv = NULL; //issue here!!!! Wrong declaration!!!!

    succeed = transact_w1_msg(W1_MASTER_CMD, W1_CMD_RESET, (BYTE *)&masterId, sizeof(w1_master_id),
                              NULL, 0, &w1msgRecv);

    if(succeed)
        succeed = (0 == w1msgRecv->status) ? TRUE : FALSE;

    if(!w1msgRecv) free(w1msgRecv);

    return succeed;
}


BOOL w1_process_cmd(BYTE * masterOrSlaveId, int idLen, BYTE w1CmdType,
                    void * dataIn, int dataInLen, void * dataOut, int * pDataOutLen)
{
    if(NULL == masterOrSlaveId) return FALSE;
    if(sizeof(w1_master_id) != idLen && sizeof(w1_slave_rn) != idLen) return FALSE;

    if(NULL == dataIn) return FALSE;
    if(NULL == dataOut) return FALSE;
    if(NULL == pDataOutLen) return FALSE;

    if(W1_CMD_READ != w1CmdType &&
       W1_CMD_WRITE != w1CmdType &&
       W1_CMD_TOUCH != w1CmdType) return FALSE;

    BOOL succeed = FALSE;

    struct w1_netlink_msg * w1msgRecv = NULL;
    //struct w1_netlink_msg * w1cmdRecv = NULL; //issue here!!!! Wrong declaration!!!!
    struct w1_netlink_cmd * w1cmdRecv = NULL;

    succeed = transact_w1_msg((sizeof(w1_slave_rn) == idLen) ? W1_SLAVE_CMD : W1_MASTER_CMD,
                              w1CmdType, masterOrSlaveId, idLen,
                              dataIn, dataInLen, &w1msgRecv);

    if(succeed)
        succeed = (0 == w1msgRecv->status) ? TRUE : FALSE;

    if(succeed)
    {
        w1cmdRecv = (struct w1_netlink_cmd *) (w1msgRecv->data);

        print_w1cmd(w1cmdRecv);

        memcpy(dataOut, w1cmdRecv->data, w1cmdRecv->len);
        //*pDataOut = w1cmdRecv->data; //TODO: Copy out???
        *pDataOutLen = w1cmdRecv->len;
    }

    if(!w1msgRecv) free(w1msgRecv);

    return succeed;
}




BOOL w1_list_masters(w1_master_id * masters, int * pMasterCount)
{
    //check input
    if(NULL == masters) return FALSE;
    if(NULL == pMasterCount) return FALSE;

    BOOL succeed = FALSE;

    //struct w1_netlink_msg ** ppRecvMsg = malloc(sizeof(struct w1_netlink_msg *));

    struct w1_netlink_msg * w1msgRecv = NULL;

    succeed = transact_w1_msg(W1_LIST_MASTERS, 0, NULL, 0, NULL, 0, &w1msgRecv);

    if(succeed)
        succeed = (0 == w1msgRecv->status) ? TRUE : FALSE;

    if(succeed)
    {
        //It will return the w1msg with master IDs (w1msg->data)
        //It is processed inside [w1_process_command_root] of w1_netlink.c
        //If w1msg sent back one by one, then the ack will begin with 1, plus 1 by 1, and end with 0
        //Here we consider it will send all IDs back inside one w1msg.
        *pMasterCount = w1msgRecv->len / sizeof(w1_master_id);

        if(*pMasterCount > 0)
        {
            memcpy(masters, w1msgRecv->data, (*pMasterCount) * sizeof(w1_master_id));
        }
    }

    if(!w1msgRecv) free(w1msgRecv);

    return succeed;
}



