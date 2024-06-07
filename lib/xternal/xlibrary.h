/*****************************************************************************\
 *  lib/xternal/xlibrary.h - 
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
 * \file xlibrary.h
 * \author M. Hautreux
 * \date 07/01/2008
 * \brief External library headers
 */
#ifndef __XLIBRARY_H_
#define __XLIBRARY_H_
#define _GNU_SOURCE
/* multihtreading support */
#include <pthread.h>

#include "xerror.h"


/* library implemented using a freelist */
#include "xfreelist.h"


/*! \addtogroup XTERNAL
 *  @{
 */

/*! \addtogroup XLIBRARY
 *  @{
 */

#define XLIBRARY_REFERENCE_MAXLENGTH   128

/*!
 * \struct xlibrary_item
 * \typedef xlibrary_item_t
 * \brief external library basic element
 */
typedef struct xlibrary_item {

  char reference[XLIBRARY_REFERENCE_MAXLENGTH];//!< unique identifier of the item

  time_t timestamp;//!< time in seconds since EPOCH of last modification

  void* object;//!<pointer on a xfreelist item that contains associated data

  void* library;//!<pointer on the associated library

} xlibrary_item_t;


/*!
 * \struct xlibrary
 * \typedef xlibrary_t
 * \brief external library implementation (based on freelist)
 */
typedef struct xlibrary {

  pthread_mutex_t mutex;//!< for thread safety
  pthread_cond_t condition;//!< for thread safety

  xfreelist_t ref_freelist;//!< freelist containing xlibrary items
  xfreelist_t obj_freelist;//!< freelist containing xlibrary items data objects

  void* root;//!< tree root of the library
  int item_nb;//!< items currently stored in the tree

  xlibrary_item_t** index;//!<workaround for tree indexation
  xlibrary_item_t** current;//!<workaround for tree indexation

} xlibrary_t;


/*!
 * \brief create an xlibrary
 *
 * \param library pointer on the xlibrary structure to initialize
 * \param default_length library default number of element
 * \param item_maxsize maximum length of an item associated data buffer
 *
 * \retval XSUCCESS                     success
 * \retval XERROR                       generic error
 * \retval XERROR_MUTEX_INIT_FAILED     unable to initialized repository mutex
 * \retval XERROR_CONDITION_INIT_FAILED unable to initialized repository condition
 * \retval XERROR_FREELIST_INIT_FAILED  unable to initialized freelist
 */
int
xlibrary_init(xlibrary_t* library,
	      size_t default_length,
	      size_t item_maxsize);

/*!
 * \brief free an xlibrary contents
 *
 * \param library xlibrary to free contents of
 *
 * \retval XSUCCESS                     success
 * \retval XERROR                       generic error
 */
int
xlibrary_free_contents(xlibrary_t* library);

/*!
 * \brief add an item to an xlibrary
 *
 * \param library pointer on the xlibrary previously initialized
 * \param reference unique identifier of the item
 * \param item pointer on the item data to add
 * \param item_size length of the data buffer to store
 *
 * \retval XSUCCESS                 success
 * \retval XERROR                   generic error
 * \retval XERROR_MUTEX_LOCK_FAILED unable to lock repository
 * \retval XERROR_FREELIST_IS_EMPTY freelist is empty
 * \retval XERROR_LIBRARY_ADD_FAILED   unable to add credential item to the tree
 */
int
xlibrary_add_item(xlibrary_t* library,
		  char* reference,
		  void* item,
		  size_t item_size);

/*!
 * \brief get an item from a repository based on associated reference
 *
 * \param library pointer on the xlibrary previously initialized and filled
 * \param reference unique identifier of the item
 * \param item pointer on the item data to get
 * \param item_size length of the data buffer to store
 *
 * \retval XSUCCESS                 success
 * \retval XERROR                   generic error
 * \retval XERROR_MUTEX_LOCK_FAILED unable to lock repository
 * \retval XERROR_ITEM_NOT_FOUND    item not found in repository
 * \retval XERROR_OBJECT_NOT_FOUND  data associated with the item not found
 */
int
xlibrary_get_item(xlibrary_t* library,
		  char* reference,
		  void* item,
		  size_t item_size);

/*!
 * \brief remove an item from a library based on associated reference
 *
 * \param library pointer on the xlibrary previously initialized and filled
 * \param reference unique identifier of the item
 *
 * \retval XSUCCESS                 success
 * \retval XERROR                   generic error
 * \retval XERROR_MUTEX_LOCK_FAILED unable to lock repository
 */
int
xlibrary_remove_item(xlibrary_t* library,
		     char* reference);

int xlibrary_lock(xlibrary_t* library);

int xlibrary_unlock(xlibrary_t* library);

int xlibrary_update_index(xlibrary_t* library);

int xlibrary_remove_item_nolock(xlibrary_t* library,
				char* reference);

/*!
 * @}
*/

/*!
 * @}
*/

#endif
