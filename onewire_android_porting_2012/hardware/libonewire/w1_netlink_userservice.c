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
#include <errno.h>

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
//#include "kernel_connector.h"
#include "w1_userspace.h"
#include "w1_userservice.h"

#include "w1_netlink_userspace.h"
#include "w1_netlink_userservice.h"


/**
 * Deven # 2012-11-03:
 * 1. To support multi-masters, interface has been changed.
 * 2. Inside w1 searching thread, masters & slaves all should been found.
 *
 * Deven # 2012-12-06:
 * 1. Inside log, "signal during -5160 ms! This command is failed!" keep showing...
 *     I think maybe ACK_TIME is used by other headers.
 * 
 * Deven # 2012-12-07:
 * 1. Change the log header
 * 2. Change the searching loop
 *
 * Deven # 2013-01-03:
 * 1. Issue: "D/w1_netlink_userservice(1852): nsact_w1_msg failed because of busy..." when testing DS1972
 *     (1) WAIT_ACK_TIMEOUT maybe not enough for waiting...
 *     (2) g_isProcessing should be lock when set value
 *     (3) log crazy???
 *     (4) something not being attentioned for DS1972???
 *
*/

/* ====================================================================== */
/* ========================= static variables =========================== */
/* ====================================================================== */

static const int g_group = W1_GROUP;

static int g_globalSeq = 1;
//static pthread_mutex_t g_globalLocker;
static sh_locker_ctrl g_globalLocker;

static int g_w1NetlinkSocket;				//SOCKET
static struct sockaddr_nl g_bindAddr;		//socket bind address
static struct sockaddr_nl g_dataAddr;		//socket data address

#define MAX_MSG_SIZE    256
#define MAX_CNMSG_SIZE  192

static struct msghdr socketMsgSend;			//socket message header, for sending
static struct msghdr socketMsgRecv;			//socket message header, for receiving
static struct iovec iovSend;				//data storage structure for I/O using uio(Userspace I/O)
static struct iovec iovRecv;				//data storage structure for I/O using uio(Userspace I/O)
static struct nlmsghdr * nlMsgSend = NULL;	//netlink message header, for sending
static struct nlmsghdr * nlMsgRecv = NULL;	//netlink message header, for receiving

static w1_user_callbacks * g_userCallbacks = NULL;

static pthread_t        g_socketReceivingThread;
static int              g_socketReceivingThreadStopFlag = 0;
static sh_signal_ctrl   g_socketReceivingThreadStopSignal;

//static struct cn_msg * cnmsgSendBuf = NULL;		//it's a buffer, only for sending purpose
//static struct cn_msg * cnmsgRecvBuf = NULL;     //it's a buffer, only for receiving purpose

static BOOL g_isProcessing;                 //indicate if it's processing now
static sh_signal_ctrl g_waitAckMsgSignal;   //the ack signal
static struct cn_msg * g_ackMsg;            //the ack message
static struct cn_msg * g_outMsg;            //the out message
   

//TIMEOUT for waiting ACK, by milliSeconds...
//Couldn't less than 3000~4000,  otherwise w1_master_search will not work...
static int              WAIT_ACK_TIMEOUT  =  9000;  


#ifndef MAX_MASTER_COUNT
#define MAX_MASTER_COUNT   10
#endif


#ifndef MAX_SLAVE_COUNT
#define MAX_SLAVE_COUNT   100
#endif


//static w1_master_object g_masters[MAX_MASTER_COUNT];
static int              g_mastersCount;
static w1_master_id     g_mastersIDs[MAX_MASTER_COUNT];

static w1_slave_rn      g_slavesIDs[MAX_MASTER_COUNT][MAX_SLAVE_COUNT];
static int              g_slavesCount[MAX_MASTER_COUNT];

static pthread_t        g_w1SearchingThread;
static int              g_w1SearchingThreadStopFlag = 0;
static int              g_w1SearchingThreadPauseFlag = 0;
static sh_signal_ctrl   g_w1SearchingThreadStopSignal;
static int              g_w1SearchingInterval = 1000; 	//by millisecond

static BOOL             g_debug_enabled = TRUE;





/* ----------------------------------------------------------------------- */
/* ------------------------------- log ------------------------------------ */
/* ---------------------------------------------------------------------- */


//#define ANDROID_NDK

#define  LOG_TAG   "w1_netlink_userservice"
#include "sh_log.h"

#define Debug(format, args...)    { 		\
	if(g_debug_enabled) 					\
		android_debug(format, ##args);		\
}

#define Error(format, args...)    android_error(format, ##args)


/* ----------------------------------------------------------------------- */
/* --------------------------- private methods ------------------------------ */
/* ----------------------------------------------------------------------- */


static void lock()
{
	//pthread_mutex_lock(&g_globalLocker);
	sh_locker_lock(&g_globalLocker);
}

static void unlock()
{
	//pthread_mutex_unlock(&g_globalLocker);
	sh_locker_unlock(&g_globalLocker);
}




int generate_w1_global_sequence(void);

/**
 * Allocate the fix-size momory to cn_msg.
 * The size is defined inside w1_netlink_userservice.c

struct cn_msg * malloc_w1_netlinkmsg(void);
 */

/**
 * You must free the memory once you finish of using it

void free_w1_netlinkmsg(struct cn_msg * cnmsg);
 */


/**
 * You can re-use the message if you want to save the memory
 */
void refresh_w1_netlinkmsg(struct cn_msg * cnmsg);


/**
 * You can also recycle(free or reuse) it after you send it
 */
BOOL send_w1_netlinkmsg(struct cn_msg * cnmsg);


/**
 * Synchronized method, cannot use Asynchronized way,
 * because more than 1 ack will be received if succeed.
 * Attention: the "masters" & "pMasterCount" are used as output parameters.
**/
BOOL w1_list_masters(w1_master_id * masters, int * pMasterCount);



/**
 * Synchronized method, search all the slaves on this master.
**/
BOOL w1_master_search(w1_master_id masterId, w1_slave_rn * slaves, int * pSlaveCount);




