/*****************************************************************************\
 *  lib/xternal/xqueue.h - 
 *****************************************************************************
 *  Copyright  CEA/DAM/DIF (2012)
 *
 *  This file is part of Bridge, an abstraction layer to ease batch system and
 *  resource manager usage in heterogeneous HPC environments.
 *
 *  Bridge is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Bridge is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Bridge.  If not, see <http://www.gnu.org/licenses/>
\*****************************************************************************/

/**
 * \file xqueue.h
 * \author M. Hautreux
 * \date 07/01/2008
 * \brief External queue headers
 */
#ifndef __XQUEUE_H_
#define __XQUEUE_H_

/* xqueue is implemented using a freelist */
#include "xfreelist.h"

/* multihtreading support */
#include <pthread.h>

/*! \addtogroup XTERNAL
 *  @{
 */

/*! \addtogroup XQUEUE
 *  @{
 */

/*!
 * \struct xqueue
 * \typedef xqueue_t
 * \brief External FIFO queue (based on freelist)
 */
typedef struct xqueue {

  xfreelist_t freelist;//!< freelist used for queue elements storage

  xfreelist_item_t* head;//!< first element
  xfreelist_item_t* tail;//!< last element

  pthread_mutex_t mutex;//!< mutex for thread safety
  pthread_cond_t condition;//!< condition for threads wake up

} xqueue_t;


/*!
 * \fn xqueue_init(xqueue_t* queue,unsigned int default_length,size_t item_size)
 * \brief create a xqueue
 *
 * \param queue pointer on the xqueue structure to initialize
 * \param default_length queue max number of element
 * \param item_size queue 's element size
 *
 * \retval XSUCCESS init succeed
 * \retval XERROR_MUTEX_INIT_FAILED unable to initialize queue mutex
 * \retval XERROR_CONDITION_INIT_FAILED unable to initiailize queue condition
 */
int
xqueue_init(xqueue_t* queue,unsigned int default_length,size_t item_size);

/*!
 * \fn xqueue_free_contents(xqueue_t* queue)
 * \brief free an xqueue contents
 *
 * \param queue xqueue to free contents of
 *
 * \retval XSUCCESS success
 * \retval XERROR generic error
 */
int
xqueue_free_contents(xqueue_t* queue);

/*!
 * \fn xqueue_enqueue(xqueue_t* queue,void* data,size_t length)
 * \brief enqueue an element into the queue
 *
 * \param queue xqueue to enqueue element in
 * \param data pointer on the data to add
 * \param length data size
 *
 * \retval XSUCCESS element successfully enqueued
 * \retval XERROR_MUTEX_LOCK_FAILED mutex lock failed
 * \retavl XERROR_QUEUE_FREELIST_IS_NULL freelist is null
 * \retval XERROR_FREELIST_IS_EMPTY freelist is empty
 * \retval XERROR_QUEUE_FREELIST_EXTRACT_ITEM unable to extract qn item from freelist
 *
 */
int
xqueue_enqueue(xqueue_t* queue,void* data,size_t length);

/*!
 * \fn xqueue_dequeue(xqueue_t* queue,void* data,size_t length)
 * \brief dequeue an element from the queue
 *
 * \param queue xqueue to dequeue from
 * \param data pointer on the element to fill with data
 * \param length data size
 *
 * \retval XSUCCESS element successfully copied and enqueued
 * \retval XERROR generic error
 * \retval XERROR_QUEUE_IS_EMPTY no more item to dequeue
 * \retval XERROR_QUEUE_FREELIST_IS_NULL queue's freelist is NULL
 * \retval XERROR_FREELIST_IS_EMPTY queue's freelist is empty
 * \retval XERROR_MUTEX_LOCK_FAILED mutex lock failed
 */
int
xqueue_dequeue(xqueue_t* queue,void* data,size_t length);


/*!
 * @}
*/

/*!
 * @}
*/

#endif
