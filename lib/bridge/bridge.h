/*****************************************************************************\
 *  lib/bridge/bridge.h - 
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

/* BRIDGE Batch Interface */
#ifndef __BRIDGE_H
#define __BRIDGE_H

#include <stdint.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "bridge_common.h"

#define BRIDGE_VERSION PACKAGE_VERSION

#define INVALID_TIME_VALUE (time_t) 0
#define INVALID_STRING_VALUE "-"
#define INVALID_INTEGER_VALUE (int) -1
#define NO_LIMIT 0

/*! \addtogroup BRIDGE_BATCH_MANAGER
 *  @{
 */
/*!
 * \ingroup BRIDGE_BATCH_MANAGER
 * \brief structure used to store information about batch manager
 */
typedef struct bridge_batch_manager {

  char* type; /*!<
	       * type of batch system used
	       */

  char* version; /*!<
		  * version of batch system used
		  */

  char* cluster; /*!<
	       * name of the `batch cluster` managed by this manager
	       */

  char* description; /*!<
		      * a little description
		      */

  char* master; /*!<
		 * name of the current manager host
		 */

  char** masters_list; /*!<
			* NULL terminated list of potential masters
			*/   

  void* private; /*!<
		  * Private pointer that can be used to store
		  * implementation specific data
		  */

} bridge_batch_manager_t;

/*!
 * @}
*/

/*! \addtogroup BRIDGE_BATCH_SESSION
 *  @{
 */
#define INVALID_SESSIONID 0
#define BRIDGE_BATCH_SESSION_STATE_UNKNOWN 0
#define BRIDGE_BATCH_SESSION_STATE_RUNNING 1
#define BRIDGE_BATCH_SESSION_STATE_PENDING 2
#define BRIDGE_BATCH_SESSION_STATE_SUSPENDED 3
#define BRIDGE_BATCH_SESSION_STATE_DONE 4
#define BRIDGE_BATCH_SESSION_STATE_FAILED 5
/*!
 * \ingroup BRIDGE_BATCH_SESSION
 * \brief structure used to store information about batch sessions ( current or finished )
 */
typedef struct bridge_batch_session {
  char* batch_id;/*!<
		  * ID of this batch session
		  */
  char* rm_id; /*!<
		* ID of the hypothetic underlying resource manager allocation
		*/
  char* name; /*!<
	       * name of the job launched by this batch session 
	       */
  char* description; /*!<
		      * some additional information about this batch session
		      */
  uint32_t state; /*!<
		   * state of this batch session
		   * \par
		   * allowed values are following '# defined' values :
		   * - BRIDGE_BATCH_SESSION_STATE_RUNNING
		   * - BRIDGE_BATCH_SESSION_STATE_PENDING
		   * - BRIDGE_BATCH_SESSION_STATE_SUSPENDED
		   * - BRIDGE_BATCH_SESSION_STATE_DONE
		   * - BRIDGE_BATCH_SESSION_STATE_FAILED
		   * - BRIDGE_BATCH_SESSION_STATE_UNKNOWN
		   */
  char* reason; /*!<
		 * additional information regarding batch session state
		 */
  char* username;/*!<
		  * batch session owner name
		  */
  char* usergroup;/*!<
		   * batch session group name
		   */
  char* projectname;/*!<
		     * batch session associated project name
		     */
  char* submit_hostname;/*!<
			 * Hostname from where batch session was submitted
			 */
  time_t submit_time;/*!<
		      * date of submission ( in seconds since EPOCH)
		      */
  char* queue;/*!<
	       * batch queue name of this batch session
	       */
  char* qos;/*!<
	       * qos  of this batch session
	       */
  uint32_t priority;/*!<
		     * batch session priority
		     */
  char* executing_hostname;/*!<
			    * Hostname on which batch session is running or ran
			    */
  pid_t session_id;/*!<
		    * session id of running batch session
		    */
  time_t start_time;/*!<
		     * batch session start time
		     */
  time_t end_time;/*!<
		   * batch session end time (for finished batch session only)
		   */
  time_t   system_time;/*!<
			* system cpu time used by this batch session
			*/
  time_t   user_time;/*!<
		      * user cpu time used by this batch session
		      */
  uint32_t seq_mem_used;/*!<
			 * memory usage of this batch session (in Mo)
			 */
  time_t   seq_time_limit;/*!<
			   * max running time of this batch session in sequential mode (in seconds)
			   */
  uint32_t seq_mem_limit;/*!<
			  * max memory usage in sequential mode (in Mo)
			  */
  time_t   par_time_limit;/*!<
			   * max ellapsed time in parallel mode of this batch session (in seconds)
			   */
  time_t   par_time_used;/*!<
			  * elapsed time in parallel mode by this batch session
			  */
  uint32_t par_mem_limit;/*!<
			  * max memory usage in parallel mode (in Mo)
			  */
  uint32_t par_cores_nb_limit;/*!<
			      * max number of cores that this batch sessions can use simultaneously in parallel mode
			      */
  uint32_t par_cores_nb;/*!<
			* number of simultaneously used cores in parallel mode
			*/
  uint32_t par_mem_used; /*!<
			  * memory usage of this batch session in parallel mode ( per cpu value )
			  */
  bridge_nodelist_t par_nodelist;/*!<
				   * space separated list of nodes used in parallel mode
				   */
  void* private; /*!<
		  * Private pointer that can be used to store
		  * implementation specific data
		  */

} bridge_batch_session_t;
/*!
 * @}
*/