/* ------------------------------------------------------------------------- */
/* ----------------------- w1 msg message handler -----------------------------*/
/* ------------------------------------------------------------------------- */

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

    //Debug("Print RecvMsg below >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> \n");
    //print_cnmsg(cnmsg);
    //print_w1msg(w1msg);

    // Deven # 2012-11-03:
    // 1. W1_SLAVE_ADD & W1_SLAVE_REMOVE events will not be raised,
    //    because the kernel w1 search thread has not started.
    //    So, the callback will not be invoked.
    // 2. W1_MASTER_ADD & W1_MASTER_REMOVE events will be raised,
    //    but we will list masters inside the searching thread.
    //    So, the callback will not be invoked.

    if(W1_SLAVE_ADD == w1msg->type || W1_SLAVE_REMOVE == w1msg->type)
    {
        //Only when Slave Device Found or removed, the slave id(w1msg->id.id) contains 64 bits
        //memcpy(slave_rn, w1msg->id.id, sizeof(w1_slave_rn)); //8 bytes
        slave_rn = (w1_slave_rn *)w1msg->id.id;

        w1_reg_num__to_string(slave_rn, idDescribe);

        if(W1_SLAVE_ADD == w1msg->type)
        {
            Debug("w1(1-wire) slave[%s] added from kernel... warnning!!!\n", idDescribe);

            //if(g_userCallbacks != NULL && g_userCallbacks->slave_added_callback != NULL)
            //    g_userCallbacks->slave_added_callback(*slave_rn);
        }
        else
        {
            Debug("w1(1-wire) slave[%s] removed from kernel... warnning!!!\n", idDescribe);

            //if(g_userCallbacks != NULL && g_userCallbacks->slave_removed_callback != NULL)
            //    g_userCallbacks->slave_removed_callback(*slave_rn);
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
            Debug("w1(1-wire) master[%d] added from kernel... Good!!!\n", master_id);

            //if(g_userCallbacks != NULL && g_userCallbacks->master_added_callback != NULL)
            //    g_userCallbacks->master_added_callback(master_id);
        }
        else
        {
            Debug("w1(1-wire) master[%d] removed from kernel... Good!!!\n", master_id);

            //if(g_userCallbacks != NULL && g_userCallbacks->master_removed_callback != NULL)
            //    g_userCallbacks->master_removed_callback(master_id);
        }
        return;
    }
    else if(W1_LIST_MASTERS == w1msg->type)
    {
        if(g_isProcessing)
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

        //print_w1cmd(w1cmd);

        if(g_isProcessing)
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
        //print_w1cmd(w1cmd);

        if(g_isProcessing)
        {
            memset(g_ackMsg, 0, MAX_CNMSG_SIZE);
            memcpy(g_ackMsg, cnmsg, sizeof(struct cn_msg) + cnmsg->len);
            //Debug("Notify for msgType[%s]!\n", msgTypeStr);
            sh_signal_notify(&g_waitAckMsgSignal);
        }
        return;
    }

}




/* ---------------------------------------------------------------------------- */
/* ---------------------- socket msg receiving thread ------------------------- */
/* ---------------------------------------------------------------------------- */


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

    int ret = recvmsg(g_w1NetlinkSocket, &socketMsgRecv, 0);
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

    Debug("w1(1-wire) socket receiving thread started!\n");

    while(!g_socketReceivingThreadStopFlag)
    {
        ret = retrieve_socket_msg();

        if (E_SOCKET_PEER_GONE == ret)
        {
            Error("System error, socket peer is gone, application exit.\n");
            exit(0);
        }
        else if (E_SOCKET_CANNOT_RECV == ret)
        {
            perror("recvmsg error...");
        	Error("System error, message cannot be received, application exit.\n");
            exit(1);
        }
        else
        {
            //Debug(">>>>>>>>>> RECV: socketmsg received, which size is %d \n", ret);
            on_w1_netlinkmsg_received((struct cn_msg *)NLMSG_DATA(nlMsgRecv));
        }
    }

    Debug("w1(1-wire) socket receiving thread stopped!\n");

    sh_signal_notify(&g_socketReceivingThreadStopSignal);

    return 0;
}


static void start_receiving_thread(void)
{
    g_socketReceivingThreadStopFlag = 0;

    sh_signal_init(&g_socketReceivingThreadStopSignal);

    {
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

        //Unless we need to use the 4rd argument in the callback, the 4th argument can be NULL
        pthread_create(&g_socketReceivingThread, &attr, socketmsg_receiving_loop, NULL);
    }

    g_isProcessing = FALSE;
    sh_signal_init(&g_waitAckMsgSignal);
}


static void stop_receiving_thread(void)
{
    //int ret = 0;

    struct cn_msg * cnmsg = NULL;
    struct w1_netlink_msg * w1msg = NULL;

    //cnmsg = malloc_w1_netlinkmsg();
    cnmsg = g_outMsg;
    refresh_w1_netlinkmsg(cnmsg);
    w1msg = (struct w1_netlink_msg *) (cnmsg + 1);

    w1msg->len = 0;
    w1msg->type = W1_LIST_MASTERS;
    cnmsg->len = sizeof(struct w1_netlink_msg) + w1msg->len;

    g_socketReceivingThreadStopFlag = 1;

    send_w1_netlinkmsg(cnmsg); //send something to activate the receiving thread
    //free_w1_netlinkmsg(cnmsg);

    {
        //ret = sh_signal_wait(&g_socketReceivingThreadStopSignal);
        sh_signal_timedwait(&g_socketReceivingThreadStopSignal, 5000);
    }

    sh_signal_destroy(&g_socketReceivingThreadStopSignal);

    sh_signal_destroy(&g_waitAckMsgSignal);
}



/* ------------------------------------------------------------------------- */
/* ----------------------- w1 searching thread ----------------------------- */
/* ------------------------------------------------------- ----------------- */

static int find_master_index(w1_master_id master, w1_master_id * mastersList, int mastersCount)
{
	int index = -1;
	int i = 0;
	for(i = 0; i < mastersCount; i++)
	{
		if(mastersList[i] == master)
		{
			index = i;
			break;
		}
	}
	return index;
}


