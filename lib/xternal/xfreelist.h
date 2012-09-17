/*****************************************************************************\
 *  lib/xternal/xfreelist.h - 
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
 * \file xfreelist.h
 * \author M. Hautreux
 * \date 07/01/2008
 * \brief External freelist headers
 */
#ifndef __XFREELIST_H_
#define __XFREELIST_H_

/*! \addtogroup XTERNAL
 *  @{
 */

/*! \addtogroup XFREELIST
 *  @{
 */

/*!
 * \struct xfreelist_item
 * \typedef xfreelist_item_t
 * \brief External freelist basic element
 */
typedef struct xfreelist_item {

  int free;//!< flag indicating that an item is ready to be used or not

  void* data;//!< pointer on the data that is associated with the item
  size_t size;//!< size of the item 's associated data

  struct xfreelist_item* next;//!< pointer to the previous item or NULL if head
  struct xfreelist_item* previous;//!< pointer to the next item or NULL if tail

  struct xfreelist* freelist;//!< pointer to the corresponding freelist

} xfreelist_item_t;


/*!
 * \struct xfreelist
 * \typedef xfreelist_t
 * \brief external freelist implementation
 */
typedef struct xfreelist {

  xfreelist_item_t* head;//!< pointer to the first item or NULL if empty
  xfreelist_item_t* tail;//!< pointer to the last item or NULL if empty
  
  xfreelist_item_t* items;//!< pointer on the array of freelist items
  unsigned int item_nb;//!< number of items in the array

  void* heap;//!< pointer on memory allocated for items data storage
  size_t item_size;//!< size of each item data chunk
  
  void* next;//!< pointer on the next freelist or NULL if the freelist was not extended

} xfreelist_t;


/*!
 * \fn int xfreelist_init(xfreelist_t* list,unsigned int default_length,size_t item_size)
 * \brief initialize freelist \a list for \a default_length element of size \a item_size
 *
 * \param list pointer on a xfreelist_t type to initialize
 * \param default_length default number of items to create
 * \param item_size size of the data associated with each item
 *
 * \retval XSUCCESS success
 * \retval XERROR_MEMORY unable to allocate memory for data storage
 *  
 */
int
xfreelist_init(xfreelist_t* list,unsigned int default_length,size_t item_size);

/*!
 * \fn int xfreelist_free_contents(xfreelist_t* list)
 * \brief free a previously initialized freelist \a list
 *
 * \param list pointer on the xfreelist_t type to free contents of
 *
 * \retval XSUCCESS on success
 * \retval XERROR generic error
 *
 */
int
xfreelist_free_contents(xfreelist_t* list);

/*!
 * \fn int xfreelist_extend(xfreelist_t* list)
 * \brief extend a previously initialized freelist \a list
 *
 * \param list pointer on a xfreelist_t type to extend
 *
 * \retval XSUCCESS on success
 * \retval XERROR_MEMORY unable to allocate memory for data storage
 *
 */
int
xfreelist_extend(xfreelist_t* list);

/*!
 * \fn int xfreelist_extract_item(xfreelist_t* list,xfreelist_item_t** pitem)
 * \brief extract from a previously initialized freelist \a list an item
 *
 * \param list pointer on a xfreelist_t type to use for item extraction
 * \param pitem pointer on a freelist item pointer that will be filled with extracted item addr
 *
 * \retval XSUCCESS on success
 * \retval XERROR_FREELIST_IS_EMPTY freelist is currently empty...should retry later
 */
int
xfreelist_extract_item(xfreelist_t* list,xfreelist_item_t** pitem);

/*!
 * \fn int xfreelist_release_item(xfreelist_t* list,xfreelist_item_t* item)
 * \brief release an item
 *
 * \param list pointer on a xfreelist_t type to use for item extraction
 * \param item pointer on a freelist item that must be released
 *
 * \retval XSUCCESS on success
 * \retval XERROR_FREELIST_ITEM_ALREADY_FREE item was previously released
 * \retval XERROR_FREELIST_ITEM_NOT_FOUND item not found in list
 */
int
xfreelist_release_item(xfreelist_t* list,xfreelist_item_t* item);


/*!
 * @}
*/

/*!
 * @}
*/

#endif
