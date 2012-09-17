/*****************************************************************************\
 *  lib/xternal/xerror.h - 
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
 * \file xerror.h
 * \author M. Hautreux
 * \date 07/29/2008
 * \brief External errors code
 */
#ifndef __XERROR_H_
#define __XERROR_H_


#define XSUCCESS                                 0
#define XERROR                                  -1

#define XERROR_EINTR                            -2
#define XERROR_MEMORY                           -3

/* pthread related error code */
#define XERROR_MUTEX_INIT_FAILED               -91
#define XERROR_MUTEX_LOCK_FAILED               -92
#define XERROR_CONDITION_INIT_FAILED           -93

/* xfreelist related error codes */
#define XERROR_FREELIST_INIT_FAILED           -101
#define XERROR_FREELIST_IS_EMPTY              -102
#define XERROR_FREELIST_ITEM_NOT_FOUND        -103
#define XERROR_FREELIST_ITEM_ALREADY_FREE     -104

/* xlibrary related error codes */
#define XERROR_LIBRARY_ITEM_NOT_FOUND         -201
#define XERROR_LIBRARY_OBJECT_NOT_FOUND       -202
#define XERROR_LIBRARY_ADD_FAILED             -203

/* xstream related error code */
#define XERROR_STREAM_SOCKET_FAILED           -301
#define XERROR_STREAM_SETSOCKOPT_FAILED       -302
#define XERROR_STREAM_GETADDRINFO_FAILED      -303
#define XERROR_STREAM_BIND_FAILED             -304
#define XERROR_STREAM_CONNECT_FAILED          -305
#define XERROR_STREAM_POLL_ERROR              -306
#define XERROR_STREAM_TIMEOUT                 -307
#define XERROR_STREAM_SOCKET_CLOSED           -308

/* xqueue related error code */
#define XERROR_QUEUE_FREELIST_IS_NULL         -401
#define XERROR_QUEUE_FREELIST_EXTRACT_ITEM    -402
#define XERROR_QUEUE_IS_EMPTY                 -403

#endif
