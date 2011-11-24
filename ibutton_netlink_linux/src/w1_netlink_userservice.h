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

#ifndef __W1_NETLINK_USERSERVICE_H
#define __W1_NETLINK_USERSERVICE_H


#include "w1_netlink_userspace.h"



/* The first thing is start service, or other function cannot be used. */
int w1_netlink_userservice_start(void);


int w1_netlink_userservice_stop(void);


/* The size is defined inside w1_netlink_userservice.c */
struct cn_msg * malloc_w1_netlinkmsg(void);

/* You must free the memory once you finish of using it */
void free_w1_netlinkmsg(struct cn_msg * cnmsg);

/* You can re-use the message if you want to save the memory */
void refresh_w1_netlinkmsg(struct cn_msg * cnmsg);

/* Return -1 when error, Return sent size if OK.
 * You can also recycle(free or reuse) it after you send it */
int send_w1_netlinkmsg(struct cn_msg * cnmsg);




#endif /* __W1_NETLINK_USERSERVICE_H */
