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

#ifndef SH_LIST_H_INCLUDED
#define SH_LIST_H_INCLUDED

typedef struct sh_list_node {
	struct sh_list_node * next;
	int data_len;
	void * data;
} sh_list_node;

typedef struct sh_list {
	sh_list_node * head;
} sh_list;

void sh_list_init(sh_list * mylist);

void sh_add_to_list(sh_list * mylist, sh_list_node * mynode);

void sh_remove_from_list(sh_list * mylist, sh_list_node * mynode);

BOOL sh_list_is_empty(sh_list * mylist);

int sh_list_get_length(sh_list * mylist);

#endif // SH_LIST_H_INCLUDED
