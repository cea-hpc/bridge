/*****************************************************************************\
 *  lib/bridged/bridge_rus.h - 
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
 * \file bridge_rus.h
 * \author M. Hautreux
 * \date 06/18/2008
 * \brief Bridge resource usage headers
 */


/*! \addtogroup BRIDGE_DAEMON
 *  @{
 */

#ifndef __BRIDGE_RUS_H_
#define __BRIDGE_RUS_H_


/* multihtreading support */
#include <pthread.h>

#include "xternal/xlibrary.h"

/*! \addtogroup BRIDGE_RUS
 *  @{
 */

#ifndef XLIBRARY_REFERENCE_MAXLENGTH
/*!<
 * \def BRIDGE_RUS_MAX_ID_LENGTH
 * \brief max length of a record identifier
 */
#define BRIDGE_RUS_MAX_ID_LENGTH 128
#else
#define BRIDGE_RUS_MAX_ID_LENGTH XLIBRARY_REFERENCE_MAXLENGTH
#endif

#define BRIDGE_RUS_SUCCESS                              0
#define BRIDGE_RUS_ERROR                           -30000
#define BRIDGE_RUS_ERROR_BAD_ID                    -30001
#define BRIDGE_RUS_ERROR_NO_CONF_FILE              -30010
#define BRIDGE_RUS_ERROR_BAD_CONF_FILE             -30011
#define BRIDGE_RUS_ERROR_LIBRARY_INIT              -30020
#define BRIDGE_RUS_ERROR_SYNC_CMD_INVALID          -30030
#define BRIDGE_RUS_ERROR_SYNC_CMD_FAILED           -30031
#define BRIDGE_RUS_ERROR_ITEM_NOT_FOUND            -30040

/*! \addtogroup BRIDGE_RUS_RECORD
 *  @{
 */

/*!
 * \struct bridge_rus_record
 * \brief bridge resource usage basic record
 *
 * this structure hold usage information about 
 * a specific allocation referenced by its unique identifier
 *
 */
typedef struct bridge_rus_record {

  char         id[BRIDGE_RUS_MAX_ID_LENGTH]; //!< unique identifier of the resource usage record

  time_t       used_time; //!< time already used by this record
  time_t       usable_time; //!< usable amount of time of this record

} bridge_rus_record_t;


/*!
 * \fn int bridge_rus_record_init(bridge_rus_record_t* record,char* id)
 * \brief initialize bridge resource usage record \a record using identifier \a id
 *
 * \param record pointer on the structure to initialize
 * \param id string representation of the record unique identifier
 *
 * \retval BRIDGE_RUS_SUCCESS on success
 * \retval BRIDGE_RUS_ERROR_BAD_ID on failure due to an invalid ID
 * \retval BRIDGE_RUS_ERROR on any other failure
 *  
 */
int
bridge_rus_record_init(bridge_rus_record_t* record,char* id);

/*!
 * \fn int bridge_rus_record_free_contents(bridge_rus_record_t* record)
 * \brief free bridge rus record \a record structure contents
 * \internal
 *
 * \param record pointer on the structure to clean
 *
 * \retval BRIDGE_RUS_SUCCESS on success
 * \retval BRIDGE_RUS_ERROR on any other failure
 *  
 */
int
bridge_rus_record_free_contents(bridge_rus_record_t* record);

/*!
 * @} BRIDGE_RUS_RECORD
*/

/*! \addtogroup BRIDGE_RUS_MGR
 *  @{
 */
#define BRIDGE_RUS_MGR_DEFAULT_LIBRARY_SIZE    500

/*!
 * \struct bridge_rus_mgr
 * \brief bridge resource usage records manager
 *
 * a bridge resource usage manager collects 
 * information about allocations usage and stores it
 * using bridge resource usage record into a library
 * for further access.
 *
 * It periodically refresh its internal information.
 *
 */
typedef struct bridge_rus_mgr {

  char*           config_file; //!< manager configuration file

  char*           synchro_command; //!< external command used for usage information collect
  time_t          refresh_interval; //!< time in seconds to wait before next collect

  time_t          protection_time; //!< time in seconds to automatically add to used time

  xlibrary_t      library; //!< internal structure used to managed records

} bridge_rus_mgr_t;


/*!
 * \fn int bridge_rus_mgr_init(bridge_rus_mgr_t* rus,char* config_file)
 * \brief initialize bridge resource manager \a rus based on configuration stored in \a config_file
 *
 * \param rus pointer on the structure to initialize
 * \param config_file URI of the configuration file to use for initiailization
 *
 * \retval BRIDGE_RUS_SUCCESS on success
 * \retval BRIDGE_RUS_ERROR_NO_CONF_FILE on failure because no conf file is defined
 * \retval BRIDGE_RUS_ERROR_BAD_CONF_FILE on failure due to a bad conf file
 * \retval BRIDGE_RUS_ERROR_LIBRARY_INIT on failure due to library init failure
 * \retval BRIDGE_RUS_ERROR on any other failure
 *  
 */
int
bridge_rus_mgr_init(bridge_rus_mgr_t* rus,char* config_file);

/*!
 * \fn int bridge_rus_mgr_free_contents(bridge_rus_mgr_t* rus)
 * \brief free bridge rus manager \a rus contents
 *
 * \param rus pointer on the structure to destroy
 *
 * \retval BRIDGE_RUS_SUCCESS on success
 * \retval BRIDGE_RUS_ERROR on failure
 *  
 */
int
bridge_rus_mgr_free_contents(bridge_rus_mgr_t* rus);

/*!
 * \fn int bridge_rus_mgr_synchronise(bridge_rus_mgr_t* rus,unsigned long * pnb)
 * \brief collect resource usage information
 *
 * \param rus pointer on the manager structure to use 
 * \param pnb pointer on the current number of items stored 
 *
 * \retval BRIDGE_RUS_SUCCESS on success
 * \retval BRIDGE_RUS_ERROR_SYNC_CMD_INVALID on failure due to an invalid command
 * \retval BRIDGE_RUS_ERROR_SYNC_CMD_FAILED on failure while executing command
 * \retval BRIDGE_RUS_ERROR on any other failure
 *  
 */
int
bridge_rus_mgr_synchronise(bridge_rus_mgr_t* rus,unsigned long * pnb);

/*!
 * \fn int bridge_rus_mgr_get_record(bridge_rus_mgr_t* rus,bridge_rus_record_t* record)
 * \brief dump resource usage record stored by the manager \a rus into record \a record
 * based on the identifier provided by \a record
 *
 * \param rus pointer on the manager structure to use
 * \param record pointer on the bridge rus record structure that contains the ID and that must be filled with usage data
 *
 * \retval BRIDGE_RUS_SUCCESS on success
 * \retval BRIDGE_RUS_ERROR_ITEM_NOT_FOUND on failure due to error while geting item from library
 * \retval BRIDGE_RUS_ERROR on any other failure
 *  
 */
int
bridge_rus_mgr_get_record(bridge_rus_mgr_t* rus,bridge_rus_record_t* record);

/*!
 * @} BRIDGE_RUS_MGR
*/

/*!
 * @} BRIDGE_RUS
*/

/*!
 * @} BRIDGE_DAEMON
*/

#endif
