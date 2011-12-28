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

#include "w1_netlink_userspace.h"

#define DebugLine(input)   printf(">>>>>>>>>> w1_netlink_userspace.c : %s  \n", (input))


BOOL is_w1_slave_rn_empty(w1_slave_rn rn)
{
    w1_slave_rn empty_rn = W1_EMPTY_REG_NUM;

    /*
    char salveIDStr[20];

    describe_w1_reg_num(&rn, salveIDStr);
    DebugLine(salveIDStr);

    describe_w1_reg_num(&empty_rn, salveIDStr);
    DebugLine(salveIDStr);
    */

    return (memcmp(&rn, &empty_rn, sizeof(w1_slave_rn)) == 0) ? TRUE : FALSE;
}


BOOL are_w1_slave_rn_equal(w1_slave_rn rn1, w1_slave_rn rn2)
{
    /*
    char salveIDStr[20];

    describe_w1_reg_num(&rn1, salveIDStr);
    DebugLine(salveIDStr);

    describe_w1_reg_num(&rn2, salveIDStr);
    DebugLine(salveIDStr);
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

