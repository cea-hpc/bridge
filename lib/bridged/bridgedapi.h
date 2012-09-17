/*****************************************************************************\
 *  lib/bridged/bridgedapi.h - 
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

#ifndef __BRIDGEDAPI_H_
#define __BRIDGEDAPI_H_

#ifndef XSUCCESS
#define XSUCCES 0
#endif

#ifndef XERROR
#define XERROR -1
#endif


/*! \addtogroup BRIDGEDAPI
 *  @{
 */

#define BRIDGE_SUCCESS                                 0
#define BRIDGE_ERROR                                  -1

/* bridge engine related error codes */
#define BRIDGE_ERROR_ENGINE_INIT_FAILED      -1101

/* communication related error codes */
#define BRIDGE_ERROR_STREAM_CONNECT_FAILED   -1201
#define BRIDGE_ERROR_STREAM_TRANS_FAILED     -1202

/* request related error codes */
#define BRIDGE_ERROR_REQUEST_FAILED          -2101
#define BRIDGE_ERROR_REQUEST_MALFORMED_REPLY -2102

/*!
 * @}
*/


/*! \addtogroup BRIDGEDAPI
 *  @{
 */

/*!
 * \brief ping bridged daemon
 *
 * \param conf_file configuration file to use or NULL if default
 *
 * \retval BRIDGE_SUCCESS                     success
 * \retval BRIDGE_ERROR_STREAM_CONNECT_FAILED conenction to servers failed
 * \retval BRIDGE_ERROR_STREAM_TRANS_FAILED transmission with server failed
 * \retval BRIDGE_ERROR                       generic error
 */
int
bridgedapi_ping(char* conf_file);

/*!
 * \brief get stats from bridged daemon
 *
 * \param conf_file configuration file to use or NULL if default
 * \param batchid unique identifier of the batch resource to get information of (NULL if not defined)
 * \param rmid unique identifier of the resource managed job to get information of (NULL if not defined)
 * \param usable pointer on a time_t field that will hold usable time
 * \param used pointer on a time_t field that will hold used time
 * \param halt pointer on a time_t field that will hold a hypothetic halt time (or 0 if no halt time defined
 *
 * \retval BRIDGE_SUCCESS success
 * \retval BRIDGE_ERROR_REQUEST_FAILED server says it can not give a valid response ( bad batchid or rmid )
 * \retval BRIDGE_ERROR_REQUEST_MALFORMED_REPLY reply is invalid
 * \retval BRIDGE_ERROR_STREAM_CONNECT_FAILED conenction to servers failed
 * \retval BRIDGE_ERROR_STREAM_TRANS_FAILED transmission with server failed
 * \retval BRIDGE_ERROR generic error
 *
 */
int
bridgedapi_get(char* conf_file,char* batchid,char* rmid,
	       time_t* usable,time_t* used,time_t* halt);

/*!
 * @}
*/

#endif
