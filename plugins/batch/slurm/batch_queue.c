/*****************************************************************************\
 *  plugins/batch/slurm/batch_queue.c - 
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

#include <string.h>
#include <time.h>

#include "bridge/bridge.h"

#define DEBUG_LOGGER xdebug
#define DEBUG2_LOGGER xdebug2
#define DEBUG3_LOGGER xdebug3
#define ERROR_LOGGER xerror

#include <slurm/slurm.h>

#ifndef JOB_STATE_BASE
#define JOB_STATE_BASE 0x00ff
#endif

int
init_batch_queue(bridge_batch_manager_t* p_batch_manager,
		 bridge_batch_queue_t* p_batch_queue)
{
	int fstatus=-1;

	if(p_batch_queue!=NULL){
		p_batch_queue->name=NULL;
		p_batch_queue->description=NULL;

		p_batch_queue->default_queue=0;

		p_batch_queue->state=BRIDGE_BATCH_QUEUE_STATE_UNKNOWN;
		p_batch_queue->activity=BRIDGE_BATCH_QUEUE_ACTIVITY_UNKNOWN;

		p_batch_queue->priority=0;

		p_batch_queue->pending_jobs_nb=0;
		p_batch_queue->running_jobs_nb=0;
		p_batch_queue->usersuspended_jobs_nb=0;
		p_batch_queue->syssuspended_jobs_nb=0;
		p_batch_queue->jobs_nb=0;
		p_batch_queue->jobs_nb_limit=NO_LIMIT;

		p_batch_queue->perHost_jobs_nb_limit=NO_LIMIT;
		p_batch_queue->perUser_jobs_nb_limit=NO_LIMIT;


		p_batch_queue->seq_time_min=NO_LIMIT;
		p_batch_queue->seq_time_max=NO_LIMIT;
		p_batch_queue->seq_mem_min=NO_LIMIT;
		p_batch_queue->seq_mem_max=NO_LIMIT;

		p_batch_queue->par_time_min=NO_LIMIT;
		p_batch_queue->par_time_max=NO_LIMIT;

		p_batch_queue->par_mem_min=NO_LIMIT;
		p_batch_queue->par_mem_max=NO_LIMIT;

		p_batch_queue->par_cores_nb_min=NO_LIMIT;
		p_batch_queue->par_cores_nb_max=NO_LIMIT;

		fstatus=0;
	}

	return fstatus;
}


int
clean_batch_queue(bridge_batch_manager_t* p_batch_manager,
		  bridge_batch_queue_t* p_batch_queue)
{
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

int
get_batch_queues(bridge_batch_manager_t* p_batch_manager,
		 bridge_batch_queue_t** p_p_batch_queues,
		 int* p_batch_queues_nb, char* batch_queue_name)
{
	int fstatus=-1;

	int i,j;

	int queue_nb=0;
	int stored_queue_nb=0;
	
	bridge_batch_queue_t* bn;

	partition_info_msg_t* ppim;
	partition_info_t* ppi;

	job_info_msg_t* pjim;
	job_info_t* pji;

	node_info_msg_t* pnim;
	node_info_t* pni;

	/* get slurm partition infos */
	if (slurm_load_partitions(0,&ppim,SHOW_ALL) != 0) {
		DEBUG3_LOGGER("unable to get slurm partitions infos");
		ppim=NULL;
		goto exit;
	}

	/* get nodes status */
	if(slurm_load_node(0,&pnim,SHOW_ALL)) {
	        DEBUG3_LOGGER("unable to get nodes informations");
		slurm_free_partition_info_msg(ppim);
		pnim=NULL;
		goto exit;
	}

	/* get slurm job infos */
	if (slurm_load_jobs(0,&pjim,SHOW_ALL) != 0) {
		DEBUG3_LOGGER("unable to get allocations informations");
		slurm_free_partition_info_msg(ppim);
		slurm_free_node_info_msg(pnim);
		goto exit;
	}
	
	/* build/initialize storage structures */
	queue_nb = ppim->record_count;
	if (*p_p_batch_queues != NULL){
		if (*p_batch_queues_nb < queue_nb)
			queue_nb=*p_batch_queues_nb;
	}
	else {
		*p_p_batch_queues = (bridge_batch_queue_t*)
			malloc(queue_nb*(sizeof(bridge_batch_queue_t)+1));
		if (*p_p_batch_queues == NULL){
			*p_batch_queues_nb = 0;
			queue_nb = *p_batch_queues_nb;
		}
		else {
			*p_batch_queues_nb = queue_nb;
		}
	}
	stored_queue_nb=0;

	/* fill queue structures */
	for (i=0;i<ppim->record_count && stored_queue_nb<queue_nb;i++) {
		
		/* get partition pointer */
		ppi=ppim->partition_array+i;
		
		if (ppi->name == NULL)
			continue;
		
		/* queue name filter */
		if (batch_queue_name != NULL &&
		    strcmp(batch_queue_name,ppi->name) != 0)
			continue;
		
		bn = &(*p_p_batch_queues)[stored_queue_nb];
		
		/* put default values */
		init_batch_queue(p_batch_manager,bn);

		/* queue Name */
		bn->name=strdup(ppi->name);

		bn->default_queue = (uint32_t) ( ppi->flags | PART_FLAG_DEFAULT);
		bn->priority = (uint32_t) ppi->priority;

		/* queue activity */
		if(ppi->state_up == PARTITION_UP) {
    		        bn->activity = BRIDGE_BATCH_QUEUE_ACTIVITY_ACTIVE ;
			bn->state = BRIDGE_BATCH_QUEUE_STATE_OPENED ;
		} else if (ppi->state_up == PARTITION_DRAIN) {
    		        bn->activity = BRIDGE_BATCH_QUEUE_ACTIVITY_ACTIVE ;
			bn->state = BRIDGE_BATCH_QUEUE_STATE_CLOSED ;
		} else if (ppi->state_up == PARTITION_DOWN) {
    		        bn->activity = BRIDGE_BATCH_QUEUE_ACTIVITY_INACTIVE ;
			bn->state = BRIDGE_BATCH_QUEUE_STATE_OPENED ;
		} else if (ppi->state_up == PARTITION_INACTIVE) {
    		        bn->activity = BRIDGE_BATCH_QUEUE_ACTIVITY_INACTIVE ;
			bn->state = BRIDGE_BATCH_QUEUE_STATE_CLOSED ;
		} else {
			bn->activity = BRIDGE_BATCH_QUEUE_ACTIVITY_UNKNOWN ;
			bn->state = BRIDGE_BATCH_QUEUE_STATE_UNKNOWN ;
		}

		/* max times */
		if ( ppi->max_time != INFINITE )
		  bn->seq_time_max = (uint32_t) ppi->max_time * 60 ;
		else
		  bn->seq_time_max = NO_LIMIT;
		bn->par_time_max = bn->seq_time_max ;

		/* slurm */
		for ( j=0 ; j < pjim->record_count ; j++ ) {
		  
		  pji=pjim->job_array+j;

		  if ( strcmp(pji->partition,ppi->name) != 0 )
		    continue;

		  switch ( pji->job_state & JOB_STATE_BASE ) {
		  case JOB_PENDING :
		    bn->jobs_nb++;
		    bn->pending_jobs_nb++;
		    break;
		  case JOB_RUNNING :
		    bn->jobs_nb++;
		    bn->running_jobs_nb++;
		    break;
		  case JOB_SUSPENDED :
		    bn->jobs_nb++;
		    bn->syssuspended_jobs_nb++;
		    break;	
		  }

		}

		/* Slurm does not provide information about Min and Max cpus per 
		 * partition. So we use the following method :
		 *
		 * if partition->name ~= /.*_seq/ min=max=1
		 * otherwise, calculate it using MinNodes, MaxNodes and nodes
		 * informations
		 */

		int done = 0 ;
		char * p;
		p = rindex(ppi->name,'_');
		if ( p != NULL ) {
		  if ( strcmp(p+1,"seq") == 0 ) {
		    done = 1;
		    bn->par_cores_nb_min = 1;
		    bn->par_cores_nb_max = 1;
		  }
		}

		if ( ! done ) {
		  /* use partition nodes information to build the min and max */
		  /* number of cores (only min and max nodes number are provided */
		  /* by slurm so we have to build this information) */
		  uint32_t max_cpus_per_node=0;
		  uint32_t min_cpus_per_node=-1;
		  bridge_nodelist_t list1,list2;
		  bridge_nodelist_init(&list1,NULL,0);
		  bridge_nodelist_add_nodes(&list1,ppi->nodes);
		  for ( j=0 ; j < pnim->record_count ; j++ ) {
		    pni=pnim->node_array+j;
		    bridge_nodelist_init(&list2,NULL,0);
		    bridge_nodelist_add_nodes(&list2,pni->name);
		    if(bridge_nodelist_intersects(&list1,&list2)==0) {
		      bridge_nodelist_free_contents(&list2);
		      continue;
		    }
		    if ( pni->cpus > max_cpus_per_node )
		      max_cpus_per_node = pni->cpus ;
		    if ( pni->cpus < min_cpus_per_node )
		      min_cpus_per_node = pni->cpus ;
		    bridge_nodelist_free_contents(&list2);
		  }
		  bridge_nodelist_free_contents(&list1);
		  
		  if ( max_cpus_per_node > 0 && ppi->max_nodes != INFINITE )
		    bn->par_cores_nb_max = max_cpus_per_node * ppi->max_nodes ;
		  if ( min_cpus_per_node < (uint32_t) -1 && ppi->min_nodes > 1 )
		    bn->par_cores_nb_min = min_cpus_per_node * ppi->min_nodes ;
		}
		
		stored_queue_nb++;
	}

	fstatus=0;

	/* free slurm informations */
	slurm_free_job_info_msg(pjim);
	slurm_free_node_info_msg(pnim);
	slurm_free_partition_info_msg(ppim);


	if(stored_queue_nb<queue_nb){
		*p_p_batch_queues=(bridge_batch_queue_t*)
			realloc(*p_p_batch_queues,
				stored_queue_nb*(sizeof(bridge_batch_queue_t)+1));
		if(*p_p_batch_queues==NULL)
			*p_batch_queues_nb=0;
		else
			*p_batch_queues_nb=stored_queue_nb;
	}

exit:

	return fstatus;
}
