/**
 * \file xmessage.h
 * \author M. Hautreux
 * \date 07/01/2008
 * \brief External Communication message headers
 */
#ifndef __XMESSAGE_H_
#define __XMESSAGE_H_

/*! \addtogroup XTERNAL
 *  @{
 */

/*! \addtogroup XMESSAGE
 *  @{
 */

/*!
 * \enum XMESSAGE_TYPE
 */
enum XMESSAGE_TYPE {

  XPING_REQUEST=0,
  XGET_REQUEST,
  XEND_REQUEST,

  XERROR_REPLY=20,
  XPING_REPLY,
  XGET_REPLY,
  XACK_REPLY

};

/*!
 * \struct xmessage
 * \typedef xmessage_t
 * \brief external stream communication basic element
 */
typedef struct xmessage {
  int type;
  size_t length;
  void* data;
} xmessage_t;

/*!
 * \fn xmessage_init(xmessage_t* msg,int type,char* buffer,size_t length)
 * \brief initialise a xmessage structure
 *
 * \param msg pointer to the structure to initialise
 * \param type type of the message to initialize
 * \param buffer message body
 * \param length length of the message body
 *
 * \retval XSUCCESS operation successfully done
 * \retval XERROR generic error
 *
 */
int
xmessage_init(xmessage_t* msg,int type,char* buffer,size_t length);

/*!
 * \fn xmessage_marshall(xmessage_t* msg,char** pbuffer,size_t* psize)
 * \brief marshall an xmessage
 *
 * \param msg xmessage pointer
 * \param pbuffer pointer on a char buffer that will be allocated and store message data
 * \param psize newly allocated buffer size
 *
 * \retval XSUCCESS operation successfully done
 * \retval XERROR generic error
 *
 */
int
xmessage_marshall(xmessage_t* msg,char** pbuffer,size_t* psize);

/*!
 * \fn xmessage_unmarshall(xmessage_t* msg,char* buffer,size_t size)
 * \brief unmarshall a xmessage
 *
 * \param msg xmessage pointer
 * \param buffer marshalled buffer that represent the message
 * \param size marshalled buffer size
 *
 * \retval XSUCCESS operation successfully done
 * \retval XERROR generic error
 *
 */
int
xmessage_unmarshall(xmessage_t* msg,char* buffer,size_t size);

/*!
 * \fn xmessage_free_contents(xmessage_t* msg)
 * \brief free an xmessage structure contents
 *
 * \param msg pointer to the structure to free its contents
 *
 * \retval XSUCCESS operation successfully done
 * \retval XERROR generic error
 *
 */
int
xmessage_free_contents(xmessage_t* msg);

/*!
 * @}
*/

/*!
 * @}
*/

#endif