/*! \addtogroup BRIDGE_BATCH_QUEUE
 *  @{
 */
#define BRIDGE_BATCH_QUEUE_STATE_UNKNOWN 0
#define BRIDGE_BATCH_QUEUE_STATE_OPENED 1
#define BRIDGE_BATCH_QUEUE_STATE_CLOSED 2

#define BRIDGE_BATCH_QUEUE_ACTIVITY_UNKNOWN 0
#define BRIDGE_BATCH_QUEUE_ACTIVITY_ACTIVE 1
#define BRIDGE_BATCH_QUEUE_ACTIVITY_INACTIVE 2
/*!
 * \ingroup BRIDGE_BATCH_QUEUE
 * \brief structure used to store information about available batch queues
 */
typedef struct bridge_batch_queue {

  char* name; /*!< 
	       * batch queue name
	       */
  char* description; /*!< 
		      * some additional information about this batch queue
		      */
  int default_queue; /*!<
		      * default flag (o|no 1|yes)
		      */
  uint32_t state; /*!<
		   * state of this batch queue
		   * \par
		   * allowed values are following '# defined' values :
		   * - BRIDGE_BATCH_QUEUE_STATE_OPENED
		   * - BRIDGE_BATCH_QUEUE_STATE_CLOSED
		   * - BRIDGE_BATCH_QUEUE_STATE_UNKNOWN
		   */
  uint32_t activity; /*!<
		      * activity of this batch queue
		      * \par
		      * allowed values are following '# defined' values :
		      * - BRIDGE_BATCH_QUEUE_ACTIVITY_ACTIVE
		      * - BRIDGE_BATCH_QUEUE_ACTIVITY_INACTIVE
		      * - BRIDGE_BATCH_QUEUE_ACTIVITY_UNKNOWN
		      */

  uint32_t priority; /*!< batch queue priority */

  uint32_t pending_jobs_nb; /*!< number of pending jobs on this queue */
  uint32_t running_jobs_nb; /*!< number of running jobs on this queue */
  uint32_t usersuspended_jobs_nb; /*!< number of user suspended jobs on this queue */
  uint32_t syssuspended_jobs_nb; /*!< number of system suspended jobs on this queue */
  uint32_t jobs_nb; /*!< total number of jobs on this queue */
  uint32_t jobs_nb_limit; /*!< max number of jobs on this queue */

  uint32_t perHost_jobs_nb_limit; /*!< max number of jobs on this queue per Host */
  uint32_t perUser_jobs_nb_limit; /*!< max number of jobs allowed to run per user on this queue */

  time_t seq_time_min; /*!< min cpu time in sequential mode to be allowed to run in this queue (seconds) */
  time_t seq_time_max; /*!< max cpu time in sequential mode to be allowed to run in this queue (seconds) */

  uint32_t seq_mem_min; /*!< min memory usage in sequential mode to be allowed to run in this queue (Mo) */
  uint32_t seq_mem_max; /*!< max memory usage in sequential mode to be allowed to run in this queue (Mo) */

  time_t par_time_min; /*!< min elapsed time in parallel mode to be allowed to run in this queue (seconds) */
  time_t par_time_max; /*!< max elapsed time in parallel mode to be allowed to run in this queue (seconds) */

  uint32_t par_mem_min; /*!< min memory usage in parallel mode to be allowed to run in this queue (Mo) */
  uint32_t par_mem_max; /*!< max memory usage in parallel mode to be allowed to run in this queue (Mo) */

  uint32_t par_cores_nb_min; /*!< max simultaneously used cores nb in parallel mode to be allowed to run in this queue (Mo) */
  uint32_t par_cores_nb_max; /*!< max simultaneously used cores nb in parallel mode to be allowed to run in this queue (Mo) */

  void* private; /*!<
		  * Private pointer that can be used to store
		  * implementation specific data
		  */

} bridge_batch_queue_t;
/*!
 * @}
*/


/*!
 * @}
*/

/*! \addtogroup BRIDGE_BATCH_NODE
 *  @{
 */
#define BRIDGE_BATCH_NODE_STATE_UNKNOWN 0
#define BRIDGE_BATCH_NODE_STATE_OPENED 1
#define BRIDGE_BATCH_NODE_STATE_CLOSED 2
#define BRIDGE_BATCH_NODE_STATE_UNAVAILABLE 3
#define BRIDGE_BATCH_NODE_STATE_UNREACHABLE 4
#define BRIDGE_BATCH_NODE_STATE_UNLICENSED 5
#define BRIDGE_BATCH_NODE_STATE_BUSY 6
/*!
 * \brief structure used to store information about available batch nodes
 */
