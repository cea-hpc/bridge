/*****************************************************************************\
 *  plugins/batch/torque/batch_session.c - 
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

#include <pbs_error.h>
#include <pbs_ifl.h>
extern char *pbs_server;

#define DEBUG_LOGGER xdebug
#define DEBUG2_LOGGER xdebug2
#define DEBUG3_LOGGER xdebug3
#define ERROR_LOGGER xerror

#define DEFAULT_PAR_CPUS_NB_LIMIT 0

int
init_batch_session(bridge_batch_manager_t* p_batch_manager,
		   bridge_batch_session_t* p_batch_session)
{
	int fstatus=-1;

	if(p_batch_session!=NULL){

		p_batch_session->batch_id=NULL;
		p_batch_session->rm_id=NULL;
		p_batch_session->name=NULL;
		p_batch_session->description=NULL;

		p_batch_session->state=BRIDGE_BATCH_SESSION_STATE_UNKNOWN;
		p_batch_session->reason=NULL;

		p_batch_session->username=NULL;
		p_batch_session->usergroup=NULL;
		p_batch_session->projectname=NULL;

		p_batch_session->submit_hostname=NULL;
		p_batch_session->submit_time=INVALID_TIME_VALUE;

		p_batch_session->qos=NULL;
		p_batch_session->queue=NULL;
		p_batch_session->priority=0;

		p_batch_session->executing_hostname=NULL;
		p_batch_session->session_id=0;
		p_batch_session->start_time=INVALID_TIME_VALUE;
		p_batch_session->end_time=INVALID_TIME_VALUE;

		/* init resource usage*/
		p_batch_session->system_time=0;
		p_batch_session->user_time=0;
		p_batch_session->seq_mem_used=0;

		/* No limit by default */
		p_batch_session->seq_time_limit=0;
		p_batch_session->par_time_limit=0;
		p_batch_session->seq_mem_limit=0;
		p_batch_session->par_mem_limit=0;
    
		/* Default to sequential job only */
		p_batch_session->par_cores_nb_limit=DEFAULT_PAR_CPUS_NB_LIMIT;

		p_batch_session->par_time_used=0;
		p_batch_session->par_mem_used=0;
		p_batch_session->par_cores_nb=0;

		bridge_nodelist_init(&(p_batch_session->par_nodelist),NULL,0);

		fstatus=0;
	}

	return fstatus;

}

int
clean_batch_session(bridge_batch_manager_t* p_batch_manager,
		    bridge_batch_session_t* p_batch_session)
{
	int fstatus=-1;

	if(p_batch_session!=NULL){

		if(p_batch_session->batch_id!=NULL)
			free(p_batch_session->batch_id);
		if(p_batch_session->rm_id!=NULL)
			free(p_batch_session->rm_id);
		if(p_batch_session->name!=NULL)
			free(p_batch_session->name);

		if(p_batch_session->description!=NULL)
			free(p_batch_session->description);
		if(p_batch_session->reason!=NULL)
			free(p_batch_session->reason);
		if(p_batch_session->username!=NULL)
			free(p_batch_session->username);
		if(p_batch_session->usergroup!=NULL)
			free(p_batch_session->usergroup);
		if(p_batch_session->projectname!=NULL)
			free(p_batch_session->projectname);
		if(p_batch_session->submit_hostname!=NULL)
			free(p_batch_session->submit_hostname);
		if(p_batch_session->qos!=NULL)
			free(p_batch_session->qos);
		if(p_batch_session->queue!=NULL)
			free(p_batch_session->queue);
		if(p_batch_session->executing_hostname!=NULL)
			free(p_batch_session->executing_hostname);

		bridge_nodelist_free_contents(&(p_batch_session->
						par_nodelist));

		fstatus=0;
	}

	return fstatus;
}



