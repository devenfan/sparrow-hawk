/*
 *
 * Original filename: kernel_src_2.6.29/drivers/w1/w1_netlink.h
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

#ifndef __W1_NETLINK_USERSPACE_H
#define __W1_NETLINK_USERSPACE_H

//#include "sh_types.h"

//netlink.h cannot be here... It will cause issues if other file include "w1_netlink_userspace.h"
//#include <linux/netlink.h>		//it will include <linux/socket.h>

//<linux/connector.h> is not contained inside NDK android-5
//Attention: NDK android-5 support android-6 & android 7


/** see kernel: connector.h  */

#include <asm/types.h>

#define CN_IDX_CONNECTOR		0xffffffff
#define CN_VAL_CONNECTOR		0xffffffff

/*
 * Process Events connector unique ids -- used for message routing
 */
#define CN_IDX_PROC			0x1
#define CN_VAL_PROC			0x1
#define CN_IDX_CIFS			0x2
#define CN_VAL_CIFS                     0x1
#define CN_W1_IDX			0x3	/* w1 communication */
#define CN_W1_VAL			0x1
#define CN_IDX_V86D			0x4
#define CN_VAL_V86D_UVESAFB		0x1
#define CN_IDX_BB			0x5	/* BlackBoard, from the TSP GPL sampling framework */
#define CN_IDX_DRBD			0x6
#define CN_VAL_DRBD			0x1

#define CN_NETLINK_USERS		7

/*
 * Maximum connector's message size.
 */
#define CONNECTOR_MAX_MSG_SIZE		16384

/*
 * idx and val are unique identifiers which
 * are used for message routing and
 * must be registered in connector.h for in-kernel usage.
 */

struct cb_id {
	__u32 idx;
	__u32 val;
};

struct cn_msg {
	struct cb_id id;

	__u32 seq;
	__u32 ack;

	__u16 len;		/* Length of the following data */
	__u16 flags;
	__u8 data[0];
};

/*
 * Notify structure - requests notification about
 * registering/unregistering idx/val in range [first, first+range].
 */
struct cn_notify_req {
	__u32 first;
	__u32 range;
};

/*
 * Main notification control message
 * *_notify_num 	- number of appropriate cn_notify_req structures after
 *				this struct.
 * group 		- notification receiver's idx.
 * len 			- total length of the attached data.
 */
struct cn_ctl_msg {
	__u32 idx_notify_num;
	__u32 val_notify_num;
	__u32 group;
	__u32 len;
	__u8 data[0];
};


/** see kernel: w1_netlink.h  */

#ifndef	W1_GROUP
#define W1_GROUP  	CN_W1_IDX
#endif

#ifndef  NETLINK_DROP_MEMBERSHIP
#define  NETLINK_DROP_MEMBERSHIP 0
#endif

#ifndef  NETLINK_ADD_MEMBERSHIP
#define  NETLINK_ADD_MEMBERSHIP 1
#endif

#ifndef  SOL_NETLINK
#define  SOL_NETLINK 270
#endif


enum w1_netlink_message_types {
	W1_SLAVE_ADD = 0,
	W1_SLAVE_REMOVE,
	W1_MASTER_ADD,
	W1_MASTER_REMOVE,
	W1_MASTER_CMD,
	W1_SLAVE_CMD,
	W1_LIST_MASTERS,
};

struct w1_netlink_msg
{
	__u8				type;
	__u8				status;
	__u16				len;
	union {
		__u8			id[8];
		struct w1_mst {
			__u32		id;
			__u32		res;
		} mst;
	} id;
	__u8				data[0];
};

enum w1_commands {
	W1_CMD_READ = 0,
	W1_CMD_WRITE,
	W1_CMD_SEARCH,
	W1_CMD_ALARM_SEARCH,
	W1_CMD_TOUCH,
	W1_CMD_RESET,
	W1_CMD_MAX,
};

struct w1_netlink_cmd
{
	__u8				cmd;
	__u8				res;
	__u16				len;
	__u8				data[0];
};



#ifdef __cplusplus
extern "C" {
#endif



/**
* OK return TRUE, Error return FALSE.
* The input-output parameter "outputStr" must be at least 20.
*/
BOOL describe_w1_msg_type(int msgType, char * outputStr);


/**
* OK return TRUE, Error return FALSE.
* The input-output parameter "outputStr" must be at least 20.
*/
BOOL describe_w1_cmd_type(int cmdType, char * outputStr);


/**
* OK return TRUE, Error return FALSE.
* The input-output parameter "outputStr" must be at least 20.
* It's formart Like: %02x.%012llx.%02x
*/
BOOL describe_w1_reg_num(struct w1_reg_num * w1RegNum, char * outputStr);


void print_cnmsg(const struct cn_msg * cnmsg);


void print_w1msg(const struct w1_netlink_msg * w1msg);


void print_w1cmd(const struct w1_netlink_cmd * w1cmd);




#ifdef __cplusplus
}
#endif


#endif /* __W1_NETLINK_USERSPACE_H */
