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


#include "w1_netlink_userspace.h"




char * describe_w1_msg_type(int msgType)
{
    switch(msgType)
    {
        case W1_SLAVE_ADD:
            return "W1_SLAVE_ADD";

        case W1_SLAVE_REMOVE:
            return "W1_SLAVE_REMOVE";

        case W1_MASTER_ADD:
            return "W1_MASTER_ADD";

        case W1_MASTER_REMOVE:
            return "W1_MASTER_REMOVE";

        case W1_MASTER_CMD:
            return "W1_MASTER_CMD";

        case W1_SLAVE_CMD:
            return "W1_SLAVE_CMD";

        case W1_LIST_MASTERS:
            return "W1_LIST_MASTERS";

        default:
            return "UNKNOWN_W1_MSG_TYPE";
    }
}

char * describe_w1_cmd_type(int cmdType)
{
    switch(cmdType)
    {
        case W1_CMD_READ:
            return "W1_CMD_READ";

        case W1_CMD_WRITE:
            return "W1_CMD_WRITE";

        case W1_CMD_SEARCH:
            return "W1_CMD_SEARCH";

        case W1_CMD_ALARM_SEARCH:
            return "W1_CMD_ALARM_SEARCH";

        case W1_CMD_TOUCH:
            return "W1_CMD_TOUCH";

        case W1_CMD_RESET:
            return "W1_CMD_RESET";

        case W1_CMD_MAX:
            return "W1_CMD_MAX";

        default:
            return "UNKNOWN_W1_CMD_TYPE";
    }
}


