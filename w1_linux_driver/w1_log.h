/*
 *	w1_log.h
 *
 * Copyright (c) 2004 Evgeniy Polyakov <johnpol@2ka.mipt.ru>
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

#ifndef __W1_LOG_H
#define __W1_LOG_H

#define DEBUG

#ifdef W1_DEBUG
#  define assert(expr) do {} while (0)
#else
#  define assert(expr) \
        if(unlikely(!(expr))) {				        \
        printk(KERN_ERR "Assertion failed! %s,%s,%s,line=%d\n",	\
	#expr, __FILE__, __func__, __LINE__);		        \
        }
#endif


#define w1_log_str(str)		printk(KERN_DEBUG "%s\n", (str))

#define w1_log_msgsend(msg)     printk(KERN_DEBUG "SEND a w1 msg[%d, %d]\n", (msg)->type, (((struct w1_netlink_cmd *)((msg)->data))->cmd))

#define w1_log_msgrecv(msg)  	printk(KERN_DEBUG "RECV a w1 msg[%d, %d]\n", (msg)->type, (((struct w1_netlink_cmd *)((msg)->data))->cmd))

#endif /* __W1_LOG_H */

