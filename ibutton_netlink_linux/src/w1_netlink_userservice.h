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

#ifndef __W1_NETLINK_USERSERVICE_H
#define __W1_NETLINK_USERSERVICE_H


#include "w1_netlink_userspace.h"



typedef void w1_master_added(int master_id);

typedef void w1_master_removed(int master_id);

typedef void w1_master_listed(int * master_ids, int master_count);

typedef void w1_slave_added(w1_slave_rn salve_rn);

typedef void w1_slave_removed(w1_slave_rn salve_rn);

typedef void w1_slave_found(w1_slave_rn * slave_ids, int slave_count);



typedef struct w1_user_interface{

    w1_master_added * master_added_callback;

    w1_master_removed * master_removed_callback;

    w1_master_listed * master_listed_callback;

    w1_slave_added * slave_added_callback;

    w1_slave_removed * slave_removed_callback;

    w1_slave_found * slave_found_callback;

}w1_user_callbacks;




/**
 * The first thing is start service, or other function cannot be used.
 */
BOOL w1_netlink_userservice_start(w1_user_callbacks * w1UserCallbacks);

/**
 * Please stop userspace service once you finish of using it.
 */
BOOL w1_netlink_userservice_stop(void);




/**
 * Synchronized method, only support SEARCH, ALARM_SEARCH, RESET.
 * Attention: the "cmd" is used as input & output parameter
 */
BOOL process_w1_master_cmd(int masterId, struct w1_netlink_cmd * cmd);

/**
 * Synchronized method, only support READ, WRITE, TOUCH.
 * Attention: the "cmd" is used as input & output parameter
 */
BOOL process_w1_slave_cmd(w1_slave_rn * slaveId, struct w1_netlink_cmd * cmd);



/**
 * Asynchronized method, the result will come back later
 *
 */
BOOL request_to_list_w1_masters(void);

/*
 * Asynchronized method, the result will come back later
 *
BOOL request_to_search_w1_slaves(BOOL alarmSearch);
 */



#endif /* __W1_NETLINK_USERSERVICE_H */