static void w1_compare_masters(w1_master_id * mastersOld, int mastersOldCount,
                               w1_master_id * mastersNew, int mastersNewCount,
                               w1_master_id * mastersAdded, int * mastersAddedCount,
                               w1_master_id * mastersRemoved, int * mastersRemovedCount,
                               w1_master_id * mastersKept, int * mastersKeptCount)
{
    //we suspect all input parameters are legal, no NULL input...
    int i, j, added, removed, kept;

    //all empty...
    if((0 == mastersOldCount && 0 == mastersNewCount))
    {
        *mastersAddedCount = 0;
        *mastersRemovedCount = 0;
        return;
    }

    //only old empty, consider the new ones are added
    if(mastersOldCount == 0)
    {
        *mastersAddedCount = mastersNewCount;
        for(i = 0; i < mastersNewCount; i++)
        {
            mastersAdded[i] = mastersNew[i];
        }
        return;
    }

    //only new empty, consider the new ones are removed
    if(mastersNewCount == 0)
    {
        *mastersRemovedCount = mastersOldCount;
        for(i = 0; i < mastersOldCount; i++)
        {
            mastersRemoved[i] = mastersOld[i];
        }
        return;
    }

    //compare both
    added = 0;
    removed = 0;
    kept = 0;

    for(i = 0; i < mastersNewCount; i++)
    {
        for(j = 0; j < mastersOldCount; j++)
        {
            if(mastersNew[i] == mastersOld[j])
            {
                mastersKept[kept++] = mastersNew[i];
                break;
            }
        }

        if(j == mastersOldCount)
        {
            //not found in mastersOld, means mastersNew[i] is newly added
            mastersAdded[added++] = mastersNew[i];
        }
    }

    if(mastersOldCount > kept)
    {
        for(i = 0; i < mastersOldCount; i++)
        {
            for(j = 0; j < kept; j++)
            {
                if(mastersOld[i] == mastersKept[j])
                {
                    break;
                }
            }

            if(j == kept)
            {
                //not found in mastersKept, means mastersOld[i] is removed
                mastersRemoved[removed++] = mastersOld[i];
            }
        }
    }

    *mastersAddedCount = added;
    *mastersRemovedCount = removed;
    *mastersKeptCount = kept;
}



static void w1_compare_slaves(w1_slave_rn * slavesOld, int slavesOldCount,
                              w1_slave_rn * slavesNew, int slavesNewCount,
                              w1_slave_rn * slavesAdded, int * slavesAddedCount,
                              w1_slave_rn * slavesRemoved, int * slavesRemovedCount,
                              w1_slave_rn * slavesKept, int * slavesKeptCount)
{
    //we suspect all input parameters are legal, no NULL input...
    int i, j, added, removed, kept;

    //all empty...
    if((0 == slavesOldCount && 0 == slavesNewCount))
    {
        *slavesAddedCount = 0;
        *slavesRemovedCount = 0;
		*slavesKeptCount = 0;
        return;
    }

    //only old empty
    if(slavesOldCount == 0)
    {
        *slavesAddedCount = slavesNewCount;
        for(i = 0; i < slavesNewCount; i++)
        {
            slavesAdded[i] = slavesNew[i];
        }
		*slavesKeptCount = 0;
		*slavesRemovedCount = 0;
        return;
    }

    //only new empty
    if(slavesNewCount == 0)
    {
        *slavesRemovedCount = slavesOldCount;
        for(i = 0; i < slavesOldCount; i++)
        {
            slavesRemoved[i] = slavesOld[i];
        }
		*slavesAddedCount = 0;
		*slavesKeptCount = 0;
        return;
    }

    //compare both
    added = 0;
    removed = 0;
    kept = 0;

    for(i = 0; i < slavesNewCount; i++)
    {
        for(j = 0; j < slavesOldCount; j++)
        {
            if(w1_slave_rn__are_equal(slavesNew[i], slavesOld[j]))
            {
                slavesKept[kept++] = slavesNew[i];
                break;
            }
        }

        if(j == slavesOldCount)
        {
            //not found in slavesOld, means slavesNew[i] is newly added
            slavesAdded[added++] = slavesNew[i];
        }
    }

	//Now added & kept already calculated...

    if(slavesOldCount > kept)
    {
        for(i = 0; i < slavesOldCount; i++)
        {
            for(j = 0; j < kept; j++)
            {
                if(w1_slave_rn__are_equal(slavesOld[i], slavesKept[j]))
                {
                    break;
                }
            }

            if(j == kept)
            {
                //not found in slavesKept, means slavesOld[i] is removed
                slavesRemoved[removed++] = slavesOld[i];
            }
        }
    }

    *slavesAddedCount = added;
    *slavesRemovedCount = removed;
    *slavesKeptCount = kept;
}