typedef struct bridge_batch_node {

  char* name; /*!< batch node name */
  char* description; /*!< some additional information about this batch node */

  char* grouplist; /*!< List of groups name that include this node */

  uint32_t state; /*!<
		   * state of this batch node
		   * \par
		   * allowed values are following '# defined' values :
		   * - BRIDGE_BATCH_NODE_STATE_OPENED
		   * - BRIDGE_BATCH_NODE_STATE_CLOSED
		   * - BRIDGE_BATCH_NODE_STATE_UNKNOWN
		   */

  uint32_t running_jobs_nb; /*!< number of running jobs on this node */
  uint32_t usersuspended_jobs_nb; /*!< number of user suspended jobs on this node */
  uint32_t syssuspended_jobs_nb; /*!< number of system suspended jobs on this node */
  uint32_t jobs_nb; /*!< total number of jobs on this node */

  uint32_t jobs_nb_limit; /*!< max number of jobs on this node */
  uint32_t perUser_jobs_nb_limit; /*!< max number of jobs allowed to run per user on this node */

  uint32_t free_swap; /*!< available swap space (in Mo) on this node */
  uint32_t free_tmp; /*!< available tmp space (in Mo) on this node */
  uint32_t free_mem; /*!< available memory space (in Mo) on this node */

  uint32_t total_swap; /*!< max swap space (in Mo) on this node */
  uint32_t total_tmp; /*!< max tmp space (in Mo) on this node */
  uint32_t total_mem; /*!< max memory space (in Mo) on this node */

  float one_min_cpu_load; /*!< one minute cpu load average */
  
  void* private; /*!<
		  * Private pointer that can be used to store
		  * implementation specific data
		  */

} bridge_batch_node_t;

/*!
 * @}
*/

/*! \addtogroup BRIDGE_BATCH_SYSTEM
 *  @{
 */
/*!
 * \ingroup BRIDGE_BATCH_SYSTEM
 * \brief structure used to store information about batch system
 */
typedef struct bridge_batch_system {

  char* name; /*!<
	       * type of batch system used
	       */
  char* lib; /*!<
	      * library file that is used
	      */
  
  void* lib_handle; /*!<
		     * handle on the library
		     */

  int (*get_batch_id) (char** p_id);

  /* batch manager related functions */
  int (*init_batch_manager) (bridge_batch_manager_t * p_batch_manager);
  int (*clean_batch_manager) (bridge_batch_manager_t * p_batch_manager);

  /* batch sessions related functions */
  int (*init_batch_session) (bridge_batch_manager_t * p_batch_manager,
			     bridge_batch_session_t * p_batch_session);
  int (*clean_batch_session) (bridge_batch_manager_t * p_batch_manager,
			      bridge_batch_session_t * p_batch_session);
  int (*get_batch_sessions) (bridge_batch_manager_t * p_batch_manager,
			     bridge_batch_session_t ** p_batch_sessions,
			     int* p_batch_sessions_nb,
			     char* batch_sessions_batch_id,
			     char* jobname,
			     char* username,
			     char* batch_queue,
			     char* execHost,
			     int parallel_infos_flag);
  int (*get_terminated_batch_sessions) (bridge_batch_manager_t * p_batch_manager,
					bridge_batch_session_t ** p_batch_sessions,
					int* p_batch_sessions_nb,
					char* batch_sessions_batch_id,
					char* jobname,
					char* username,
					char* batch_queue,
					char* execHost,
					int parallel_infos_flag,
					time_t begin_eventTime,time_t end_eventTime);

  /* batch queues related functions */
  int (*init_batch_queue)(bridge_batch_manager_t * p_batch_manager,
			  bridge_batch_queue_t * p_batch_queue);
  int (*clean_batch_queue)(bridge_batch_manager_t * p_batch_manager,
			   bridge_batch_queue_t * p_batch_queue);
  int (*get_batch_queues)(bridge_batch_manager_t * p_batch_manager,
			  bridge_batch_queue_t ** p_batch_queues,
			  int* p_batch_queues_nb,
			  char* batch_queue_name);

  /* batch nodes related functions */
  int (*init_batch_node)(bridge_batch_manager_t * p_batch_manager,
			 bridge_batch_node_t* p_batch_node);
  int (*clean_batch_node)(bridge_batch_manager_t * p_batch_manager,
			  bridge_batch_node_t* p_batch_node);
  int (*get_batch_nodes)(bridge_batch_manager_t * p_batch_manager,
			 bridge_batch_node_t** p_batch_nodes,
			 int* p_batch_nodes_nb,
			 char* batch_node_name);

} bridge_batch_system_t ;
/*!
 * @}
*/


/*! \addtogroup BRIDGE_RM_SYSTEM
 * \brief structure used to store information about batch manager
 *  @{
 */
