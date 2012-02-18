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


#ifndef SH_ARRAY_H_INCLUDED
#define SH_ARRAY_H_INCLUDED


typedef struct sh_array_node {
	int     itemSize;   //size of item
	void *  item;
} sh_array_node;

typedef struct sh_array {
	sh_array_node * nodes;
	int             count;
} sh_array;



#ifdef __cplusplus
extern "C" {
#endif


void sh_array_init(sh_array * myArray, void * itemArray, int itemSize, int itemCount);

void sh_add_to_array(sh_array * myArray, sh_array_node * mynode);

void sh_remove_from_array(sh_array * myArray, sh_array_node * mynode);

BOOL sh_array_is_empty(sh_array * myArray);

int sh_array_get_length(sh_array * myArray);


#ifdef __cplusplus
}
#endif



#endif // SH_ARRAY_H_INCLUDED