static void * w1_searching_loop(void * param)
{

	int          newMastersCount = 0;
	w1_master_id newMastersIDs[MAX_MASTER_COUNT];
	w1_slave_rn  newSlavesIDs[MAX_MASTER_COUNT][MAX_SLAVE_COUNT];
	int          newSlavesCount[MAX_MASTER_COUNT];

    w1_master_id mastersSearched[MAX_MASTER_COUNT];
    w1_master_id mastersAdded[MAX_MASTER_COUNT];
    w1_master_id mastersRemoved[MAX_MASTER_COUNT];
    w1_master_id mastersKept[MAX_MASTER_COUNT];

    int mastersSearchedCount = 0;
    int mastersAddedCount = 0;
    int mastersRemovedCount = 0;
    int mastersKeptCount = 0;

    w1_master_id currentMaster;

    w1_slave_rn slavesSearched[MAX_SLAVE_COUNT];
    w1_slave_rn slavesAdded[MAX_SLAVE_COUNT];
    w1_slave_rn slavesRemoved[MAX_SLAVE_COUNT];
    w1_slave_rn slavesKept[MAX_SLAVE_COUNT];

    int slavesSearchedCount = 0;
    int slavesAddedCount = 0;
    int slavesRemovedCount = 0;
    int slavesKeptCount = 0;

	BOOL hasChanged = FALSE;
	
    int i, j, k;
	int index, index2;

    char idString[20];
    memset(idString, 0, 20);

    Debug("w1(1-wire) searching thread started!\n");

    while(!g_w1SearchingThreadStopFlag)
    {

		//pthread_mutex_lock(&g_globalLocker);
	
        if(!g_w1SearchingThreadPauseFlag)
        {
        
            mastersSearchedCount = 0;
            mastersAddedCount = 0;
            mastersRemovedCount = 0;
            mastersKeptCount = 0;

			slavesSearchedCount = 0;
    		slavesAddedCount = 0;
    		slavesRemovedCount = 0;
    		slavesKeptCount = 0;
			
			hasChanged = FALSE;

            if(w1_list_masters(mastersSearched, &mastersSearchedCount))
            {
			
            	//Debug("%d w1 masters listed during the searching!\n", mastersSearchedCount);

                w1_compare_masters(g_mastersIDs, g_mastersCount, mastersSearched, mastersSearchedCount,
                                  mastersAdded, &mastersAddedCount, mastersRemoved, &mastersRemovedCount,
                                  mastersKept, &mastersKeptCount);


                newMastersCount = mastersKeptCount + mastersAddedCount;
                memcpy(newMastersIDs, mastersKept, sizeof(w1_master_id) * mastersKeptCount);
                memcpy(newMastersIDs + mastersKeptCount, mastersAdded, sizeof(w1_master_id) * mastersAddedCount);
				

				//process removed masters...
				if(mastersRemovedCount > 0)
                {

					hasChanged = TRUE;
			
                    for(i = 0; i < mastersRemovedCount; i++)
                    {
                        Debug("w1(1-wire) master[%d] removed during searching... so as the slaves on it...\n", mastersRemoved[i]);

						currentMaster = mastersRemoved[i];
						
						index = find_master_index(currentMaster, g_mastersIDs, g_mastersCount);

						if(index >= 0)
						{
							for(j = 0; j < g_slavesCount[index]; j++)
							{
								w1_reg_num__to_string(g_slavesIDs[index] + j, idString);
								Debug("w1(1-wire) slave[%s] removed on master[%d] due to master removing...\n", idString, currentMaster);

								if(g_userCallbacks != NULL && g_userCallbacks->slave_removed_callback != NULL)
                                	g_userCallbacks->slave_removed_callback(currentMaster, g_slavesIDs[index][j]);
							}
						}
						else
						{
							Debug("w1(1-wire) master[%d] judged to be removed, but it dosen't exist in the global list!");
						}

                        if(g_userCallbacks != NULL && g_userCallbacks->master_removed_callback != NULL)
                            g_userCallbacks->master_removed_callback(mastersRemoved[i]);
						
                    }
                }


				//process added masters...
				if(mastersAddedCount > 0)
                {

					hasChanged = TRUE;
				
                    for(i = 0; i < mastersAddedCount; i++)
                    {
							
                        Debug("w1(1-wire) master[%d] added on during searching...\n", mastersAdded[i]);

                        if(g_userCallbacks != NULL && g_userCallbacks->master_added_callback != NULL)
                            g_userCallbacks->master_added_callback(mastersAdded[i]);

						sleep(1); //let the 1-Wire bus have a break
						
						currentMaster = mastersAdded[i];

						index2 = find_master_index(currentMaster, newMastersIDs, newMastersCount);

						//Search Slaves...
	                    if(w1_master_search(currentMaster, slavesSearched, &slavesSearchedCount))
	                    {
						
							newSlavesCount[index2] = slavesSearchedCount;
							
							memcpy(newSlavesIDs[index2], slavesSearched, sizeof(w1_slave_rn) * slavesSearchedCount);

							if(slavesSearchedCount > 0)
							{
								
								for(j = 0; j < slavesSearchedCount; j++)
								{
									w1_reg_num__to_string(slavesSearched + j, idString);
									Debug("w1(1-wire) slave[%s] added on master[%d] during searching...\n", idString, currentMaster);

									if(g_userCallbacks != NULL && g_userCallbacks->slave_added_callback != NULL)
										g_userCallbacks->slave_added_callback(currentMaster, slavesSearched[j]);
								}
							}

						}
						else
	                    {
	                        Debug("w1 slaves searching failed on master[%d]...\n", currentMaster);
	                    }
                    }
                }

				//process kept masters...
				if(mastersKeptCount > 0)
				{
					for(i = 0; i < mastersKeptCount; i++)
                    {
						sleep(1); //let the 1-Wire bus have a break
						
                        Debug("w1(1-wire) master[%d] kept during searching... now search slaves on it...\n", mastersKept[i]);

						currentMaster = mastersKept[i];
						
						index = find_master_index(currentMaster, g_mastersIDs, g_mastersCount);
						index2 = find_master_index(currentMaster, newMastersIDs, newMastersCount);
						
						if(index == index2)
						{
							Debug("w1(1-wire) master[%d]'s index[%d] not changed! \n", mastersKept[i], index);
						}
						else
						{
							Debug("w1(1-wire) master[%d]'s old index is %d, new index is %d \n", mastersKept[i], index, index2);
						}

						if(index >= 0)
						{
							//Search Slaves...
		                    if(w1_master_search(currentMaster, slavesSearched, &slavesSearchedCount))
		                    {

								Debug("w1(1-wire) master[%d] search: %d slaves found! \n", currentMaster, slavesSearchedCount);
							
								w1_compare_slaves(g_slavesIDs[index], g_slavesCount[index], slavesSearched, slavesSearchedCount,
                                          slavesAdded, &slavesAddedCount, slavesRemoved, &slavesRemovedCount,
                                          slavesKept, &slavesKeptCount);

								Debug("w1(1-wire) master[%d]: before %d slaves, now %d slaves added, %d slaves kept, %d slaves removed! \n", 
									currentMaster, g_slavesCount[index], slavesAddedCount, slavesKeptCount, slavesRemovedCount);

								if(slavesAddedCount > 0 || slavesRemovedCount > 0)
								{
								
									hasChanged = TRUE;
										
									newSlavesCount[index2] = slavesKeptCount + slavesAddedCount;
	                        		memcpy(newSlavesIDs[index2], slavesKept, sizeof(w1_slave_rn) * slavesKeptCount);
	                        		memcpy(newSlavesIDs[index2] + slavesKeptCount, slavesAdded, sizeof(w1_slave_rn) * slavesAddedCount);

									if(slavesAddedCount > 0)
			                        {
			                            for(j = 0; j < slavesAddedCount; j++)
			                            {
			                                w1_reg_num__to_string(slavesAdded + j, idString);
			                                Debug("w1(1-wire) slave[%s] added on master[%d] during searching...\n", idString, currentMaster);

			                                if(g_userCallbacks != NULL && g_userCallbacks->slave_added_callback != NULL)
			                                    g_userCallbacks->slave_added_callback(currentMaster, slavesAdded[j]);
			                            }
			                        }
									
			                        if(slavesRemovedCount > 0)
			                        {
			                            for(j = 0; j < slavesRemovedCount; j++)
			                            {
			                                w1_reg_num__to_string(slavesRemoved + j, idString);
			                                Debug("w1(1-wire) slave[%s] removed from master[%d] during searching...\n", idString, currentMaster);

			                                if(g_userCallbacks != NULL && g_userCallbacks->slave_removed_callback != NULL)
			                                    g_userCallbacks->slave_removed_callback(currentMaster, slavesRemoved[j]);
			                            }
			                        }
								}
								
							}
							else
		                    {
		                        Debug("w1 slaves searching failed on master[%d]...\n", currentMaster);
		                    }
						}
						else
						{
							Debug("w1(1-wire) master[%d] judged to be kept, but it dosen't exist in the global list! Why???");
						}
						
                    }
				}
				
				

				//copy new list into the global list
				//pthread_mutex_lock(&g_globalLocker);
				lock();

				if(hasChanged)
				{
					g_mastersCount = newMastersCount;
					memcpy(g_mastersIDs, newMastersIDs, sizeof(w1_master_id) * newMastersCount);
					memcpy(g_slavesCount, newSlavesCount, sizeof(int) * newMastersCount);
					memcpy(g_slavesIDs, newSlavesIDs, sizeof(w1_slave_rn) * MAX_SLAVE_COUNT * newMastersCount);
				}
				
				//pthread_mutex_unlock(&g_globalLocker);
				unlock();

            }
            else
            {
                Debug("w1 master searching failed...\n");
            }

			//pthread_mutex_unlock(&g_globalLocker);

        }

        usleep(g_w1SearchingInterval * 1000);   //by microsecond

		
    }

    Debug("w1(1-wire) searching thread stopped!\n");

    sh_signal_notify(&g_w1SearchingThreadStopSignal);

    return 0;
}



