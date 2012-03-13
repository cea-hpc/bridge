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
