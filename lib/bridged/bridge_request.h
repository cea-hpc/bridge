/**
 * \file bridge_request.h
 * \author M. Hautreux
 * \date 07/01/2008
 * \brief Bridge request headers
 */

/*! \addtogroup BRIDGE_API
 *  @{
 */

#ifndef __BRIDGE_REQUEST_H_
#define __BRIDGE_REQUEST_H_

#include "xternal/xmessage.h"

/*! \addtogroup BRIDGE_REQUEST
 *  @{
 */

/*!
 * \struct bridge_get_req
 * \typedef bridge_get_req_t
 * \brief bridge get request wrapper
 */
typedef struct bridge_get_req {

  char* batchid;
  char* rmid;

} bridge_get_req_t;

/*!
 * \struct bridge_get_rep
 * \typedef bridge_get_rep_t
 * \brief bridge get reply wrapper
 */
typedef struct bridge_get_rep {

  time_t used_time;
  time_t usable_time;

  time_t halt_time;

} bridge_get_rep_t;


/*!
 * \brief initialize a bridge_get_req_t
 *
 * \param bgreq pointer on the structure to initialize
 * \param batchid pointer on the string representation of corresponding batch unique identifier
 * \param rmid pointer on the string representation of corresponding resource manager unique identifier
 *
 * \retval XSUCCESS                     success
 * \retval XERROR                       generic error
 */
int
bridge_get_req_init(bridge_get_req_t* bgreq,char* batchid,char* rmid);

/*!
 * \brief free previously initialized bridge_get_req_t contents
 *
 * \param bgreq pointer on the structure to free contents of
 *
 * \retval XSUCCESS                     success
 * \retval XERROR                       generic error
 */
int
bridge_get_req_free_contents(bridge_get_req_t* bgreq);

/*!
 * \brief initialize a bridge_get_req_t based on an external message
 *
 * \param bgreq pointer on the structure to initialize
 * \param msg pointer on the external message to read data from
 *
 * \retval XSUCCESS                     success
 * \retval XERROR                       generic error
 */
int
bridge_get_req_init_from_message(bridge_get_req_t* bgreq,xmessage_t* msg);

/*!
 * \brief create a message based on a bridge_get_req_t
 *
 * \param bgreq pointer on the structure to use
 * \param msg pointer on the external message to write data to
 *
 * \retval XSUCCESS                     success
 * \retval XERROR                       generic error
 */
int
bridge_get_req_create_message(bridge_get_req_t* bgreq,xmessage_t* msg);


/*!
 * \brief initialize a bridge_get_rep_t
 *
 * \param bgrep pointer on the structure to initialize
 * \param used_time time already used in seconds
 * \param usable_time max allowed time in seconds
 * \param halt_time date of an hypothetic halt of the system (0 if not defined)
 *
 * \retval XSUCCESS                     success
 * \retval XERROR                       generic error
 */
int
bridge_get_rep_init(bridge_get_rep_t* bgrep,time_t used_time,time_t usable_time,time_t halt_time);

/*!
 * \brief free previously initialized bridge_get_rep_t contents
 *
 * \param bgrep pointer on the structure to free contents of
 *
 * \retval XSUCCESS                     success
 * \retval XERROR                       generic error
 */
int
bridge_get_rep_free_contents(bridge_get_rep_t* bgrep);


/*!
 * \brief initialize a bridge_get_rep_t based on an external message
 *
 * \param bgrep pointer on the structure to initialize
 * \param msg pointer on the external message to read data from
 *
 * \retval XSUCCESS                     success
 * \retval XERROR                       generic error
 */
int
bridge_get_rep_init_from_message(bridge_get_rep_t* bgrep,xmessage_t* msg);


/*!
 * \brief create a message based on a bridge_get_rep_t
 *
 * \param bgrep pointer on the structure to use
 * \param msg pointer on the external message to write data to
 *
 * \retval XSUCCESS                     success
 * \retval XERROR                       generic error
 */
int
bridge_get_rep_create_message(bridge_get_rep_t* bgrep,xmessage_t* msg);

/*!
 * @}
*/

#endif

/*!
 * @}
*/