static void w1_searching_loop2(void * param)
{
    w1_searching_loop(param);
}



static void start_searching_thread(void)
{
    g_w1SearchingThreadStopFlag = 0;
    g_w1SearchingThreadPauseFlag = 0;

    sh_signal_init(&g_w1SearchingThreadStopSignal);


	if(NULL == g_userCallbacks->create_thread_cb)
    {
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

        //Unless we need to use the 4rd argument in the callback, the 4th argument can be NULL
        pthread_create(&g_w1SearchingThread, &attr, w1_searching_loop, NULL);

		Debug("w1(1-wire) slaves searching created by linux pthread...\n");
    }
	else
    {
        //g_w1SearchingThread = sh_create_thread("w1_netlink_searching", w1_searching_loop, NULL);
        g_w1SearchingThread = g_userCallbacks->create_thread_cb("w1_netlink_searching", w1_searching_loop2, NULL);

		Debug("w1(1-wire) slaves searching created by android runtime...\n");
    }
}


static void stop_searching_thread(void)
{
    g_w1SearchingThreadStopFlag = 1;
    g_w1SearchingThreadPauseFlag = 1;

    {
        sh_signal_timedwait(&g_w1SearchingThreadStopSignal, 5000);
    }

    sh_signal_destroy(&g_w1SearchingThreadStopSignal);
}




/* ====================================================================== */
/* =========================== Public functions ========================= */
/* ====================================================================== */

/**
 * Inject callbacks...
**/
void w1_netlink_userservice_init(w1_user_callbacks * w1UserCallbacks)
{
    g_userCallbacks = w1UserCallbacks;

	if(NULL == g_userCallbacks)
	{
		Debug("w1(1-wire) netlink service init without callbacks...\n");
	}
	else
	{
		Debug("w1(1-wire) netlink service init with callbacks...\n");
	}
}

/**
 * The other functions cannot be used before started.
**/
BOOL w1_netlink_userservice_start()
{
    int retcode = 0;

    Debug("w1(1-wire) netlink service starting...\n");

    //pthread_mutex_init(&g_globalLocker, NULL);
    sh_locker_init(&g_globalLocker);

    //open socket
    g_w1NetlinkSocket = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_CONNECTOR);
    if(-1 == g_w1NetlinkSocket)
    {
        perror("socket open");
        return FALSE;
    }

    Debug("w1(1-wire) netlink open socket OK!\n");

    g_bindAddr.nl_family = AF_NETLINK;
    g_bindAddr.nl_groups = g_group;
    g_bindAddr.nl_pid = getpid();

    g_dataAddr.nl_family = AF_NETLINK;
    g_dataAddr.nl_groups = g_group;
    g_dataAddr.nl_pid = 0;

    //bind socket
    retcode = bind(g_w1NetlinkSocket, (struct sockaddr *)&g_bindAddr, sizeof(struct sockaddr_nl));
    if (retcode == -1)
    {
        perror("socket bind");

        Debug("socket bind error: %s\n", strerror(errno));

        //close socket
        close(g_w1NetlinkSocket);
        return FALSE;
    }

    Debug("w1(1-wire) netlink socket bind OK!\n");

    //Add membership to W1 Group. Or, you cannot send any message.
    retcode = setsockopt(g_w1NetlinkSocket, SOL_NETLINK, NETLINK_ADD_MEMBERSHIP, &g_group, sizeof(g_group));
    if (retcode < 0)
    {
        perror("socket setsockopt");

        Debug("socket setsockopt error: %s\n", strerror(errno));

        //close socket
        close(g_w1NetlinkSocket);
        return FALSE;
    }

    Debug("w1(1-wire) netlink socket setsockopt OK!\n");

    //init socket messages
    nlMsgSend = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_MSG_SIZE));
    nlMsgRecv = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_MSG_SIZE));
    if (nlMsgSend == NULL || nlMsgRecv == NULL)
    {
        Debug("Cannot allocate memory for netlink message header\n");
        //close socket
        close(g_w1NetlinkSocket);
        return FALSE;
    }
    memset(nlMsgSend, 0, NLMSG_SPACE(MAX_MSG_SIZE));
    memset(nlMsgRecv, 0, NLMSG_SPACE(MAX_MSG_SIZE));

    //init cnmsg as out buffer
    g_outMsg = (struct cn_msg *)malloc(MAX_CNMSG_SIZE);
    if(NULL == g_outMsg)
    {
        Debug("Cannot allocate memory for out cnmsg!\n");
        //close socket
        close(g_w1NetlinkSocket);
        return FALSE;
    }
    memset(g_outMsg, 0, MAX_CNMSG_SIZE);

    //init cnmsg as ack buffer
    g_ackMsg = (struct cn_msg *)malloc(MAX_CNMSG_SIZE);
    if(NULL == g_ackMsg)
    {
        Debug("Cannot allocate memory for ack cnmsg!\n");
        //close socket
        close(g_w1NetlinkSocket);
        return FALSE;
    }
    memset(g_ackMsg, 0, MAX_CNMSG_SIZE);

    start_receiving_thread();

    int i = 0;
    int j = 0;
    w1_master_id masterId = 0;

    //List Masters...
    g_mastersCount = 0;

    for(i = 0; i < MAX_MASTER_COUNT; i++)
    {
        g_slavesCount[i] = 0;
    }

    /*
    w1_list_masters(g_mastersIDs, &g_mastersCount);

    Debug("%d w1 masters listed!\n", g_mastersCount);

    if(g_mastersCount > 0)
    {
        for(i = 0; i < g_mastersCount; i++)
        {
            masterId = g_mastersIDs[i];

            //Search Slaves...
            w1_master_search(masterId, g_slavesIDs[i], (g_slavesCount + i));

            Debug("%d w1 slaves searched for master[%d]!\n", g_slavesCount[i], masterId);
        }

    }
    */

    start_searching_thread();

    Debug("w1(1-wire) netlink userspace service started!\n");

    return TRUE;
}