typedef struct bridge_rm_manager {


  char* type; /*!<
	       * type of resource manager used
	       */
  char* version; /*!<
		  * version of resource manager used
		  */

  char* cluster; /*!<
		  * name of the `cluster` managed by this manager
		  */
  char* description; /*!<
		      * a little description
		      */
  char* master; /*!<
		 * name of the current manager host
		 */
  
  char** masters_list; /*!<
			* NULL terminated list of potential masters
			*/

  void* private; /*!<
		  * Private pointer that can be used to store
		  * implementation specific data
		  */

} bridge_rm_manager_t;


#define BRIDGE_RM_ALLOCATION_STATE_PENDING 10
#define BRIDGE_RM_ALLOCATION_STATE_ALLOCATED 20
#define BRIDGE_RM_ALLOCATION_STATE_INUSE     21
#define BRIDGE_RM_ALLOCATION_STATE_SUSPENDED 22
#define BRIDGE_RM_ALLOCATION_STATE_COMPLETED 30
#define BRIDGE_RM_ALLOCATION_STATE_CANCELLED 31
#define BRIDGE_RM_ALLOCATION_STATE_FAILED 32
#define BRIDGE_RM_ALLOCATION_STATE_TIMEOUT 33
#define BRIDGE_RM_ALLOCATION_STATE_NODE_FAILURE 34
#define BRIDGE_RM_ALLOCATION_STATE_UNKNOWN 0

typedef struct bridge_rm_allocation {

  char* id;
  char* name; /* resource name */
  char* description; /* More info about resource */

  char* partition; /* Parition on which resource is executed */

  uint32_t state; /*!<
		   * state of this allocation
		   * \par
		   * allowed values are following '# defined' values :
		   * - BRIDGE_RM_ALLOCATION_STATE_PENDING
		   * - BRIDGE_RM_ALLOCATION_STATE_RUNNING
		   * - BRIDGE_RM_ALLOCATION_STATE_SUSPENDED
		   * - BRIDGE_RM_ALLOCATION_STATE_COMPLETED
		   * - BRIDGE_RM_ALLOCATION_STATE_CANCELLED
		   * - BRIDGE_RM_ALLOCATION_STATE_FAILED
		   * - BRIDGE_RM_ALLOCATION_STATE_TIMEOUT
		   * - BRIDGE_RM_ALLOCATION_STATE_NODE_FAILURE
		   * - BRIDGE_RM_ALLOCATION_STATE_UNKNOWN
		   */

  char* reason; /* reason of this state */

  int priority; /* resource priority 0=held */

  char* username; /* owner name */
  uid_t userid; /* resource owner username*/
  gid_t groupid; /* resource owner groupname*/

  time_t submit_time; /* time of resource request submission */
  time_t start_time; /* time of resource start */
  time_t end_time; /* time of estimated or realized end time of resource*/
  time_t suspend_time; /* if suspended state, contains time of suspension */

  time_t elapsed_time; /* time used by resource */
  time_t allocated_time; /* elapsed_time * cores_nb */

  time_t system_time; /* sum of cores time used by system due to jobs activity */
  time_t user_time; /* sum of cores time used by user due to jobs activity */

  int total_cores_nb; /* Total number of allocated cores */
  int used_cores_nb; /* Number of cores in use by jobs */

  int total_nodes_nb; /* Total number of allocated nodes */
  int used_nodes_nb; /* Number of nodes in use by jobs */

  int memory_usage; /* memory usage per cpu */

  char* allocating_hostname; /* Hostname where resource request was submitted */
  int allocating_session_id; /* Session ID on Hostname where resource request was submitted */
  int allocating_pid; /* Process ID of the allocator */

  char* allocating_session_batchid; /* batchid value of allocating session ( may be invalid if not a batch session ) */

  bridge_nodelist_t nodelist; /* nodes list structure */

  bridge_idlist_t jobidlist; /* IDs of jobs launched by the resource */

} bridge_rm_allocation_t;


#define BRIDGE_RM_NODE_STATE_UNKNOWN "unknown"
#define BRIDGE_RM_NODE_STATE_OUT "out"
#define BRIDGE_RM_NODE_STATE_IN "in"
#define BRIDGE_RM_NODE_ACTIVITY_NOT_RESPONDING "not responding"
#define BRIDGE_RM_NODE_ACTIVITY_DRAINING "draining"
#define BRIDGE_RM_NODE_ACTIVITY_RUNNING "running"
#define BRIDGE_RM_NODE_ACTIVITY_NONE "none"
typedef struct bridge_rm_node {

  char* name; /* Node name */
  char* description; /* More info about node */

  char* state; /* State of node */
  char* reason; /* reason of this state */

  char* activity; /* Node 's activity, running | draining | not responding */

  char* current_partition; /* Node current 's partition */

  int total_cores_nb; /* Total number of cores on the node */
  int active_cores_nb; /* Number of usable cores on this node */
  int used_cores_nb; /* Number of already in use cores */

  char* cpumask; /* info about which Cores are used */

  int total_memory_size; /* Memory space available on this node (Mo) */
  int used_memory_size; /* Memory space already in use (Mo) */

  int total_swap_size; /* Swap space available on this node (Mo) */
  int used_swap_size; /* Swap space already in use (Mo)*/

  int total_tmp_size; /* Temporary space available on this node (Mo) */
  int used_tmp_size;/* Temporary space already in use (Mo)*/

  int swapped_in_pages_nb; /* number of pages per second swapped in */
  int swapped_out_pages_nb; /* number of pages per second swapped out */

  time_t boot_time;

  float one_min_load_average;
  float five_min_load_average;
  float fifteen_min_load_average;
  
} bridge_rm_node_t;