int
get_batch_sessions(bridge_batch_manager_t* p_batch_manager,
		   bridge_batch_session_t** p_p_batch_sessions,
		   int* p_batch_sessions_nb,
		   char* batch_sessions_batch_ids,
		   char* jobname,char* username,char* batch_queue,
		   char* execHost,int parallel_infos_flag)
{
	int fstatus=-1;
	
	int con_fd;
	struct batch_status * nstat;
	struct batch_status * cstat;
	struct attrl *attr;

	char* p;

	int enabled=0,started=0;

	int session_nb=0;
	int stored_session_nb=0;
	
	bridge_batch_session_t* bn;

	con_fd = *((int*)p_batch_manager->private);

	/* get session stats */
	nstat = pbs_statjob(con_fd,NULL,NULL,NULL);
	if ( nstat == NULL ) 
		return fstatus;
	
	/* count sessions quantity */
	cstat = nstat ;
	while ( cstat != NULL ) {
		session_nb++;
		cstat=cstat->next;
	}
	
	/* initialized output buffer if required */
	if(*p_p_batch_sessions!=NULL){
		if(*p_batch_sessions_nb<session_nb)
			session_nb=*p_batch_sessions_nb;
	}
	else{
		*p_p_batch_sessions=(bridge_batch_session_t*)
			malloc(session_nb*(sizeof(bridge_batch_session_t)+1));
		if(*p_p_batch_sessions==NULL){
			*p_batch_sessions_nb=0;
			session_nb=*p_batch_sessions_nb;
		}
		else{
			*p_batch_sessions_nb=session_nb;
		}
	}
	stored_session_nb=0;

	/* fill output buffer with stats */
	cstat = nstat ;
	while ( cstat != NULL && stored_session_nb < session_nb ) {

		bn = &(*p_p_batch_sessions)[stored_session_nb];
		
		/* put default values */
		init_batch_session(p_batch_manager,bn);

		/* session id */
		bn->batch_id=strdup(cstat->name);
		p = index(bn->batch_id,'.');
		if ( p != NULL )
			*p='\0';
		
		attr = cstat->attribs ;
		while ( attr != NULL ) {
			if (!strcmp(attr->name, ATTR_p)) {
				sscanf(attr->value,"%d",&(bn->priority));
			}
			else if (!strcmp(attr->name, ATTR_l)) {
				if (!strcmp(attr->resource,"nodes")) {
                                        extract_cores(attr->value,
                                                      &(bn->par_cores_nb_limit));
				}
				else if (!strcmp(attr->resource,"pmem")) {
					convert_mem(attr->value,
						    &(bn->par_mem_limit));
					bn->seq_mem_limit = bn->par_mem_limit;
				}
				else if (!strcmp(attr->resource,"walltime")) {
					convert_time(attr->value,
						     &(bn->par_time_limit));
					bn->seq_time_limit = bn->par_time_limit ;
				}
			}
			else if (!strcmp(attr->name, ATTR_used)) {
				if (!strcmp(attr->resource,"vmem")) {
					convert_mem(attr->value,
						    &(bn->par_mem_used));
					bn->seq_mem_used = bn->par_mem_used;
				}
				else if (!strcmp(attr->resource,"walltime")) {
					convert_time(attr->value,
						     &(bn->par_time_used));
					bn->user_time = bn->par_time_used ;
				}
			}
			else if (!strcmp(attr->name, ATTR_queue)) {
				bn->queue=strdup(attr->value);
			}
			else if (!strcmp(attr->name, ATTR_N)) {
				bn->name=strdup(attr->value);
			}
			else if (!strcmp(attr->name, ATTR_start_time)) {
				sscanf(attr->value,"%u",&(bn->start_time));
			}
			else if (!strcmp(attr->name, ATTR_session)) {
				sscanf(attr->value,"%u",&(bn->session_id));
			}
			else if (!strcmp(attr->name, ATTR_ctime)) {
				sscanf(attr->value,"%u",&(bn->submit_time));
			}
			else if (!strcmp(attr->name, ATTR_A)) {
				bn->projectname=strdup(attr->value);
			}
			/* set job state */
			else if (!strcmp(attr->name, ATTR_state)) {
				if (!strcmp(attr->value,"R"))
					bn->state=BRIDGE_BATCH_SESSION_STATE_RUNNING;
				else if (!strcmp(attr->value,"Q"))
					bn->state=BRIDGE_BATCH_SESSION_STATE_PENDING;
				else if (!strcmp(attr->value,"E"))
					bn->state=BRIDGE_BATCH_SESSION_STATE_RUNNING;
				else if (!strcmp(attr->value,"H"))
					bn->state=BRIDGE_BATCH_SESSION_STATE_SUSPENDED;
				else if (!strcmp(attr->value,"T"))
					bn->state=BRIDGE_BATCH_SESSION_STATE_PENDING;
				else if (!strcmp(attr->value,"W"))
					bn->state=BRIDGE_BATCH_SESSION_STATE_PENDING;
				else
					bn->state=BRIDGE_BATCH_SESSION_STATE_UNKNOWN;
				
			}
			/* set owner and subhost */
			else if (!strcmp(attr->name, ATTR_owner)) {
				char* ind;
				char* ind2;
				ind = index(attr->value,'@');
				if ( ind == NULL ) {
					bn->username=strdup(attr->value);
				}
				else {
					*ind='\0';
					bn->username=strdup(attr->value);
					*ind='@';
					ind++;
					ind2 = index(ind,'.');
					if ( ind2 != NULL ) {
						*ind2='\0';
						bn->submit_hostname=strdup(ind);
						*ind2='.';
					}
				}
			}
			/* set script exec host and parallele nodes */
			else if (!strcmp(attr->name, ATTR_exechost)) {
				/* the first exec node is the node */
				/* that executes the bacth script */
				char* tmp;
				tmp = index(attr->value,'/');
				if ( tmp == NULL )
					bn->executing_hostname = 
						strdup(attr->value);
				else {
					*tmp='\0';
					bn->executing_hostname = 
						strdup(attr->value);
					*tmp='/';
				}
				tmp = strdup(attr->value);
				if ( tmp != NULL ) {
					p=tmp;
					while ( *p != '\0' ) {
						if ( *p == '/' ) {
							while ( *p != '+' &&
								*p != '\0' ) {
								*p=' ';
								p++;
							}
							if ( *p == '+' )
								*p=' ';
						}
						else
							p++;
					}
					bridge_nodelist_add_nodes(&(bn->par_nodelist),tmp);
					free(tmp);
				}
			}
			
			attr = attr->next ;
		}

		if ( bn->state == BRIDGE_BATCH_SESSION_STATE_RUNNING ) {
			bn->par_cores_nb = bn->par_cores_nb_limit ;
			bn->reason=strdup("R00");
		}

		/* torque mem value is global for all the cores */
		if ( bn->par_cores_nb_limit > 0 ) {
			bn->par_mem_used = bn->par_mem_used / bn->par_cores_nb_limit ;
			bn->seq_mem_used = bn->seq_mem_used / bn->par_cores_nb_limit ;
		}

		/* username filter */
		if ( bn != NULL && 
		     username != NULL && bn->username!=NULL &&
		     strcmp(username,bn->username)!=0 ) {
			clean_batch_session(p_batch_manager,bn);
			bn=NULL;
		}

		/* queue filter */
		if ( bn != NULL && 
		     batch_queue != NULL && bn->queue != NULL &&
		     strcmp(batch_queue,bn->queue)!=0 ) {
			clean_batch_session(p_batch_manager,bn);
			bn=NULL;
		}

		/* execHost filter */
		if ( bn != NULL && 
		     execHost != NULL && 
		     ( (bn->executing_hostname != NULL &&
			strcmp(execHost,bn->executing_hostname)!=0 ) ||
		       bn->executing_hostname == NULL ) ) {
			clean_batch_session(p_batch_manager,bn);
			bn=NULL;
		}

		/* batchid filter batch_sessions_batch_ids*/
		if ( bn != NULL && 
		     batch_sessions_batch_ids != NULL && bn->batch_id &&
		     strcmp(batch_sessions_batch_ids,bn->batch_id)!=0 ) {
			clean_batch_session(p_batch_manager,bn);
			bn=NULL;
		}

		/* job name filter */
		if ( bn != NULL && 
		     jobname != NULL && bn->name &&
		     strcmp(jobname,bn->name)!=0 ) {
			clean_batch_session(p_batch_manager,bn);
			bn=NULL;
		}
		
		if ( bn != NULL ) {
			stored_session_nb++;
		}

		cstat=cstat->next;
	}
	pbs_statfree(nstat);

	fstatus=0;

	if(stored_session_nb<session_nb){
		*p_p_batch_sessions=(bridge_batch_session_t*)
			realloc(*p_p_batch_sessions,
				stored_session_nb*(sizeof(bridge_batch_session_t)+1));
		if(*p_p_batch_sessions==NULL)
			*p_batch_sessions_nb=0;
		else
			*p_batch_sessions_nb=stored_session_nb;
	}

	return fstatus;
}

int
get_terminated_batch_sessions(bridge_batch_manager_t* p_batch_manager,
			      bridge_batch_session_t** p_p_batch_sessions,
			      int* p_batch_sessions_nb,
			      char* batch_sessions_batch_ids,
			      char* jobname,char* username,
			      char* batch_queue,char* execHost,
			      int parallel_infos_flag,
			      time_t begin_eventTime,time_t end_eventTime)
{
	int fstatus=-1;

	return fstatus;
}