/**
 * Please stop userspace service once you finish of using it.
**/
void w1_netlink_userservice_stop()
{

    stop_searching_thread();

    //Attesntion:
    //if the thread is blocked inside [recvmsg], this method will be blocked here forever!!!
    stop_receiving_thread();

    //Remove membership when you don't want to use netlink
    setsockopt(g_w1NetlinkSocket, SOL_NETLINK, NETLINK_DROP_MEMBERSHIP, &g_group, sizeof(g_group));

    //close socket
    close(g_w1NetlinkSocket);

    free(nlMsgSend);
    free(nlMsgRecv);

    free(g_outMsg);
    free(g_ackMsg);

    //pthread_mutex_destroy(&g_globalLocker);
    sh_locker_destroy(&g_globalLocker);

    Debug("w1(1-wire) netlink userspace service stopped!\n");

}





/**
 * Pause w1 searching thread, so that it won't interfere the transaction.
**/
BOOL pause_w1_searching_thread()
{
    if(1 == g_w1SearchingThreadPauseFlag)
        return FALSE;

    lock();
    g_w1SearchingThreadPauseFlag = 1;   //needs locker???
    unlock();

    return TRUE;
}

/**
 * Wakeup w1 searching thread.
**/
void wakeup_w1_searching_thread()
{
    lock();
    g_w1SearchingThreadPauseFlag = 0;   //needs locker???
    unlock();
}


/* ====================================================================== */
/* ========================== private methods =========================== */
/* ====================================================================== */

