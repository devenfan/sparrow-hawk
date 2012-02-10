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

#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "sh_types.h"
#include "sh_error.h"
#include "sh_util.h"
#include "sh_thread.h"

//<linux/connector.h> is not contained inside NDK android-5
//Attention: NDK android-5 support android-6 & android 7
#include "kernel_connector.h"

#include "w1_netlink_userspace.h"
#include "w1_netlink_util.h"


/* ====================================================================== */
/* ============================ log ralated ============================= */
/* ====================================================================== */

#define LOG_TAG   "w1_netlink_util"

#define ANDROID_NDK

#include "sh_log.h"

#define Debug(format, args...)    android_debug(LOG_TAG, format, ##args)


/* ====================================================================== */
/* =========================== public methods =========================== */
/* ====================================================================== */

BOOL is_w1_slave_rn_empty(w1_slave_rn rn)
{
    w1_slave_rn empty_rn = W1_EMPTY_REG_NUM;

    /*
    char salveIDStr[20];

    describe_w1_reg_num(&rn, salveIDStr);
    Debug(salveIDStr);

    describe_w1_reg_num(&empty_rn, salveIDStr);
    Debug(salveIDStr);
    */

    return (memcmp(&rn, &empty_rn, sizeof(w1_slave_rn)) == 0) ? TRUE : FALSE;
}


BOOL are_w1_slave_rn_equal(w1_slave_rn rn1, w1_slave_rn rn2)
{
    /*
    char salveIDStr[20];

    describe_w1_reg_num(&rn1, salveIDStr);
    Debug(salveIDStr);

    describe_w1_reg_num(&rn2, salveIDStr);
    Debug(salveIDStr);
    */

    return (memcmp(&rn1, &rn2, sizeof(w1_slave_rn)) == 0) ? TRUE : FALSE;
}

BOOL describe_w1_msg_type(int msgType, char * outputStr)
{
    BOOL ret = TRUE;

    if(outputStr == NULL) return FALSE;

    switch(msgType)
    {
        case W1_SLAVE_ADD:
            sprintf(outputStr, "W1_SLAVE_ADD");
            break;

        case W1_SLAVE_REMOVE:
            sprintf(outputStr, "W1_SLAVE_REMOVE");
            break;

        case W1_MASTER_ADD:
            sprintf(outputStr, "W1_MASTER_ADD");
            break;

        case W1_MASTER_REMOVE:
            sprintf(outputStr, "W1_MASTER_REMOVE");
            break;

        case W1_MASTER_CMD:
            sprintf(outputStr, "W1_MASTER_CMD");
            break;

        case W1_SLAVE_CMD:
            sprintf(outputStr, "W1_SLAVE_CMD");
            break;

        case W1_LIST_MASTERS:
            sprintf(outputStr, "W1_LIST_MASTERS");
            break;

        default:
            ret = FALSE;
            break;
    }

    return ret;
}

BOOL describe_w1_cmd_type(int cmdType, char * outputStr)
{
    BOOL ret = TRUE;

    if(outputStr == NULL) return FALSE;

    switch(cmdType)
    {
        case W1_CMD_READ:
            sprintf(outputStr, "W1_CMD_READ");
            break;

        case W1_CMD_WRITE:
            sprintf(outputStr, "W1_CMD_WRITE");
            break;

        case W1_CMD_SEARCH:
            sprintf(outputStr, "W1_CMD_SEARCH");
            break;

        case W1_CMD_ALARM_SEARCH:
            sprintf(outputStr, "W1_CMD_ALARM_SEARCH");
            break;

        case W1_CMD_TOUCH:
            sprintf(outputStr, "W1_CMD_TOUCH");
            break;

        case W1_CMD_RESET:
            sprintf(outputStr, "W1_CMD_RESET");
            break;

        case W1_CMD_MAX:
            sprintf(outputStr, "W1_CMD_MAX");
            break;

        default:
            ret = FALSE;
            break;
    }

    return ret;
}


BOOL describe_w1_reg_num(struct w1_reg_num * w1RegNum, char * outputStr)
{
    if(w1RegNum == NULL || outputStr == NULL) return FALSE;

    sprintf(outputStr, "%02X.%012llX.%02X", w1RegNum->family, (unsigned long long)w1RegNum->id, w1RegNum->crc);

    return TRUE;
}


void print_cnmsg(const struct cn_msg * cnmsg)
{
    //Attention: DO NOT mistake any of %d, %s... Or, you will get "Segmentation fault".
    Debug("CNMSG: seq[%d], ack[%d], dataLen[%d]\n",
        cnmsg->seq, cnmsg->ack, cnmsg->len);
}


void print_w1msg(const struct w1_netlink_msg * w1msg)
{
	char msgTypeStr[20];

    memset(msgTypeStr, 0, 20);

    describe_w1_msg_type(w1msg->type, msgTypeStr);

    //Attention: DO NOT mistake any of %d, %s... Or, you will get "Segmentation fault".
    Debug("W1MSG: type[%s], dataLen[%d], status[%d]\n",
        msgTypeStr, w1msg->len, w1msg->status);
}


void print_w1cmd(const struct w1_netlink_cmd * w1cmd)
{
	char cmdTypeStr[32];
	char cmdDataStr[w1cmd->len * 2 + 1];   //too large will get OutOfMemory error
	int cmdDataStrLen = 0;

    memset(cmdTypeStr, 0, 32);
    memset(cmdDataStr, 0, w1cmd->len * 2 + 1);

    describe_w1_cmd_type(w1cmd->cmd, cmdTypeStr);

    convert_bytes_to_hexstr((BYTE *)w1cmd->data, 0, w1cmd->len, cmdDataStr, &cmdDataStrLen);

    //Attention: DO NOT mistake any of %d, %s... Or, you will get "Segmentation fault".
    Debug("W1CMD: type[%s], dataLen[%d], data[%s]\n", cmdTypeStr,  w1cmd->len, cmdDataStr);
}
