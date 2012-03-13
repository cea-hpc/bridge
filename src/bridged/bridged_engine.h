/**
 * \file bridged_engine.h
 * \author M. Hautreux
 * \date 07/01/2008
 * \brief Bridge daemon engine headers
 */
#ifndef __BRIDGED_ENGINE_H_
#define __BRIDGED_ENGINE_H_

/*! \addtogroup BRIDGE_DAEMON
 *  @{
 */

/*! \addtogroup BRIDGED_ENGINE
 *  @{
 */

#ifndef BRIDGED_CONF
#define BRIDGED_CONF "/etc/bridged.conf"
#endif

typedef struct bridged_engine {
  
  char*        config_file;

  char*        address;//!< server hostname or address
  char*        port;//!< server service name or port
  
  char*        cachedir;//!< directory used for temporary data storage
  time_t       timeout; //!< timeout to use during transmission
  
  int          worker_nb;//!< number of worker thread (default is 2, one collector and one request processor)
  int          queue_size;//!< request queue length

  char*        logfile;//!< log messages go to this file (NULL means no log)
  int          loglevel;//!< log level
  char*        debugfile;//!< debug messages go to this file (NULL means no debug)
  int          debuglevel;//!< debug level
  
} bridged_engine_t;

/*!
 * \brief initialize bridged engine structure
 * \internal
 *
 * \param engine pointer on the structure to initialize
 *
 * \param config_file string representing the configuration file
 *
 * \param address address to use
 * \param port port to use
 *
 * \param cachedir directory that contains cached data
 * \param timeout timeout in seconds of communication stages
 *
 * \param logfile file that wil be used to store verbosity
 * \param loglevel verbosity level
 *
 * \param debugfile file that wil be used to store debug data
 * \param debuglevel debug level
 *
 * \param worker_nb quantity of worker threads to launch
 * \param queue_size length of the queue that stores incoming socket 
 *
 * \retval  0 on success
 * \retval -1 on failure
 * \retval -2 on failure due to conf file save
 *  
 */
int
bridged_engine_init(bridged_engine_t* engine,
		    char* config_file,
		    char* address,
		    char* port,
		    char* cachedir,
		    time_t timeout,
		    char* logfile,
		    int loglevel,
		    char* debugfile,
		    int debuglevel,
		    int worker_nb,
		    int queue_size);

/*!
 * \brief initialize bridged engine structure from a configuration file
 * \internal
 *
 * \param engine pointer on the structure to initialize
 *
 * \param conf_file configuration file to use
 *
 * \retval  0 on success
 * \retval -1 on failure
 *  
 */
int
bridged_engine_init_from_config_file(bridged_engine_t* engine,char* conf_file);


/*!
 * \brief free bridged engine structure contents
 * \internal
 *
 * \param engine pointer on the structure to initialize
 *
 * \retval  0 on success
 * \retval -1 on failure
 *  
 */
int
bridged_engine_free_contents(bridged_engine_t* engine);

/*!
 * @}
*/

/*!
 * @}
*/

#endif
