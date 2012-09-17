/*****************************************************************************\
 *  plugins/batch/lsf_slurm/lsf_batch_queue.c - 
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

#include "config.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <time.h>
#include <math.h>

#include <string.h>

#include "bridge/bridge.h"

#include <lsf/lsbatch.h>

#define DEBUG_LOGGER xdebug
#define DEBUG2_LOGGER xdebug2
#define DEBUG3_LOGGER xdebug3
#define ERROR_LOGGER xerror

int init_batch_queue(bridge_batch_manager_t* p_batch_manager,
		     bridge_batch_queue_t* p_batch_queue){
  
  int fstatus=-1;

  if(p_batch_queue!=NULL){

    p_batch_queue->name=NULL; /* batch queue name */
    p_batch_queue->description=NULL; /* More info about this queue */

    p_batch_queue->default_queue=0;

    p_batch_queue->state=BRIDGE_BATCH_QUEUE_STATE_UNKNOWN; /* Queue state (open/closed)*/
    p_batch_queue->activity=BRIDGE_BATCH_QUEUE_ACTIVITY_UNKNOWN; /* Queue activity (active/inactive) */

    p_batch_queue->priority=0; /* resource priority 0=held */

    p_batch_queue->pending_jobs_nb=0; /* number of pending jobs on this queue */
    p_batch_queue->running_jobs_nb=0; /* number of running jobs on this queue */
    p_batch_queue->usersuspended_jobs_nb=0; /* number of user suspended jobs on this queue */
    p_batch_queue->syssuspended_jobs_nb=0; /* number of system suspended jobs on this queue */
    p_batch_queue->jobs_nb=0; /* number of jobs on this queue */
    p_batch_queue->jobs_nb_limit=NO_LIMIT; /* max number of jobs on this queue */

    p_batch_queue->perHost_jobs_nb_limit=NO_LIMIT; /* max number of jobs on this queue per Host */
    p_batch_queue->perUser_jobs_nb_limit=NO_LIMIT; /* max number of jobs allowed to run per user */


    p_batch_queue->seq_time_min=NO_LIMIT; /* sequential time min (seconds) */
    p_batch_queue->seq_time_max=NO_LIMIT; /* sequential time max (seconds) */

    p_batch_queue->seq_mem_min=NO_LIMIT; /* sequential memory min (Mo) */
    p_batch_queue->seq_mem_max=NO_LIMIT; /* sequential memory max (Mo) */

    p_batch_queue->par_time_min=NO_LIMIT; /* parallel time min (seconds) */
    p_batch_queue->par_time_max=NO_LIMIT; /* parallel time max (seconds) */

    p_batch_queue->par_mem_min=NO_LIMIT; /* parallel memory min (Mo) */
    p_batch_queue->par_mem_max=NO_LIMIT; /* parallel memory max (Mo) */

    p_batch_queue->par_cores_nb_min=NO_LIMIT; /* parallel cores min number */
    p_batch_queue->par_cores_nb_max=NO_LIMIT; /* parallel cores max number */

    fstatus=0;
  }

  return fstatus;

}

int clean_batch_queue(bridge_batch_manager_t* p_batch_manager,
		      bridge_batch_queue_t* p_batch_queue){

  int fstatus=-1;

  if(p_batch_queue!=NULL){

    if(p_batch_queue->name!=NULL)
      free(p_batch_queue->name);
    if(p_batch_queue->description!=NULL)
      free(p_batch_queue->description);

    fstatus=0;
  }

  return fstatus;

}



