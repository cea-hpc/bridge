/*****************************************************************************\
 *  plugins/batch/torque/batch_queue.c - 
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

#include "batch_common.h"

#include <pbs_error.h>
#include <pbs_ifl.h>
extern char *pbs_server;

#define DEBUG_LOGGER xdebug
#define DEBUG2_LOGGER xdebug2
#define DEBUG3_LOGGER xdebug3
#define ERROR_LOGGER xerror

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

	int con_fd;
	struct batch_status * mstat;
	struct batch_status * nstat;
	struct batch_status * cstat;
	struct attrl *attr;

	int enabled=0,started=0;

	char* default_queue = NULL ;

	int queue_nb=0;
	int stored_queue_nb=0;
	
	bridge_batch_queue_t* bn;

	con_fd = *((int*)p_batch_manager->private);

	/* get default queue name */
	mstat = pbs_statserver(con_fd,NULL,NULL);
	if ( mstat == NULL ) 
		return fstatus;
	attr = mstat->attribs ;
	while ( attr != NULL ) {
		if (!strcmp(attr->name, ATTR_dfltque))
		{
			default_queue = strdup(attr->value);
		}
		attr = attr->next ;
	}
	pbs_statfree(mstat);

	/* get queue stats */
	nstat = pbs_statque(con_fd,batch_queue_name,NULL,NULL);
	if ( nstat == NULL ) 
		return fstatus;

	/* count queues quantity */
	cstat = nstat ;
	while ( cstat != NULL ) {
		queue_nb++;
		//print_batch_status(cstat);
		cstat=cstat->next;
	}
	
	/* initialized output buffer if required */
	if(*p_p_batch_queues!=NULL){
		if(*p_batch_queues_nb<queue_nb)
			queue_nb=*p_batch_queues_nb;
	}
	else{
		*p_p_batch_queues=(bridge_batch_queue_t*)
			malloc(queue_nb*(sizeof(bridge_batch_queue_t)+1));
		if(*p_p_batch_queues==NULL){
			*p_batch_queues_nb=0;
			queue_nb=*p_batch_queues_nb;
		}
		else{
			*p_batch_queues_nb=queue_nb;
		}
	}
	stored_queue_nb=0;

	/* fill output buffer with stats */
	cstat = nstat ;
	while ( cstat != NULL && queue_nb ) {

		bn = &(*p_p_batch_queues)[stored_queue_nb];

		/* queue name filter */
		if(batch_queue_name!=NULL){
			if(strcmp(batch_queue_name,
				  cstat->name)!=0)
				continue;
		}

		/* put default values */
		init_batch_queue(p_batch_manager,bn);

		/* queue Name */
		bn->name=strdup(cstat->name);

		attr = cstat->attribs ;
		while ( attr != NULL ) {
			if (!strcmp(attr->name, ATTR_enable))
			{
				if (!strcmp(attr->value,"True"))
					enabled=1;
				else if (!strcmp(attr->value,"False"))
					enabled=0;
				else
					started=-1;
			}
			else if (!strcmp(attr->name, ATTR_start))
			{
				if (!strcmp(attr->value,"True"))
					started=1;
				else if (!strcmp(attr->value,"False"))
					started=0;
				else
					started=-1;
			}
			else if (!strcmp(attr->name, ATTR_p)) {
				sscanf(attr->value,"%d",&(bn->priority));
			}
			else if (!strcmp(attr->name, ATTR_total)) {
				sscanf(attr->value,"%u",&(bn->jobs_nb));
			}
			else if (!strcmp(attr->name, ATTR_rescmax)) {
				if (!strcmp(attr->resource,"nodes")) {
					sscanf(attr->value,"%d",
					       &(bn->par_cores_nb_max));
				}
				else if (!strcmp(attr->resource,"pmem")) {
					convert_mem(attr->value,
						    &(bn->par_mem_max));
					bn->seq_mem_max = bn->par_mem_max;
				}
				else if (!strcmp(attr->resource,"walltime")) {
					convert_time(attr->value,
						     &(bn->par_time_max));
					bn->seq_time_max = bn->par_time_max ;
				}
			}
			else if (!strcmp(attr->name, ATTR_rescmin)) {
				if (!strcmp(attr->resource,"nodes")) {
					sscanf(attr->value,"%d",
					       &(bn->par_cores_nb_min));
				}
				else if (!strcmp(attr->resource,"pmem")) {
					convert_mem(attr->value,
						    &(bn->par_mem_min));
					bn->seq_mem_min = bn->par_mem_min;
				}
				else if (!strcmp(attr->resource,"walltime")) {
					convert_time(attr->value,
						     &(bn->par_time_min));
					bn->seq_time_min = bn->par_time_min ;
				}
			}
			else if (!strcmp(attr->name, ATTR_count)) {
				int trans,que,held,wait,run,exiting;
				if ( sscanf(attr->value,"Transit:%d Queued:%d"
					    " Held:%d Waiting:%d Running:%d "
					    "Exiting:%d",&trans,&que,&held,
					    &wait,&run,&exiting) == 6 ) {

					bn->running_jobs_nb = run + exiting ;
					bn->pending_jobs_nb = trans + que + wait ;
					bn->syssuspended_jobs_nb = held ;

				}
			}

			attr = attr->next ;
		}

		if ( enabled == 1 )
			bn->state = BRIDGE_BATCH_QUEUE_STATE_OPENED ;
		else if ( enabled == 0 ) 
			bn->state = BRIDGE_BATCH_QUEUE_STATE_CLOSED ;
		else
			bn->state = BRIDGE_BATCH_QUEUE_STATE_UNKNOWN ;

		if ( started == 1 )
			bn->activity = BRIDGE_BATCH_QUEUE_ACTIVITY_ACTIVE ;
		else if ( started == 0 ) 
			bn->activity = BRIDGE_BATCH_QUEUE_ACTIVITY_INACTIVE ;
		else
			bn->activity = BRIDGE_BATCH_QUEUE_ACTIVITY_UNKNOWN ;

		if (!strcmp(bn->name,default_queue))
			bn->default_queue=1;

		cstat=cstat->next;
		queue_nb--;
		stored_queue_nb++;
	}
	pbs_statfree(nstat);


	if ( default_queue != NULL )
		free(default_queue);

	fstatus=0;

	if(stored_queue_nb<queue_nb){
		*p_p_batch_queues=(bridge_batch_queue_t*)
			realloc(*p_p_batch_queues,
				stored_queue_nb*(sizeof(bridge_batch_queue_t)+1));
		if(*p_p_batch_queues==NULL)
			*p_batch_queues_nb=0;
		else
			*p_batch_queues_nb=stored_queue_nb;
	}

	return fstatus ;
}
