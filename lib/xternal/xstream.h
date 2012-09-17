/*****************************************************************************\
 *  lib/xternal/xstream.h - 
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
 * \file xstream.h
 * \author M. Hautreux
 * \date 07/01/2008
 * \brief External stream headers
 */
#ifndef __XSTREAM_H
#define __XSTREAM_H

#include "xerror.h"

/*! \addtogroup XTERNAL
 *  @{
 */

/*! \addtogroup XSTREAM
 *  @{
 */

#include <sys/socket.h>

/*!
 * \brief Create stream socket and connect it to given host and service
 *
 * \param hostname host to connect to
 * \param servicename service to connect to (name or port)
 * \param timeout connection timeout (0 if no timeout)
 *
 * \retval >0 socket fd
 * \retval XERROR generic error
 * \retval XERROR_STREAM_SOCKET_FAILED unable to create socket
 * \retval XERROR_STREAM_SETSOCKOPT_FAILED unable to set socket options
 * \retval XERROR_STREAM_GETADDRINFO_FAILED unable to get hostname addr info
 * \retval XERROR_STREAM_CONNECT_FAILED unable to connect hostname
 * \retval XERROR_STREAM_POLL_ERROR unable to connect hostname due to poll error
 *
*/
int
xstream_connect(const char* hostname,const char* servicename,
		time_t timeout);

/*!
 * \brief Create stream socket and bind it using given host and service
 *
 * \param hostname host to use during bind
 * \param servicename service  to use during bind
 *
 * \retval >0 socket fd
 * \retval XERROR generic error
 * \retval XERROR_STREAM_SOCKET_FAILED unable to create socket
 * \retval XERROR_STREAM_SETSOCKOPT_FAILED unable to set socket options
 * \retval XERROR_STREAM_GETADDRINFO_FAILED unable to get hostname addr info
 * \retval XERROR_STREAM_BIND_FAILED unable to bind socket on any addr
 *
 */
int
xstream_create(const char* hostname,const char* servicename);


/*!
 * \brief Specify queue limit for incoming connections on a a socket
 *
 * \param socket previously created stream socket (@xstream_create)
 * \param backlog queue limit (see man listen for more details)
 *
 * \retval  0 operation successfully done
 * \retval XERROR generic error
 *
*/
int
xstream_listen(int socket, int backlog);


/*!
 * \brief Accept incoming connection from previously created stream socket
 *
 * \param socket previously created stream socket (@xstream_create)
 *
 * \retval >0 incoming request socket fd
 * \retval XERROR generic error
 * \retval XERROR_EINTR interrupted
 *
*/
int
xstream_accept(int socket);


/*!
 * \brief Close previously created or connected stream socket
 *
 * \param socket the socket to close
 *
 * \retval XSUCCESS success
 * \retval XERROR generic error
 *
*/
int
xstream_close(int socket);

/*!
 * \brief Send data into a socket with timeout
 *
 * \param socket the socket FD to use
 * \param buffer pointer on the data to send
 * \param length amount of data to send from buffer
 * \param timeout timeout in milliseconds for the operation
 *
 * \retval XSUCCESS success
 * \retval XERROR generic error
 * \retval XERROR_STREAM_SETSOCKOPT_FAILED unable to switch socket to non blocking mode
 * \retval XERROR_STREAM_TIMEOUT timeout during the operation
 * \retval XERROR_STREAM_POLL_ERROR error while polling
 * \retval XERROR_STREAM_SOCKET_CLOSED socket is closed
 *
*/
int xstream_send_timeout(int socket,char* buffer,size_t length,int timeout);

/*!
 * \brief Send data into a socket without timeout
 *
 * \param socket the socket FD to use
 * \param buffer pointer on the data to send
 * \param length amount of data to send from buffer
 *
 * \retval XSUCCESS success
 * \retval XERROR generic error
 *
*/
int xstream_send(int socket,char* buffer,size_t length);

/*!
 * \brief Receive data from a socket with timeout
 *
 * \param socket the socket FD to use
 * \param buffer pointer on the data to receive
 * \param length amount of data to receive from buffer
 * \param timeout timeout in milliseconds for the operation
 *
 * \retval XSUCCESS success
 * \retval XERROR generic error
 * \retval XERROR_STREAM_SETSOCKOPT_FAILED unable to switch socket to non blocking mode
 * \retval XERROR_STREAM_TIMEOUT timeout during the operation
 * \retval XERROR_STREAM_POLL_ERROR error while polling
 * \retval XERROR_STREAM_SOCKET_CLOSED socket is closed
 *
*/
int xstream_receive_timeout(int socket,char* buffer,size_t length,int timeout);

/*!
 * \brief Receive data from a socket without timeout
 *
 * \param socket the socket FD to use
 * \param buffer pointer on the data to receive
 * \param length amount of data to receive from buffer
 *
 * \retval XSUCCESS success
 * \retval XERROR generic error
 *
*/
int xstream_receive(int socket,char* buffer,size_t length);


/*!
 * \brief Send message into a socket with timeout
 *
 * \param socket the socket FD to use
 * \param buffer pointer on the data to send
 * \param length amount of data to send from buffer
 * \param timeout timeout in milliseconds for the operation
 *
 * \retval XSUCCESS success
 * \retval XERROR generic error
 * \retval XERROR_STREAM_SETSOCKOPT_FAILED unable to switch socket to non blocking mode
 * \retval XERROR_STREAM_TIMEOUT timeout during the operation
 * \retval XERROR_STREAM_POLL_ERROR error while polling
 * \retval XERROR_STREAM_SOCKET_CLOSED socket is closed
 *
*/
int xstream_send_msg_timeout(int socket,char* buffer,size_t length,int timeout);

/*!
 * \brief Send message into a socket without timeout
 *
 * \param socket the socket FD to use
 * \param buffer pointer on the data to send
 * \param length amount of data to send from buffer
 *
 * \retval XSUCCESS success
 * \retval XERROR generic error
 *
*/
int xstream_send_msg(int socket,char* buffer,size_t length);

/*!
 * \brief Receive a message from a socket with timeout
 *
 * \param socket the socket FD to use
 * \param pbuffer pointer on a buffer to allocate and fill with message data (must be free externally)
 * \param plength pointer on the amount of data corresponding to the message
 * \param timeout timeout in milliseconds for the operation
 *
 * \retval XSUCCESS success
 * \retval XERROR generic error
 * \retval XERROR_MEMORY unable to allocate memory for message data storage
 * \retval XERROR_STREAM_SETSOCKOPT_FAILED unable to switch socket to non blocking mode
 * \retval XERROR_STREAM_TIMEOUT timeout during the operation
 * \retval XERROR_STREAM_POLL_ERROR error while polling
 * \retval XERROR_STREAM_SOCKET_CLOSED socket is closed
 *
*/
int xstream_receive_msg_timeout(int socket,char** pbuffer,size_t* plength,int timeout);

/*!
 * \brief Receive data from a socket without timeout
 *
 * \param socket the socket FD to use
 * \param pbuffer pointer on a buffer to allocate and fill with message data (must be free externally)
 * \param plength pointer on the amount of data corresponding to the message
 *
 * \retval XSUCCESS success
 * \retval XERROR generic error
 * \retval XERROR_MEMORY unable to allocate memory for message data storage
 *
*/
int xstream_receive_msg(int socket,char** buffer,size_t* length);



/*!
 * @}
*/

/*!
 * @}
*/

#endif