int generate_w1_global_sequence(void)
{
    int ret = 0;
    lock();
    ret = g_globalSeq++;
    unlock();
    return ret;
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
    ret = sendmsg(g_w1NetlinkSocket, &socketMsgSend, 0);

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

static BOOL transact_w1_msg(BYTE w1MsgType, BYTE w1CmdType,
                            BYTE * masterOrSlaveId, int idLen,
                            void * dataIn, int dataInLen,
                            //void * dataOut, int * dataOutLen)
                            struct w1_netlink_msg ** ppRecvMsg)
{
    if((idLen < 0) || (idLen > 0 && NULL == masterOrSlaveId)) return FALSE;
    if((dataInLen < 0) || (dataInLen > 0 && NULL == dataIn))  return FALSE;
    //if(NULL == ppRecvMsg) return FALSE;
    //if(NULL == dataOut || NULL == dataOutLen) return FALSE;

    //check busy or not
    BOOL isBusy = FALSE;

	lock();
    if(g_isProcessing)
        isBusy = TRUE;              //already busy
    else
        g_isProcessing = TRUE;   //mark busy
    unlock();
	
    if(isBusy)
    {
    	Debug("transact_w1_msg failed because of busy...");
		return FALSE;        //busy
    }

    //declaration
    BOOL succeed = FALSE;
    struct cn_msg * cnmsg = NULL;
    struct w1_netlink_msg * w1msg = NULL;
    struct w1_netlink_cmd * w1cmd = NULL;

    //allocate new message
    //cnmsg = malloc_w1_netlinkmsg();
    cnmsg = g_outMsg;
    refresh_w1_netlinkmsg(g_outMsg);
    w1msg = (struct w1_netlink_msg *)(cnmsg + 1);
    w1cmd = (struct w1_netlink_cmd *)(w1msg + 1);

    //assemble w1cmd
    w1cmd->cmd = w1CmdType;
    if(dataInLen > 0)
    {
        memcpy(w1cmd->data, dataIn, dataInLen);
    }
    w1cmd->len = dataInLen;

    //assemble w1msg
    w1msg->type = w1MsgType;
    if(idLen > 0)
    {
        memcpy(w1msg->id.id, masterOrSlaveId, idLen);
    }
    w1msg->len = sizeof(struct w1_netlink_cmd) + w1cmd->len;

    //assemble cnmsg
    cnmsg->len = sizeof(struct w1_netlink_msg) + w1msg->len;

    /*
    Debug("Print SendMsg below >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> \n");
    print_cnmsg(cnmsg);
    print_w1msg(w1msg);
    print_w1cmd(w1cmd);
    */

    //send the message
    succeed = send_w1_netlinkmsg(cnmsg);

    if(!succeed) goto End;

    //Debug("Before sh_signal_wait...\n");

    //waiting for the ack message
    if(sh_signal_timedwait(&g_waitAckMsgSignal, WAIT_ACK_TIMEOUT) != 0)
    //if(sh_signal_wait(&g_waitAckMsgSignal) != 0)
    {
        Error("Cannot wait signal during %d ms! This command[%x,%x] is failed!", 
			WAIT_ACK_TIMEOUT, w1MsgType, w1CmdType);
        succeed = FALSE;
        goto End;
    }

    //Debug("After sh_signal_wait... OK\n");

    *ppRecvMsg = (struct w1_netlink_msg *)(g_ackMsg + 1);

    /*
    *ppRecvMsg = (struct w1_netlink_msg *)malloc(g_ackMsg->len);
    if(NULL == *ppRecvMsg)
    {
        Debug("Out of memory!\n"); //It will sometimes occur...
        succeed = FALSE;
        goto End;
    }
    memset(*ppRecvMsg, 0, g_ackMsg->len);
    memcpy(*ppRecvMsg, g_ackMsg->data, g_ackMsg->len);
    */

    /*
    if(g_ackMsg->len > 0 && dataOut != NULL)
        memcpy(dataOut, g_ackMsg->data, g_ackMsg->len);

    if(dataOutLen != NULL)
        *dataOutLen = g_ackMsg->len;
    */
    //Debug("Print AckMsg below >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> \n");
    //print_cnmsg(g_ackMsg);
    //print_w1msg(*ppRecvMsg);

End:
	lock();
    g_isProcessing = FALSE;
	unlock();
    //free message
    //free_w1_netlinkmsg(cnmsg);
    return succeed;
}


/* ====================================================================== */
/* =========================== public methods =========================== */
/* ====================================================================== */


/**
 * Synchronized method, search all the slaves on this master.
**/
BOOL w1_master_search(w1_master_id masterId, w1_slave_rn * slaves, int * pSlaveCount)
{
    if(0 == masterId) return FALSE;   //no master id
    if(NULL == slaves) return FALSE;
    if(NULL == pSlaveCount) return FALSE;

    BOOL succeed = FALSE;
    BOOL isSearchAlarm = FALSE;

	w1_slave_rn slavesSearched[MAX_SLAVE_COUNT];
	int slavesSearchedCount = 0;
	w1_slave_rn * p = NULL;
	int i, j;
	
    //struct w1_netlink_msg ** ppRecvMsg = malloc(sizeof(struct w1_netlink_msg *));

    struct w1_netlink_msg * w1msgRecv = NULL;
    //Pay attention of all warnings, or you will get unspected result....
    //struct w1_netlink_msg * w1cmdRecv = NULL; //issue here!!!! Wrong declaration!!!!
    struct w1_netlink_cmd * w1cmdRecv = NULL;


	Debug("w1_master_search begin");


    succeed = transact_w1_msg(W1_MASTER_CMD, (isSearchAlarm ? W1_CMD_ALARM_SEARCH : W1_CMD_SEARCH),
                              (BYTE *)&masterId, sizeof(w1_master_id), NULL, 0, &w1msgRecv);

    if(succeed)
        succeed = (0 == w1msgRecv->status) ? TRUE : FALSE;

	Debug("w1_master_search with flag[%d]", succeed);

    if(succeed)
    {
        w1cmdRecv = (struct w1_netlink_cmd *)(w1msgRecv->data);

		slavesSearchedCount = w1cmdRecv->len / sizeof(w1_slave_rn);
        //*pSlaveCount = w1cmdRecv->len / sizeof(w1_slave_rn);

		
		Debug("w1_master_search with slave count[%d]", slavesSearchedCount);

        if(slavesSearchedCount > 0)
        {
        	p = w1cmdRecv->data;
			
            //filter invalid salve rn
            for(i = 0, j = 0; i < slavesSearchedCount; i++)
			{
				if((p + i)->id != 0)
				{
					slavesSearched[j++] = *(p + i);
				}
			}

			*pSlaveCount = j;
			
            memcpy(slaves, slavesSearched, (*pSlaveCount) * sizeof(w1_slave_rn));
        }
		else
		{
			*pSlaveCount = 0;
		}
		
		Debug("w1_master_search with valid slave count[%d]", *pSlaveCount);
		
    }

    if(!w1msgRecv) free(w1msgRecv);

	Debug("w1_master_search end");
	

    return succeed;
}


/**
 * Synchronized method, reset this master.
**/
BOOL w1_master_reset(w1_master_id masterId)
{
    if(0 == masterId) return FALSE;   //no master id

    BOOL succeed = FALSE;

    struct w1_netlink_msg * w1msgRecv = NULL;

    succeed = transact_w1_msg(W1_MASTER_CMD, W1_CMD_RESET, (BYTE *)&masterId, sizeof(w1_master_id),
                              NULL, 0, &w1msgRecv);

    if(succeed)
        succeed = (0 == w1msgRecv->status) ? TRUE : FALSE;

    if(!w1msgRecv) free(w1msgRecv);

    return succeed;
}

/**
 * Synchronized method, only support READ, WRITE, TOUCH.
 *
 * Input Parameters: masterOrSlaveId, idLen, w1CmdType, dataIn, dataInLen
 * Output Parameters: dataOut, pDataOutLen
**/
static BOOL w1_process_cmd(BYTE * masterOrSlaveId, int idLen, BYTE w1CmdType,
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
    struct w1_netlink_cmd * w1cmdRecv = NULL;

    succeed = transact_w1_msg((sizeof(w1_slave_rn) == idLen) ? W1_SLAVE_CMD : W1_MASTER_CMD,
                              w1CmdType, masterOrSlaveId, idLen,
                              dataIn, dataInLen, &w1msgRecv);

    if(succeed)
        succeed = (0 == w1msgRecv->status) ? TRUE : FALSE;

    if(succeed)
    {
        w1cmdRecv = (struct w1_netlink_cmd *) (w1msgRecv->data);

        //print_w1cmd(w1cmdRecv);

        memcpy(dataOut, w1cmdRecv->data, w1cmdRecv->len);
        //*pDataOut = w1cmdRecv->data; //TODO: Copy out???
        *pDataOutLen = w1cmdRecv->len; //Actually, the number of in & out are exactly the same
    }

    if(!w1msgRecv) free(w1msgRecv);

    return succeed;
}

/**
 * Synchronized method
 *
 * Input Parameters: masterId, readLen
 * Output Parameters: dataOut
**/
BOOL w1_master_read(w1_master_id masterId, int readLen, void * dataOut)
{
    if(0 >= masterId || 0 >= readLen) return FALSE;
    if(NULL == dataOut) return FALSE;

    BYTE dataIn[readLen];
    memset(dataIn, 0xFF, readLen);

    int dataOutLen = readLen;

    return w1_process_cmd((BYTE *)&masterId, sizeof(w1_master_id), W1_CMD_READ,
                          dataIn, readLen, dataOut, &dataOutLen);
}

/**
 * Synchronized method
 *
 * Input Parameters: masterId, writeLen
 * Output Parameters: dataIn
**/
BOOL w1_master_write(w1_master_id masterId, int writeLen, void * dataIn)
{
    if(0 >= masterId || 0 >= writeLen) return FALSE;
    if(NULL == dataIn) return FALSE;

    BYTE dataOut[writeLen];
    memset(dataOut, 0xFF, writeLen);

    int dataOutLen = writeLen;

    return w1_process_cmd((BYTE *)&masterId, sizeof(w1_master_id), W1_CMD_WRITE,
                          dataIn, writeLen, dataOut, &dataOutLen);
}

/**
 * Synchronized method
 *
 * Input Parameters: masterId, dataIn, dataInLen
 * Output Parameters: dataOut, pDataOutLen
**/
BOOL w1_master_touch(w1_master_id masterId, void * dataIn, int dataInLen, void * dataOut, int * pDataOutLen)
{
    if(0 >= masterId || 0 >= dataInLen) return FALSE;

    return w1_process_cmd((BYTE *)&masterId, sizeof(w1_master_id), W1_CMD_TOUCH,
                          dataIn, dataInLen, dataOut, pDataOutLen);
}



/**
 * Synchronized method, cannot use Asynchronized way,
 * because more than 1 ack will be received if succeed.
 * Attention: the "masters" & "pMasterCount" are used as output parameters.
**/
BOOL w1_list_masters(w1_master_id * masters, int * pMasterCount)
{
    //check input
    if(NULL == masters) return FALSE;
    if(NULL == pMasterCount) return FALSE;

    BOOL succeed = FALSE;
	
	w1_master_id mastersListed[MAX_MASTER_COUNT];
	int mastersListedCount = 0;
	w1_master_id * p = NULL;
	int i, j;

    struct w1_netlink_msg * w1msgRecv = NULL;

	Debug("w1_list_masters begin");

    succeed = transact_w1_msg(W1_LIST_MASTERS, 0, NULL, 0, NULL, 0, &w1msgRecv);

	Debug("w1_list_masters 1 with flag[%d]", succeed);

    if(succeed)
    {
        succeed = (0 == w1msgRecv->status) ? TRUE : FALSE;

		//if unsucceed, cannot use pointer w1msgRecv, otherwise the system will reset by "signal 11 (SIGSEGV)"
		Debug("w1_list_masters 2 with flag[%d]", w1msgRecv->status);
    }
	
    if(succeed)
    {
        //It will return the w1msg with master IDs (w1msg->data)
        //It is processed inside [w1_process_command_root] of w1_netlink.c
        //If w1msg sent back one by one, then the ack will begin with 1, plus 1 by 1, and end with 0
        //Here we consider it will send all IDs back inside one w1msg.
        mastersListedCount = w1msgRecv->len / sizeof(w1_master_id);
		
		Debug("w1_list_masters 3 with master count[%d]", mastersListedCount);
		
        if(mastersListedCount > 0)
        {
        	p = w1msgRecv->data;
			
            //filter invalid master id
            for(i = 0, j = 0; i < mastersListedCount; i++)
			{
				if(*(p + i) > 0)
				{
					mastersListed[j++] = *(p + i);
				}
			}

			*pMasterCount = j;
			memcpy(masters, mastersListed, (*pMasterCount) * sizeof(w1_master_id));
			//memcpy(masters, w1msgRecv->data, (*pMasterCount) * sizeof(w1_master_id));
        }
		else
		{
			*pMasterCount = 0;	
		}
		
		Debug("w1_list_masters 4 with valid master count[%d]", *pMasterCount);

    }

    if(!w1msgRecv) free(w1msgRecv);

	Debug("w1_list_masters end");

    return succeed;
}

// implement ======================================================================


static BOOL w1_is_debug_enabled()
{
	return g_debug_enabled;
}

static void w1_set_debug_enabled(BOOL enableOrNot)
{
	g_debug_enabled = enableOrNot;
}

static BOOL w1_master_begin_exclusive()
{
    return pause_w1_searching_thread();
}

static void w1_master_end_exclusive()
{
    wakeup_w1_searching_thread();
}


static void w1_get_current_masters(w1_master_id * masterIDs, int * masterCount)
{
	lock();
	
	if(g_mastersCount > 0)
	{
		*masterCount = g_mastersCount;
		memcpy(masterIDs, g_mastersIDs, sizeof(w1_master_id) * g_mastersCount);
	}
	else
	{
		*masterCount = 0;
	}	
					
	unlock();
}


struct w1_user_service w1_netlink_userservice =
{
    .init = w1_netlink_userservice_init,
    .start = w1_netlink_userservice_start,
    .stop = w1_netlink_userservice_stop,

	.is_debug_enabled = w1_is_debug_enabled,
	.set_debug_enabled = w1_set_debug_enabled,
    
    .begin_exclusive = w1_master_begin_exclusive,
    .end_exclusive = w1_master_end_exclusive,

    //.get_current_master = get_current_w1_master,
    //.get_current_slaves = get_current_w1_slaves,
    .get_current_masters = w1_get_current_masters,

    .list_masters = w1_list_masters,
    .search_slaves = w1_master_search,
    .master_reset = w1_master_reset,
    .master_read = w1_master_read,
    .master_write = w1_master_write,
    .master_touch = w1_master_touch,
};