#define BRIDGE_RM_PARTITION_STATE_UNKNOWN  0
#define BRIDGE_RM_PARTITION_STATE_BLOCKED  5
#define BRIDGE_RM_PARTITION_STATE_IN      10
#define BRIDGE_RM_PARTITION_STATE_OUT     20
#define BRIDGE_RM_PARTITION_STATE_DRAIN   30
typedef struct bridge_rm_partition {

  char* name; /* Node name */
  char* description; /* More info about node */

  int state; /* State of the partition */
  char* reason; /* reason of this state */

  int total_cores_nb; /* Total number of cores in the partition */
  int active_cores_nb; /* Number of usable cores in the partition */
  int used_cores_nb; /* Number of already in use cores in the partition */

  int total_nodes_nb; /* Total number of nodes in the partition */
  int active_nodes_nb; /* Number of usable nodes in the partition */
  int used_nodes_nb; /* Number of already in use nodes in the partition */

  time_t time_limit; /* Default max elapsed time available for resources
			0 stands for no limit */
  int memory_limit; /* Default max memory space available for resources on a node per cpu (Mo)
			    0 stands for no limit */

  time_t start_time; /* Start time of the partition 
			0 stands for unknown */

  bridge_nodelist_t total_nodelist;
  bridge_nodelist_t active_nodelist;
  bridge_nodelist_t used_nodelist;
  
} bridge_rm_partition_t;

/*!
 * @}
*/

/*! \addtogroup BATCH_SYSTEM
 *  @{
 */
/*!
 * \ingroup BATCH_SYSTEM
 * \brief structure used to store information about resource management system
 */
typedef struct bridge_rm_system {

  char* name; /*!<
	       * type of rm system used
	       */
  char* lib; /*!<
	      * library file that is used
	      */
  
  void* lib_handle; /*!<
		     * handle on the library
		     */

  int (*get_rm_id) (char** p_id);
  
  /* rm manager related functions */
  int (*init_rm_manager) (bridge_rm_manager_t * p_rm_manager);
  int (*clean_rm_manager) (bridge_rm_manager_t * p_rm_manager);
  
  /* rm allocations related functions */
  int (*init_rm_allocation) (bridge_rm_manager_t * p_rm_manager,
			     bridge_rm_allocation_t * p_rm_allocation);
  int (*clean_rm_allocation) (bridge_rm_manager_t * p_rm_manager,
			      bridge_rm_allocation_t * p_rm_allocation);
  int (*get_rm_allocations) (bridge_rm_manager_t * p_rm_manager,
			     bridge_rm_allocation_t ** p_rm_allocations,
			     int* p_rm_allocations_nb,
			     char* rm_allocations_rm_id,
			     char* username,
			     char* rm_partition,
			     char* intersectingNodes,
			     char* includingNodes);
  int (*get_terminated_rm_allocations) (bridge_rm_manager_t * p_rm_manager,
					bridge_rm_allocation_t ** p_rm_allocations,
					int* p_rm_allocations_nb,
					char* rm_allocations_rm_id,
					char* username,
					char* rm_partition,
					char* intersectingNodes,
					char* includingNodes,
					time_t begin_eventTime,time_t end_eventTime);

  /* rm partitions related functions */
  int (*init_rm_partition)(bridge_rm_manager_t * p_rm_manager,
			   bridge_rm_partition_t * p_rm_partition);
  int (*clean_rm_partition)(bridge_rm_manager_t * p_rm_manager,
			    bridge_rm_partition_t * p_rm_partition);
  int (*get_rm_partitions)(bridge_rm_manager_t * p_rm_manager,
			   bridge_rm_partition_t ** p_rm_partitions,
			   int* p_rm_partitions_nb,
			   char* rm_partition_name,
			   char* intersectingNodes,
			   char* includingNodes);

  /* rm nodes related functions */
  int (*init_rm_node)(bridge_rm_manager_t * p_rm_manager,
		      bridge_rm_node_t* p_rm_node);
  int (*clean_rm_node)(bridge_rm_manager_t * p_rm_manager,
		       bridge_rm_node_t* p_rm_node);
  int (*get_rm_nodes)(bridge_rm_manager_t * p_rm_manager,
		      bridge_rm_node_t** p_rm_nodes,
		      int* p_rm_nodes_nb,
		      char* rm_node_name);

} bridge_rm_system_t ;


