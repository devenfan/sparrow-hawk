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



#include <pthread.h>

#include "sh_thread.h"
#include "sh_error.h"



int sh_signal_init(sh_signal_ctrl * signal)
{
	if(NULL == signal)	return E_NULL_POINTER;

	pthread_mutexattr_init(&(signal->mutex_attr));
    //Add recursive attribute
    pthread_mutexattr_settype(&(signal->mutex_attr), PTHREAD_MUTEX_RECURSIVE_NP);

	if(0 != pthread_mutex_init(&(signal->mutex), &(signal->mutex_attr)))
	{
		return E_CANNOT_INIT;
	}

	if(0 != pthread_cond_init(&(signal->cond), NULL))
	{
		pthread_mutex_destroy(&(signal->mutex));
		return E_CANNOT_INIT;
	}

    signal->cond_counter = 0;

	return E_OK;
}


int sh_signal_destroy(sh_signal_ctrl * signal)
{
	if(NULL == signal) return E_NULL_POINTER;

	int ret = E_OK;

	if(0 != pthread_mutex_destroy(&(signal->mutex)))
	{
		ret = E_CANNOT_DESTROY;
	}

	if(0 != pthread_cond_destroy(&(signal->cond)))
	{
		ret = E_CANNOT_DESTROY;
	}

	return ret;
}


int sh_signal_wait(sh_signal_ctrl * signal)
{
    int result;

    pthread_mutex_lock(&(signal->mutex));

    signal->cond_counter++;

	//OK will get 0
	result = pthread_cond_wait(&(signal->cond), &(signal->mutex));

    pthread_mutex_unlock(&(signal->mutex));

    return result;
}


int sh_signal_notify(sh_signal_ctrl * signal)
{
    int result;

    while(1)
    {
        pthread_mutex_lock(&(signal->mutex));

        if(signal->cond_counter > 0)
        {
            //it will notify the first thread waiting for this signal
            //pthread_cond_signal(&signal->cond);

            //it will notify all the threads waiting for this signal
            result = pthread_cond_broadcast(&(signal->cond));
            pthread_mutex_unlock(&(signal->mutex));
            break;
        }

        pthread_mutex_unlock(&(signal->mutex));
    }

	//OK will return 0
    return result;
}


