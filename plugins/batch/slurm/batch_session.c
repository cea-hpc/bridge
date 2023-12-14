/*****************************************************************************\
 *  plugins/batch/slurm/batch_session.c -
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

#include "xternal/xlogger.h"
#include "bridge/bridge.h"

#define DEBUG_LOGGER xdebug
#define DEBUG2_LOGGER xdebug2
#define DEBUG3_LOGGER xdebug3
#define ERROR_LOGGER xerror

#include <slurm/slurm.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>

#ifndef JOB_STATE_BASE
#define JOB_STATE_BASE 0x00ff
#endif

/*Missing in slurm.h*/
extern char *slurm_job_reason_string(enum job_state_reason inx);

#define DEFAULT_PAR_CPUS_NB_LIMIT 0

#define DEFAULT_TIME_LIMIT   999999

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

		p_batch_session->queue=NULL;
		p_batch_session->qos=NULL;
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

	int i;

	job_info_msg_t* pjim;
	job_info_t* pji;

	int session_nb=0;
	int stored_session_nb=0;

	struct passwd mypasswd;
	struct passwd* pmypasswd;
	size_t mypasswdbuf_len=256;
	char mypasswdbuf[mypasswdbuf_len];

	struct group mygroup;
	struct group* pmygroup;
	size_t mygroupbuf_len=256;
	char mygroupbuf[mygroupbuf_len];



	bridge_batch_session_t* bn;

	/* get session stats */
	if (slurm_load_jobs(0,&pjim,SHOW_ALL) != 0) {
		DEBUG3_LOGGER("unable to get allocations information");
		goto exit;
	}
	session_nb = pjim->record_count;

	/* build/initialize storage structures */
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


	/* fill structures */
	for (i=0;i<pjim->record_count && stored_session_nb<session_nb;i++) {

		int loop=0;

		/* get session pointer */
		pji=pjim->job_array+i;

		if (pji->name == NULL)
			continue;

		switch ( pji->job_state ) {
		case JOB_COMPLETE | JOB_COMPLETING :
		case JOB_CANCELLED | JOB_COMPLETING :
		case JOB_FAILED | JOB_COMPLETING :
		case JOB_TIMEOUT | JOB_COMPLETING :
		        break;
		case JOB_COMPLETE :
		case JOB_CANCELLED :
		case JOB_FAILED :
		case JOB_TIMEOUT :
			loop=1;
			break;
		default:
			break;
		}

		if ( loop )
			continue;

		/* queue name filter */
		if (jobname != NULL &&
		    strcmp(jobname,pji->name) != 0)
			continue;

		bn = &(*p_p_batch_sessions)[stored_session_nb];

		/* put default values */
		init_batch_session(p_batch_manager,bn);

		/* id */
		char strid[128];
		snprintf(strid,128,"%d",pji->job_id);
		bn->batch_id=strdup(strid);
		bn->rm_id=strdup(strid);

		/* name */
		bn->name=strdup(pji->name);

		/* state */
		if ( pji->job_state & JOB_COMPLETING ) {
  		        bn->state=BRIDGE_BATCH_SESSION_STATE_RUNNING;
			bn->reason = strdup("R00");
		}
		else
 		        switch ( pji->job_state & JOB_STATE_BASE ) {

			case JOB_PENDING :
			  bn->state=BRIDGE_BATCH_SESSION_STATE_PENDING;
			  if ( pji->state_desc != NULL )
			    bn->reason=strdup(pji->state_desc);
			  else
			    bn->reason=strdup(slurm_job_reason_string(pji->state_reason));
			  break;
			case JOB_RUNNING :
			  bn->state=BRIDGE_BATCH_SESSION_STATE_RUNNING;
			  bn->reason = strdup("R01");
			  break;
			case JOB_SUSPENDED :
			  bn->state=BRIDGE_BATCH_SESSION_STATE_SUSPENDED;
			  break;
			default :
			  bn->state=BRIDGE_BATCH_SESSION_STATE_UNKNOWN;
			  break;

			}

		/* user & group names */
		if (getpwuid_r(pji->user_id,&mypasswd,mypasswdbuf,
			       mypasswdbuf_len,&pmypasswd) == 0 &&
		    pmypasswd != NULL ) {
			bn->username=strdup(mypasswd.pw_name);
		}
		else {
		        char* username = (char*) malloc(128*sizeof(char));
			if ( username != NULL ) {
			        snprintf(username,128,"%u",pji->user_id);
			}
			bn->username = username;
		}
		if (getgrgid_r(pji->group_id,&mygroup,mygroupbuf,
			       mygroupbuf_len,&pmygroup) == 0 &&
		    pmygroup != NULL ) {
			bn->usergroup=strdup(mygroup.gr_name);
		}
		else {
		        char* groupname = (char*) malloc(128*sizeof(char));
			if ( groupname != NULL ) {
			        snprintf(groupname,128,"%u",pji->group_id);
			}
			bn->usergroup = groupname;
		}

		/* account */
		if (pji->account != NULL)
			bn->projectname=strdup(pji->account);

		/* submit host and time */
		if (pji->alloc_node != NULL)
			bn->submit_hostname=strdup(pji->alloc_node);
		bn->submit_time=pji->submit_time;
		bn->start_time=pji->start_time;
		bn->end_time=pji->end_time;

		/* queue */
		if (pji->partition != NULL)
			bn->queue=strdup(pji->partition);

		/* priority */
		bn->priority = pji->priority ;

		/* qos */
		if (pji->qos != NULL)
		        bn->qos = strdup(pji->qos) ;

		/* exec host and nodes list */
		if (pji->nodes != NULL) {
// Slurm >= 23.02.2
#if SLURM_VERSION_NUMBER >= SLURM_VERSION_NUM(23,2,2)
			hostlist_t *hl = slurm_hostlist_create(pji->nodes);
#else
			hostlist_t hl = slurm_hostlist_create(pji->nodes);
#endif
			bn->executing_hostname = slurm_hostlist_shift(hl);
			slurm_hostlist_destroy(hl);
			bridge_nodelist_add_nodes(&(bn->par_nodelist),pji->nodes);
		}

		/* time usage */
		if (pji->time_limit != INFINITE) {
			bn->seq_time_limit = pji->time_limit * 60;
			bn->par_time_limit = pji->time_limit * 60;
		}
		else {
			bn->seq_time_limit = bn->end_time - bn->start_time ;
			bn->par_time_limit = bn->seq_time_limit ;
		}
		if ( pji->suspend_time > 0 ) {
			bn->par_time_used = pji->pre_sus_time ;
		}
		if (bn->state == BRIDGE_BATCH_SESSION_STATE_RUNNING) {
			if ( pji->suspend_time > 0 )
				bn->par_time_used += pjim->last_update - pji->suspend_time ;
			else
				bn->par_time_used += pjim->last_update - pji->start_time ;
		}
		bn->user_time = bn->par_time_used;

		/* cores usage */
		if ( bn->state == BRIDGE_BATCH_SESSION_STATE_RUNNING )
		  bn->par_cores_nb=pji->num_cpus;
		bn->par_cores_nb_limit=pji->num_cpus;

		/* mem usage (see scontrol) */
		if (pji->pn_min_memory & MEM_PER_CPU) {
			pji->pn_min_memory &= (~MEM_PER_CPU);
		}
		bn->par_mem_limit= (uint32_t) pji->pn_min_memory;
		bn->seq_mem_limit= (uint32_t) pji->pn_min_memory;
		/* must find a good way to do that... */
		if ( bn->state == BRIDGE_BATCH_SESSION_STATE_RUNNING ) {
		  bn->par_mem_used = bn->par_mem_limit;
		  bn->seq_mem_used = bn->seq_mem_limit;
		}

		/* username filter */
		if ( bn && username != NULL && bn->username!=NULL &&
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

	}

	fstatus=0;

	/* free slurm partition information */
	slurm_free_job_info_msg(pjim);

	if(stored_session_nb<session_nb){
		*p_p_batch_sessions=(bridge_batch_session_t*)
			realloc(*p_p_batch_sessions,
				stored_session_nb*(sizeof(bridge_batch_session_t)+1));
		if(*p_p_batch_sessions==NULL)
			*p_batch_sessions_nb=0;
		else
			*p_batch_sessions_nb=stored_session_nb;
	}

exit:
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