/*! \addtogroup BRIDGE_MANAGER
 *  @{
 */
#ifndef BRIDGE_CONF
#define BRIDGE_CONF "/etc/bridge.conf"
#endif

#define BRIDGE_CONF_FILE_ENV_VAR "BRIDGE_CONF"

#define BRIDGE_BS_RM_BINDING_NONE           0
#define BRIDGE_BS_RM_BINDING_BATCHID        1
#define BRIDGE_BS_RM_BINDING_EXECHOST_SID   2
#define BRIDGE_BS_RM_BINDING_RMID           3

/*!
 * \ingroup BRIDGE_MANAGER
 * \brief structure used to store information about batch system
 */
typedef struct bridge_manager {

  char* plugins_dir;

  int batch_system_flag;
  int batch_manager_flag;
  char* batch_system_name;
  char* batch_system_lib;
  bridge_batch_system_t batch_system;
  bridge_batch_manager_t batch_manager;

  int rm_system_flag;
  int rm_manager_flag;
  char* rm_system_name;
  char* rm_system_lib;
  bridge_rm_system_t rm_system;
  bridge_rm_manager_t rm_manager;


  FILE* logfile;
  FILE* debugfile;
  FILE* errorfile;

  int loglevel;
  int debuglevel;
  int errorlevel;

  int bs_rm_binding;

  char* version;

} bridge_manager_t ;

/*!
 * @}
*/

/*!
 * \ingroup BRIDGE_MANAGER
 * \brief Give bridge version
 *
 * \retval version of bridge curently in use
 * \retval NULL if operation failed
*/
char* bridge_version();

/*!
 * \ingroup BRIDGE_MANAGER
 * \brief Initialise bridge manager
 *
 * \param p_manager pointer on a bridge manager structure to initialize
 *
 * \retval  0 operation successfully done
 * \retval -1 operation failed
*/
int bridge_init_manager(bridge_manager_t* p_manager);

/*!
 * \ingroup BRIDGE_MANAGER
 * \brief Clean bridge manager
 *
 * \param p_manager pointer on a bridge manager structure to clean
 *
 * \retval  0 operation successfully done
 * \retval -1 operation failed
*/
int bridge_clean_manager(bridge_manager_t* p_manager);

/*!
 * \ingroup BRIDGE_BATCH_SYSTEM
 * \brief Get batch ID of current job
 *
 * \param p_manager pointer on a bridge manager structure
 * \param p_id pointer on a char* to create with current job ID
 *
 * \retval  0 operation successfully done
 * \retval -1 no batch system loaded in bridge manager
 * \retval  * batch system plugin function return value
*/
int bridge_get_batch_id(bridge_manager_t * p_manager,char** p_id);

/*!
 * \ingroup BRIDGE_BATCH_SYSTEM
 * \brief Get active batch sessions information
 *
 * \par Informations
 * \par
 * You don't have to create all bridge_batch_session_t structures, you just have to set parameters
 * according to the following rules :
 * \par
 * if p_batch_sessions==NULL :
 *  - set total batch sessions number in p_batch_sessions_nb
 *  - allocate a bridge_batch_session_t** containing *p_batch_sessions_nb bridge_batch_session_t*
 *  - fill the *p_batch_sessions_nb bridge_batch_session_t
 * \par
 * else :
 *  - get max batch sessions number in *p_batch_sessions_nb
 *  - fill the *p_batch_sessions_nb bridge_batch_session_t if possible
 *  - update value of *p_batch_sessions_nb if needed 
 *
 * \param p_manager pointer on a bridge manager structure
 * \param p_batch_sessions pointer on a uninitialized batch session array to build
 * \param p_batch_sessions_nb int pointer for retrieved batch sessions quantity storage
 * \param batch_sessions_batch_id required session id
 * \param name required sessions name
 * \param username user owning the required sessions
 * \param batch_queue name of the queue that dispatch the required sessions
 * \param execHost name of the host that run the required sessions
 * \param rm_infos_flag indicates that rm system information must be linked using configured binding
 *
 * \retval  0 operation successfully done
 * \retval -1 no batch system loaded in bridge manager (or rm system)
 * \retval  * batch system plugin function return value
*/
int bridge_get_batch_sessions(bridge_manager_t* p_manager,
			      bridge_batch_session_t ** p_batch_sessions,
			      int* p_batch_sessions_nb,
			      char* batch_sessions_batch_id,
			      char* name,
			      char* username,
			      char* batch_queue,
			      char* execHost,
			      int rm_infos_flag);

