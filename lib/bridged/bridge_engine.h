/**
 * \file bridge_engine.h
 * \author M. Hautreux
 * \date 07/28/2008
 * \brief Bridge Engine headers
 */
#ifndef __BRIDGE_ENGINE_H_
#define __BRIDGE_ENGINE_H_

/*! \addtogroup BRIDGE_API
 *  @{
 */

/*! \addtogroup BRIDGE_ENGINE
 *  @{
 */

#ifndef BRIDGEDAPI_CONF
#define BRIDGEDAPI_CONF "/etc/bridgedapi.conf"
#endif

/*!
 * \struct bridge_engine
 * \typedef bridge_engine_t
 * \brief bridge client engine
 */
typedef struct bridge_engine {

  char*        primary_address;//!< primary bridge server hostname or address
  char*        primary_port;//!< primary bridge server hservice name or port

  char*        secondary_address;//!< secondary bridge server hostname or address
  char*        secondary_port;//!< secondary bridge server hservice name or port

  int          retries;//!< max retries number per server
  time_t       timeout;//!< communication timeout

  char*        logfile;//!< log messages go to this file (NULL means no log)
  int          loglevel;//!< log level
  char*        debugfile;//!< debug messages go to this file (NULL means no debug)
  int          debuglevel;//!< debug level

} bridge_engine_t;


/*!
 * \brief initialize bridge engine structure
 * \internal
 *
 * \param engine pointer on the structure to initialize
 *
 * \param primary_address address of the primary Bridge server
 * \param primary_port port of the primary Bridge server
 * \param primary_principal kerberos V principal of the primary Bridge server
 * \param primary_keytab file that contain the kerberos keytab of the primary Bridge server
 *
 * \param secondary_address address of the secondary Bridge server
 * \param secondary_port port of the secondary Bridge server
 * \param secondary_principal kerberos V principal of the secondary Bridge server
 * \param secondary_keytab file that contain the kerberos keytab of the secondary Bridge server
 *
 * \param timeout timeout in seconds of communication stages
 *
 * \param logfile file that wil be used to store verbosity
 * \param loglevel verbosity level
 *
 * \param debugfile file that wil be used to store debug data
 * \param debuglevel debug level
 *
 * \retval XSUCCESS success
 * \retval XERROR generic error
 *  
 */
int
bridge_engine_init(bridge_engine_t* engine,
		   char* primary_address,
		   char* primary_port,
		   char* secondary_address,
		   char* secondary_port,
		   time_t timeout,
		   char* logfile,
		   int loglevel,
		   char* debugfile,
		   int debuglevel);

/*!
 * \brief initialize bridge engine structure \a engine based on configuration elements of file \a conf_file
 * \internal
 *
 * \param engine pointer on the structure to initialize
 * \param conf_file configuration file to use
 *
 * \retval XSUCCESS success
 * \retval XERROR generic error
 *  
 */
int
bridge_engine_init_from_config_file(bridge_engine_t* engine,char* conf_file);


/*!
 * \brief free bridge engine structure contents
 * \internal
 *
 * \param engine pointer on the structure to destroy
 *
 * \retval XSUCCESS success
 * \retval XERROR generic error
 *  
 */
int
bridge_engine_free_contents(bridge_engine_t* engine);

/*!
 * @}
*/

/*!
 * @}
*/


#endif