/*
  Get batch queues informations
  -------------------------------
  You don't have to create all bridge_batch_queue_t structure, you just have to set parameters
  according to the following rules :

  if batch_queues_batch_ids equals NULL or "" or "all", get all current queues, otherwise get only batch_queues by
  given batch_id

  if p_batch_queues==NULL :
  - set total batch queues number in p_batch_queues_nb
  - allocate a bridge_batch_queue_t** containing *p_batch_queues_nb bridge_batch_queue_t*
  - fill the *p_batch_queues_nb bridge_batch_queue_t
  else :
  - get max batch queues number in *p_batch_queues_nb
  - fill the *p_batch_queues_nb bridge_batch_queue_t if possible
  - update value of *p_batch_queues_nb according to 


  Returns :
  0 on success
  1 on succes, but p_nodes_nb contains a new valid value for nodes_nb
  -1 on error

  On succes, you 'll have to clean all nodes with bridge_rmi_clean_node(...) before
  freeing *p_nodes

*/
int get_batch_queues(bridge_batch_manager_t* p_batch_manager,
		     bridge_batch_queue_t** p_p_batch_queues,
		     int* p_batch_queues_nb, char* batch_queue_name){

  int fstatus=-1;
  int status;

  int i,j;

  char buffer[256];

  char* queue_array[1];

  char* reason;

  struct queueInfoEnt* p_queueInfo=NULL;
  int queue_nb=0;
  int stored_queue_nb=0;

  queue_array[0]=NULL;
  
  /*
   * Check that batch system is running or exit with error 1
   */
  if(!ls_getclustername()){
    DEBUG3_LOGGER("unable to get cluster informations\n");
    return 1;
  }

  p_queueInfo=lsb_queueinfo(queue_array,&queue_nb,NULL,NULL,ALL_QUEUE);

  if(p_queueInfo==NULL)
    DEBUG3_LOGGER("unable to get queues informations\n");
  else{
    
    if(*p_p_batch_queues!=NULL){
      if(*p_batch_queues_nb<queue_nb)
	queue_nb=*p_batch_queues_nb;
    }
    else{
      *p_p_batch_queues=(bridge_batch_queue_t*)malloc(queue_nb*(sizeof(bridge_batch_queue_t)+1));
      if(*p_p_batch_queues==NULL){
	*p_batch_queues_nb=0;
	queue_nb=*p_batch_queues_nb;
      }
      else{
	*p_batch_queues_nb=queue_nb;
      }
    }
    
    stored_queue_nb=0;

    for(i=0;i<queue_nb;i++){

      if(batch_queue_name!=NULL){
	if(stored_queue_nb==1)
	  break;
	if(strcmp(batch_queue_name,p_queueInfo[i].queue)!=0)
	  continue;
      }

      init_batch_queue(p_batch_manager,(*p_p_batch_queues)+stored_queue_nb);

      /* Queue Name */
      (*p_p_batch_queues)[stored_queue_nb].name=strdup(p_queueInfo[i].queue);

      /* Queue description */
      (*p_p_batch_queues)[stored_queue_nb].description=strdup(p_queueInfo[i].description);

      /* is default queue ? */
      if(p_queueInfo[i].qAttrib & Q_ATTRIB_DEFAULT)
	(*p_p_batch_queues)[stored_queue_nb].default_queue=1;
      else
	(*p_p_batch_queues)[stored_queue_nb].default_queue=0;
      
      /* Queue priority */
      (*p_p_batch_queues)[stored_queue_nb].priority=p_queueInfo[i].priority;

      /* Queue state */
      if(p_queueInfo[i].qStatus & QUEUE_STAT_OPEN){
	(*p_p_batch_queues)[stored_queue_nb].state=BRIDGE_BATCH_QUEUE_STATE_OPENED;
      }
      else{
	(*p_p_batch_queues)[stored_queue_nb].state=BRIDGE_BATCH_QUEUE_STATE_CLOSED;
      }

      /* Queue activity */
      if(p_queueInfo[i].qStatus & QUEUE_STAT_ACTIVE){
	(*p_p_batch_queues)[stored_queue_nb].activity=BRIDGE_BATCH_QUEUE_ACTIVITY_ACTIVE;
      }
      else{
	(*p_p_batch_queues)[stored_queue_nb].activity=BRIDGE_BATCH_QUEUE_ACTIVITY_INACTIVE;
      }

      (*p_p_batch_queues)[stored_queue_nb].pending_jobs_nb=p_queueInfo[i].numPEND;

      (*p_p_batch_queues)[stored_queue_nb].running_jobs_nb=p_queueInfo[i].numRUN;

      (*p_p_batch_queues)[stored_queue_nb].usersuspended_jobs_nb=p_queueInfo[i].numUSUSP;

      (*p_p_batch_queues)[stored_queue_nb].syssuspended_jobs_nb=p_queueInfo[i].numSSUSP;

      (*p_p_batch_queues)[stored_queue_nb].jobs_nb=p_queueInfo[i].numJobs;

      (*p_p_batch_queues)[stored_queue_nb].jobs_nb_limit=(p_queueInfo[i].maxJobs==INT_MAX)?NO_LIMIT:p_queueInfo[i].maxJobs;

      (*p_p_batch_queues)[stored_queue_nb].perHost_jobs_nb_limit=(p_queueInfo[i].hostJobLimit==INT_MAX)?NO_LIMIT:p_queueInfo[i].hostJobLimit;

      (*p_p_batch_queues)[stored_queue_nb].perUser_jobs_nb_limit=(p_queueInfo[i].userJobLimit==INT_MAX)?NO_LIMIT:p_queueInfo[i].userJobLimit;

      // Seq time min
      (*p_p_batch_queues)[stored_queue_nb].seq_time_min=0;

      // Seq time max
      if(p_queueInfo[i].rLimits[LSF_RLIMIT_RUN]>0)
	(*p_p_batch_queues)[stored_queue_nb].seq_time_max=p_queueInfo[i].rLimits[LSF_RLIMIT_RUN];
      else if(p_queueInfo[i].rLimits[LSF_RLIMIT_CPU]>0)
	(*p_p_batch_queues)[stored_queue_nb].seq_time_max=p_queueInfo[i].rLimits[LSF_RLIMIT_CPU];

      // Seq mem min
      (*p_p_batch_queues)[stored_queue_nb].seq_mem_min=0;

      // Seq mem max
      (*p_p_batch_queues)[stored_queue_nb].seq_mem_max=p_queueInfo[i].rLimits[LSF_RLIMIT_RSS]/1024;

      // Par time min
      (*p_p_batch_queues)[stored_queue_nb].par_time_min=0;

      // Par time max
      (*p_p_batch_queues)[stored_queue_nb].par_time_max=(*p_p_batch_queues)[stored_queue_nb].seq_time_max;

      // Par mem min
      (*p_p_batch_queues)[stored_queue_nb].par_mem_min=0;

      // Par mem max
      (*p_p_batch_queues)[stored_queue_nb].par_mem_max=p_queueInfo[i].rLimits[LSF_RLIMIT_RSS]/1024;
      
      // Par proc min number
      (*p_p_batch_queues)[stored_queue_nb].par_cores_nb_min=p_queueInfo[i].minProcLimit;

      // Par proc max number
      (*p_p_batch_queues)[stored_queue_nb].par_cores_nb_max=p_queueInfo[i].procLimit;

      stored_queue_nb++;

    }
    
    fstatus=0;
  }

  if(stored_queue_nb<queue_nb){
    *p_p_batch_queues=(bridge_batch_queue_t*)realloc(*p_p_batch_queues,stored_queue_nb*(sizeof(bridge_batch_queue_t)+1));
    if(*p_p_batch_queues==NULL)
      *p_batch_queues_nb=0;
    else
      *p_batch_queues_nb=stored_queue_nb;
  }

  return fstatus;

}