/*!
 * \ingroup BRIDGE_BATCH_SYSTEM
 * \brief Get terminated batch sessions information
 *
 * \par Informations
 * \par
 * You don't have to create all bridge_batch_session_t structures, you just have to set parameters
 * according to the following rules :
 * \par
 * if p_batch_sessions==NULL :
 *  - set total batch sessions number in p_batch_sessions_nb
 *  - allocate a bridge_batch_session_t** containing *p_batch_sessions_nb bridge_batch_session_t*
 *  - fill the *p_batch_sessions_nb bridge_batch_session_t
 * \par
 *  else :
 *  - get max batch sessions number in *p_batch_sessions_nb
 *  - fill the *p_batch_sessions_nb bridge_batch_session_t if possible
 *  - update value of *p_batch_sessions_nb if needed
 *
 * \param p_manager pointer on a bridge manager structure
 * \param p_batch_sessions pointer on a uninitialized batch session array to build
 * \param p_batch_sessions_nb int pointer for retrieved batch sessions quantity storage
 * \param batch_sessions_batch_id required session id
 * \param name required sessions name
 * \param username user owning the required sessions
 * \param batch_queue name of the queue that dispatched the required sessions
 * \param execHost name of the host that ran the required sessions
 * \param rm_infos_flag indicates that rm system information must be linked using configured binding
 * \param begin_eventTime required sessions have an end time greater than  or equal to date (seconds since epoch)
 * \param end_eventTime required sessions have an end time lower than date (seconds since epoch)
 *
 * \retval  0 operation successfully done
 * \retval -1 no rm system loaded in bridge manager
 * \retval  * resource manager plugin function return value
*/
int bridge_get_terminated_batch_sessions(bridge_manager_t* p_manager,
					 bridge_batch_session_t ** p_batch_sessions,
					 int* p_batch_sessions_nb,
					 char* batch_sessions_batch_id,
					 char* name,
					 char* username,
					 char* batch_queue,
					 char* execHost,
					 int rm_infos_flag,
					 time_t begin_eventTime,time_t end_eventTime);

/*!
 * \ingroup BRIDGE_BATCH_SYSTEM
 * \brief Clean a batch session structure
 *
 * \param p_manager pointer on a bridge manager structure
 * \param p_batch_session pointer on a batch session structure to clean
 *
 * \retval  0 operation successfully done
 * \retval -1 no batch system loaded in bridge manager
 * \retval  * resource manager plugin function return value
 */
int bridge_clean_batch_session(bridge_manager_t* p_manager,
			       bridge_batch_session_t * p_batch_session);

int bridge_get_rm_allocations(bridge_manager_t * p_manager,
				  bridge_rm_allocation_t ** p_rm_allocations,
				  int* p_rm_allocations_nb,
				  char* rm_allocations_rm_id,
				  char* jobname,
				  char* username,
				  char* rm_partition,
				  char* execHost);

/*!
 * \ingroup BRIDGE_BATCH_SYSTEM
 * \brief Get batch queues
 *
 * \param p_manager pointer on a bridge manager structure
 * \param p_batch_queues pointer on a uninitialized batch queue array to build
 * \param p_batch_queues_nb int pointer for retrieved batch queues quantity storage
 * \param batch_queue_name required batch queue 
 *
 * \retval  0 operation successfully done
 * \retval -1 no batch system loaded in bridge manager (or rm system)
 * \retval  * resource manager plugin function return value
*/
int bridge_get_batch_queues(bridge_manager_t* p_manager,
			    bridge_batch_queue_t ** p_batch_queues,
			    int* p_batch_queues_nb,
			    char* batch_queue_name);

/*!
 * \ingroup BRIDGE_BATCH_SYSTEM
 * \brief Destroy batch queue
 *
 * \param p_manager pointer on a bridge manager structure
 * \param p_batch_queue pointer on a batch queue structure to clean
 *
 * \retval  0 operation successfully done
 * \retval -1 no batch system loaded in bridge manager
 * \retval  * resource manager plugin function return value
 */
int bridge_clean_batch_queue(bridge_manager_t* p_manager,
			     bridge_batch_queue_t * p_batch_queue);

/*!
 * \ingroup BRIDGE_BATCH_SYSTEM
 * \brief Get batch nodes
 *
 * \param p_manager pointer on a bridge manager structure
 * \param p_batch_nodes pointer on a uninitialized batch node array to build
 * \param p_batch_nodes_nb int pointer for retrieved batch nodes quantity storage
 * \param batch_node_name required batch node 
 *
 * \retval  0 operation successfully done
 * \retval -1 no batch system loaded in bridge manager (or rm system)
 * \retval  * resource manager plugin function return value
*/
int bridge_get_batch_nodes(bridge_manager_t* p_manager,
			   bridge_batch_node_t ** p_batch_nodes,
			   int* p_batch_nodes_nb,
			   char* batch_node_name);

/*!
 * \ingroup BRIDGE_BATCH_SYSTEM
 * \brief Destroy batch node
 *
 * \param p_manager pointer on a bridge manager structure
 * \param p_batch_node pointer on a batch node structure to clean
 *
 * \retval  0 operation successfully done
 * \retval -1 no batch system loaded in bridge manager
 * \retval  * resource manager plugin function return value
 */
