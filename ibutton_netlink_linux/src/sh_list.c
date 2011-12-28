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


#include <stddef.h>

#include "sh_types.h"
#include "sh_list.h"

void sh_list_init(sh_list * mylist)
{
    mylist->head = NULL;
}

void sh_add_to_list(sh_list * mylist, sh_list_node * mynode)
{
    if(NULL == mylist) return;
    if(NULL == mynode) return;

    //we mustn't add a node that has linked nodes, or we may get a circle
    if(NULL != mynode->next) return;

    sh_list_node * current = mylist->head;

    if(NULL == current)
    {
        mylist->head = mynode;
        return;
    }

    while(TRUE)
    {
        if(current == mynode)
        {
            break; //already exist
        }

        if(current->next == NULL)
        {
            current->next = mynode;
            break;
        }

        current = current->next;
    }
}

void sh_remove_from_list(sh_list * mylist, sh_list_node * mynode)
{
    if(NULL == mylist) return;
    if(NULL == mynode) return;

    //we mustn't add a node that has linked to itself, or we will have a circle
    if(mynode == mynode->next) return;

    sh_list_node * current = mylist->head;

    if(NULL == current) return; //not exist

    if(mynode == current)
    {
        mylist->head = mynode->next;
        return;
    }

    while(TRUE)
    {
        if(NULL == current->next)
        {
            break; //not exist
        }

        if(mynode == current->next)
        {
            current->next = mynode->next;
            break;
        }

        current = current->next;
    }
}

BOOL sh_list_is_empty(sh_list * mylist)
{
    return (NULL == mylist->head) ? TRUE : FALSE;
}

int sh_list_get_length(sh_list * mylist)
{
    int count = 0;

    sh_list_node * current = mylist->head;

    while(NULL != current)
    {
        count ++;
    }

    return count;
}