int bridge_clean_batch_node(bridge_manager_t* p_manager,
			    bridge_batch_node_t * p_batch_node);


/*!
 * \ingroup BRIDGE_RM_SYSTEM
 * \brief Get rm ID of current job
 *
 * \param p_manager pointer on a bridge manager structure
 * \param p_id pointer on a char* to create with current job ID
 *
 * \retval  0 operation successfully done
 * \retval -1 no rm system loaded in bridge manager
 * \retval  * resource manager plugin function return value
*/
int bridge_get_rm_id(bridge_manager_t * p_manager, char** p_id);

/*!
 * \ingroup BRIDGE_RM_SYSTEM
 * \brief Get rm allocations
 *
 * \param p_manager pointer on a bridge manager structure
 * \param p_rm_allocations pointer on a uninitialized rm allocation array to build
 * \param p_rm_allocations_nb int pointer for retrieved rm allocations quantity storage
 * \param rm_allocations_rm_id required allocation id
 * \param username user owning required allocations
 * \param rm_partition name of the partition to get
 * \param intersectingNodes list of nodes that partitions need to intersect to be retrieved
 * \param includingNodes list of nodes that partitions need to include to be retrieved
 *
 * \retval  0 operation successfully done
 * \retval -1 no rm system loaded in bridge manager
 * \retval  * resource manager plugin function return value
*/
int bridge_get_rm_allocations(bridge_manager_t * p_manager,
			      bridge_rm_allocation_t ** p_rm_allocations,
			      int* p_rm_allocations_nb,
			      char* rm_allocations_rm_id,
			      char* username,
			      char* rm_partition,
			      char* intersectingNodes,
			      char* includingNodes);

/*!
 * \ingroup BRIDGE_RM_SYSTEM
 * \brief Get rm allocations
 *
 * \param p_manager pointer on a bridge manager structure
 * \param p_rm_allocations pointer on a uninitialized rm allocation array to build
 * \param p_rm_allocations_nb int pointer for rm allocations quantity storage
 * \param rm_allocations_rm_id list of the required allocation ids
 * \param username user owning required allocations
 * \param rm_partition name of the partition to get
 * \param intersectingNodes list of nodes that partitions need to intersect to be retrieved
 * \param includingNodes list of nodes that partitions need to include to be retrieved
 * \param begin_eventTime required allocations have an end time greater than  or equal to date (seconds since epoch)
 * \param end_eventTime required allocations have an end time lower than date (seconds since epoch)
 * \retval  0 operation successfully done
 * \retval -1 no rm system loaded in bridge manager
 * \retval  * resource manager plugin function return value
*/
int bridge_get_terminated_rm_allocations(bridge_manager_t * p_manager,
					 bridge_rm_allocation_t ** p_rm_allocations,
					 int* p_rm_allocations_nb,
					 char* rm_allocations_rm_id,
					 char* username,
					 char* rm_partition,
					 char* intersectingNodes,
					 char* includingNodes,
					 time_t begin_eventTime,time_t end_eventTime);

/*!
 * \ingroup BRIDGE_RM_SYSTEM
 * \brief Destroy rm allocation
 *
 * \param p_manager pointer on a bridge manager structure
 * \param p_rm_allocation pointer on a rm allocation structure to clean
 *
 * \retval  0 operation successfully done
 * \retval -1 no rm system loaded in bridge manager
 * \retval  * resource manager plugin function return value
 */
int bridge_clean_rm_allocation(bridge_manager_t* p_manager,
			       bridge_rm_allocation_t * p_rm_allocation);

/*!
 * \ingroup BRIDGE_RM_SYSTEM
 * \brief Get rm partitions
 *
 * \param p_manager pointer on a bridge manager structure
 * \param p_rm_partitions pointer on a uninitialized rm partition array to build
 * \param p_rm_partitions_nb int pointer for rm partitions quantity storage
 * \param rm_partition name of the partition to get
 * \param intersectingNodes list of nodes that partitions need to intersect to be retrieved
 * \param includingNodes list of nodes that partitions need to include to be retrieved
 *
 * \retval  0 operation successfully done
 * \retval -1 no rm system loaded in bridge manager
 * \retval  * resource manager plugin function return value
*/
int bridge_get_rm_partitions(bridge_manager_t * p_manager,
			     bridge_rm_partition_t ** p_rm_partitions,
			     int* p_rm_partitions_nb,
			     char* rm_partition,
			     char* intersectingNodes,
			     char* includingNodes);

/*!
 * \ingroup BRIDGE_RM_SYSTEM
 * \brief Destroy rm partition
 *
 * \param p_manager pointer on a bridge manager structure
 * \param p_rm_partition pointer on a rm partition structure to clean
 *
 * \retval  0 operation successfully done
 * \retval -1 no rm system loaded in bridge manager
 * \retval  * resource manager plugin function return value
 */
int bridge_clean_rm_partition(bridge_manager_t* p_manager,
			      bridge_rm_partition_t * p_rm_partition);

#endif
