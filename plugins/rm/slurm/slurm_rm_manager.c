/*****************************************************************************\
 *  plugins/rm/slurm/slurm_rm_manager.c - 
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

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <search.h>
#include <string.h>

#include <limits.h>
#include <time.h>
#include <ctype.h>

#include <slurm/slurm.h>
#include <pwd.h>

#include "xternal/xlogger.h"
#include "bridge/bridge.h"

#define DEBUG_LOGGER xdebug
#define DEBUG2_LOGGER xdebug2
#define DEBUG3_LOGGER xdebug3
#define ERROR_LOGGER xerror

#ifndef JOB_STATE_BASE
#define JOB_STATE_BASE 0x00ff
#endif

/* local functions */
int get_rm_allocations_base(bridge_rm_manager_t * p_rm_manager,
			    bridge_rm_allocation_t ** p_rm_allocations,
			    int* p_rm_allocations_nb,
			    char* rm_allocations_rm_id,
			    char* username,
			    char* rm_partition,
			    char* intersectingNodes,
			    char* includingNodes,
			    int terminated_flag,
			    time_t begin_eventTime,time_t end_eventTime);

/*
####################################################################################################
MANAGER FUNCTIONS
####################################################################################################
*/

int
get_rm_id(char** id)
{
  int fstatus=-1;
  
  char* rm_id = getenv("SLURM_JOBID");
  
  if ( rm_id != NULL ) {
    *id = strdup(rm_id);

    if ( *id != NULL )
      fstatus=0;
  }
  
  return fstatus;
}

int
init_rm_manager(bridge_rm_manager_t* p_manager){

  int fstatus=-1;
  char* cluster;
  char* p;

  long api_version;
  char version[128];

  char* separator;

  slurm_conf_t * pscc;

  p_manager->version=NULL;
  p_manager->cluster=NULL;
  p_manager->master=NULL;
  
  /* get slurm API version */
  api_version=slurm_api_version();
  snprintf(version,128,"%ld.%ld.%ld",
	   SLURM_VERSION_MAJOR(api_version),
	   SLURM_VERSION_MINOR(api_version),
	   SLURM_VERSION_MICRO(api_version));

  /* get slurm controller configuration */
  if(slurm_load_ctl_conf(0,&pscc)==0)
    {
      p_manager->type=strdup("SLURM");
      p_manager->version=strdup(version);
      p_manager->description=strdup("no desc");
      
      /* get cluster and master names */
      if(pscc->control_machine[0]!=NULL)
	{
	  p_manager->master=strdup(pscc->control_machine[0]);
	  cluster=strdup(p_manager->master);
	  if(cluster!=NULL)
	    {
	      separator=index(cluster,'-');
	      if(separator==NULL)
		{
		  p=cluster;
		  while(!isdigit(*p) && *p!='\0')
		    p++;
		  if(*p!='\0')
		    {
		      *p='\0';
		      p_manager->cluster=strdup(cluster);
		    }
		}
	      else
		{
		  if(strncmp("mgr",separator+1,4)==0)
		    {
		      *separator='\0';
		      p_manager->cluster=strdup(cluster);
		    }
		  else if(strncmp("cws",separator-3,3)==0)
		    {
		      p_manager->cluster=strdup(separator+1);
		    }
		}
	      free(cluster);
	    }
	}
      p_manager->masters_list=NULL;
      fstatus=0;
      /* free slurm controller configuration data */
      slurm_free_ctl_conf(pscc);
    }
  else
    {
      DEBUG3_LOGGER("unable to get slurmctl configuration");
    }

  return fstatus;

}


int
bridge_rm_check_manager_validity(bridge_rm_manager_t* p_manager){
  int fstatus=-1;
  if( p_manager->type!=NULL &&
      p_manager->version!=NULL &&
      p_manager->cluster!=NULL &&
      p_manager->master!=NULL &&
      p_manager->description!=NULL)
    fstatus=0;
  return fstatus;
}


int
clean_rm_manager(bridge_rm_manager_t* p_manager){
  int fstatus=0;
  xfree(p_manager->type);
  xfree(p_manager->version);
  xfree(p_manager->cluster);
  xfree(p_manager->master);
  xfree(p_manager->description);
  if(p_manager->private!=NULL)
    {
      p_manager->private=NULL;
    }
  return fstatus;
}

int 
init_rm_partition(bridge_rm_manager_t * p_rm_manager,
		  bridge_rm_partition_t * p_rm_partition){
  int status=0;
  if(p_rm_partition==NULL)
    {
      status=-1;
    }
  else
    {
      p_rm_partition->name=NULL;
      p_rm_partition->description=NULL;

      p_rm_partition->state=BRIDGE_RM_PARTITION_STATE_UNKNOWN;
      p_rm_partition->reason=NULL;

      p_rm_partition->total_cores_nb=INVALID_INTEGER_VALUE;
      p_rm_partition->active_cores_nb=INVALID_INTEGER_VALUE;
      p_rm_partition->used_cores_nb=INVALID_INTEGER_VALUE;
      
      p_rm_partition->total_nodes_nb=INVALID_INTEGER_VALUE;
      p_rm_partition->active_nodes_nb=INVALID_INTEGER_VALUE;
      p_rm_partition->used_nodes_nb=INVALID_INTEGER_VALUE;

      p_rm_partition->time_limit=INVALID_TIME_VALUE;
      p_rm_partition->memory_limit=INVALID_INTEGER_VALUE;

      p_rm_partition->start_time=INVALID_TIME_VALUE;

      bridge_nodelist_init(&(p_rm_partition->total_nodelist),NULL,0);
      bridge_nodelist_init(&(p_rm_partition->active_nodelist),NULL,0);
      bridge_nodelist_init(&(p_rm_partition->used_nodelist),NULL,0);
    }
  return status;
}

int
clean_rm_partition(bridge_rm_manager_t * p_rm_manager,
		   bridge_rm_partition_t * p_rm_partition){
  int status=0;
  if(p_rm_partition==NULL)
    {
      status=-1;
    }
  else
    {
      xfree(p_rm_partition->name);
      xfree(p_rm_partition->description);

      p_rm_partition->state=BRIDGE_RM_PARTITION_STATE_UNKNOWN;
      xfree(p_rm_partition->reason);
      
      p_rm_partition->total_cores_nb=0;
      p_rm_partition->active_cores_nb=0;
      p_rm_partition->used_cores_nb=0;
      
      p_rm_partition->total_nodes_nb=0;
      p_rm_partition->active_nodes_nb=0;
      p_rm_partition->used_nodes_nb=0;
      
      p_rm_partition->time_limit=INVALID_TIME_VALUE;
      p_rm_partition->memory_limit=INVALID_TIME_VALUE;
      
      p_rm_partition->start_time=INVALID_TIME_VALUE;
      
      bridge_nodelist_free_contents(&(p_rm_partition->total_nodelist));
      bridge_nodelist_free_contents(&(p_rm_partition->active_nodelist));
      bridge_nodelist_free_contents(&(p_rm_partition->used_nodelist));
    }
  return status;
}

int
get_rm_partitions(bridge_rm_manager_t * p_rm_manager,
		  bridge_rm_partition_t ** p_rm_partitions,
		  int* p_rm_partitions_nb,
		  char* rm_partition_name,
		  char* intersectingNodes,
		  char* includingNodes){
  int fstatus=-1;

  int partitions_nb,real_partitions_nb;
  int i;
  bridge_rm_partition_t* partitions;

  partition_info_msg_t* ppim;
  partition_info_t* ppi;

  node_info_msg_t* pnim;
  node_info_t* pni;
  job_info_msg_t* pjim;
  job_info_t* pji;
  int j;

  bridge_nodelist_t list;

  /* get nodes status */
  if(slurm_load_node(0,&pnim,SHOW_ALL))
    {
      DEBUG3_LOGGER("unable to get nodes informations");
      pnim=NULL;
    }
  /* get jobs status */
  if(slurm_load_jobs(0,&pjim,SHOW_ALL))
    {
      DEBUG3_LOGGER("unable to get jobs informations");
      pjim=NULL;
    }
  /* get partitions infos */
  if(slurm_load_partitions(0,&ppim,SHOW_ALL)!=0)
    {
      DEBUG3_LOGGER("unable to get partitions informations");
    }
  else
    {
      /* build storage structures */
      partitions_nb=ppim->record_count;
      if( *p_rm_partitions == NULL)
	{
	  *p_rm_partitions=(bridge_rm_partition_t*)malloc(partitions_nb*sizeof(bridge_rm_partition_t));
	}
      else
	{
	  if(partitions_nb>*p_rm_partitions_nb)
	    partitions_nb=*p_rm_partitions_nb;
	}
      partitions=*p_rm_partitions;
      real_partitions_nb=0;
      
      /* fill in bridge rm partitions structure */
      if(partitions!=NULL)
	{
	  for(i=0;i<partitions_nb;i++)
	    {
	      /* get partition pointer */
	      ppi=ppim->partition_array+i;
	      
	      /* apply  required filters */
	      if(rm_partition_name!=NULL)
		{
		  if(ppi->name!=NULL)
		    {
		      if(strcmp(ppi->name,rm_partition_name)!=0)
			continue;
		    }
		  else
		    continue;
		}
	      
	      /* initialize partition structure */
	      init_rm_partition(p_rm_manager,&partitions[real_partitions_nb]);

	      /* fill in partition structure using query result */
	      /* partition name */
	      if(ppi->name!=NULL)
		{
		  partitions[real_partitions_nb].name=strdup(ppi->name);
		}
	      /* partition type */
	      partitions[real_partitions_nb].description=strdup("no desc");
	      /* partition state && reason of this state */
	      if(ppi->state_up == PARTITION_UP)
		{
		  partitions[real_partitions_nb].state=BRIDGE_RM_PARTITION_STATE_IN;
		  partitions[real_partitions_nb].reason=strdup("submission and scheduling");
		}
	      else if (ppi->state_up == PARTITION_DRAIN) {
		  partitions[real_partitions_nb].state=BRIDGE_RM_PARTITION_STATE_DRAIN;
		  partitions[real_partitions_nb].reason=strdup("scheduling only");
	      } else if (ppi->state_up == PARTITION_DOWN) {
		  partitions[real_partitions_nb].state=BRIDGE_RM_PARTITION_STATE_OUT;
		  partitions[real_partitions_nb].reason=strdup("submission only");
	      } else {		      
		  partitions[real_partitions_nb].state=BRIDGE_RM_PARTITION_STATE_OUT;
		  partitions[real_partitions_nb].reason=strdup("inactive");
	      }
	      
	      /* resources informations */
	      partitions[real_partitions_nb].total_cores_nb=ppi->total_cpus;
	      partitions[real_partitions_nb].total_nodes_nb=ppi->total_nodes;
	      if(ppi->max_time!=-1)
		partitions[real_partitions_nb].time_limit=ppi->max_time*60;
	      else
		partitions[real_partitions_nb].time_limit=INVALID_TIME_VALUE;
	      //partitions[real_partitions_nb].memory_limit=;
	      //partitions[real_partitions_nb].start_time=;
	      bridge_nodelist_add_nodes(&(partitions[real_partitions_nb].total_nodelist),
					ppi->nodes);
	      
	      /* usage & active informations */ 
	      if(pnim!=NULL)
		{
		  partitions[real_partitions_nb].used_nodes_nb=0;
		  partitions[real_partitions_nb].used_cores_nb=0;
		  partitions[real_partitions_nb].active_nodes_nb=0;
		  partitions[real_partitions_nb].active_cores_nb=0;
		  for(j=0;j<pnim->record_count;j++)
		    {
		      pni=pnim->node_array+j;
		      bridge_nodelist_init(&list,NULL,0);
		      bridge_nodelist_add_nodes(&list,pni->name);
		      if(bridge_nodelist_intersects(&(partitions[real_partitions_nb].total_nodelist),&list)==0)
			{
			  bridge_nodelist_free_contents(&list);
			  continue;
			}
		      bridge_nodelist_free_contents(&list);
		      
		      if((pni->node_state & NODE_STATE_BASE)==NODE_STATE_ALLOCATED)
			{
			  bridge_nodelist_add_nodes(&(partitions[real_partitions_nb].used_nodelist),
						    pni->name);
			  partitions[real_partitions_nb].used_nodes_nb+=1;
			  if(!(pni->node_state & NODE_STATE_DRAIN)) {
			    bridge_nodelist_add_nodes(&(partitions[real_partitions_nb].active_nodelist),
						      pni->name);
			    partitions[real_partitions_nb].active_nodes_nb+=1;
			    partitions[real_partitions_nb].active_cores_nb+=pni->cpus;
			  }
			}
		      else if((pni->node_state & NODE_STATE_BASE)==NODE_STATE_IDLE &&
			      !(pni->node_state & NODE_STATE_DRAIN))
			{
			  bridge_nodelist_add_nodes(&(partitions[real_partitions_nb].active_nodelist),
							     pni->name);
			  partitions[real_partitions_nb].active_nodes_nb+=1;
			  partitions[real_partitions_nb].active_cores_nb+=pni->cpus;
			}
		    }
		}
	      if(pjim!=NULL) {
		for ( j=0 ; j < pjim->record_count ; j++ ) {
		  pji=pjim->job_array+j;
		  switch ( pji->job_state & JOB_STATE_BASE ) {
		  case JOB_RUNNING :
		    if ( strcmp(pji->partition,ppi->name) != 0 )
		      continue;
		    partitions[real_partitions_nb].used_cores_nb+=pji->num_cpus;
		    break;
		  }
		}
	      }

	      
	
	      /* node filtering */
	      int skip_flag=0;
	      if(intersectingNodes!=NULL) /* intersection */
		{
		  bridge_nodelist_t list;
		  bridge_nodelist_init(&list,NULL,0);
		  bridge_nodelist_add_nodes(&list,intersectingNodes);
		  if(bridge_nodelist_intersects(&(partitions[real_partitions_nb].total_nodelist),&list)==0)
		    {
		      skip_flag=1;
		    }
		  bridge_nodelist_free_contents(&list);
		}
	      else if(includingNodes!=NULL) /* inclusion */
		{
		  bridge_nodelist_t list;
		  bridge_nodelist_init(&list,NULL,0);
		  bridge_nodelist_add_nodes(&list,includingNodes);
		  if(bridge_nodelist_includes(&list,&(partitions[real_partitions_nb].total_nodelist))==0)
		    {
		      skip_flag=1;
		    }
		  bridge_nodelist_free_contents(&list);
		}

	      /* include or remove this partition */
	      if(skip_flag==1){
		clean_rm_partition(p_rm_manager,&partitions[real_partitions_nb]);
	      }
	      else
		real_partitions_nb++;
	    
	    }
	}
    
      /* skrink partitions buffer */
      *p_rm_partitions_nb=real_partitions_nb;
      if(real_partitions_nb!=partitions_nb)
	*p_rm_partitions=(bridge_rm_partition_t*)realloc(*p_rm_partitions,real_partitions_nb*sizeof(bridge_rm_partition_t));
    
      /* set return status */
      if(real_partitions_nb>0 && *p_rm_partitions!=NULL)
	fstatus=0;
     
      /* free slurm partition informations */
      slurm_free_partition_info_msg(ppim);
    }
  
  /* free slurm node info messages */
  if(pnim!=NULL)
    slurm_free_node_info_msg(pnim);
  /* free slurm job info messages */
  if(pjim!=NULL)
    slurm_free_job_info_msg(pjim);
  
  return fstatus;
}


int
init_rm_allocation(bridge_rm_manager_t * p_rm_manager,
		   bridge_rm_allocation_t * p_allocation){
  int status=0;
  if(p_allocation==NULL)
    {
      status=-1;
    }
  else
    {
      p_allocation->id=NULL; /* Allocation id */
      p_allocation->name=NULL; /* Allocation name */
      p_allocation->description=NULL; /* More info about allocation */

      p_allocation->partition=NULL; /* Allocation partition */
      
      p_allocation->state=BRIDGE_RM_ALLOCATION_STATE_UNKNOWN; /* State of allocation */
      p_allocation->reason=NULL; /* reason of this state */
      
      p_allocation->priority=INVALID_INTEGER_VALUE; /* Allocation priority */
      
      p_allocation->username=NULL; /* allocation owner username*/
      p_allocation->userid=INVALID_INTEGER_VALUE; /* allocation owner username*/
      p_allocation->groupid=INVALID_INTEGER_VALUE; /* allocation owner groupname*/
      
      p_allocation->submit_time=INVALID_TIME_VALUE; /* time of allocation request submission */
      p_allocation->start_time=INVALID_TIME_VALUE; /* time of allocation start */
      p_allocation->end_time=INVALID_TIME_VALUE; /* time of estimated or realized end time of allocation*/
      p_allocation->suspend_time=INVALID_TIME_VALUE; /* if suspended state, contains time of suspension */

      p_allocation->elapsed_time=INVALID_TIME_VALUE; /* time used by allocation */
      p_allocation->allocated_time=INVALID_TIME_VALUE; /* elapsed_time * cores_nb */
      
      p_allocation->total_cores_nb=INVALID_INTEGER_VALUE; /* Total number of allocated cores */
      p_allocation->used_cores_nb=INVALID_INTEGER_VALUE; /* Number of cores in use */
      
      p_allocation->total_nodes_nb=INVALID_INTEGER_VALUE; /* Total number of allocated nodes */
      p_allocation->used_nodes_nb=INVALID_INTEGER_VALUE; /* Number of nodes in use */
      
      p_allocation->memory_usage=INVALID_INTEGER_VALUE; /* Average usage of memory per core */
      
      p_allocation->system_time=INVALID_TIME_VALUE; /* sum of cores time used by system due to jobs activity */
      p_allocation->user_time=INVALID_TIME_VALUE; /* sum of cores time used by user due to jobs activity */
      
      bridge_nodelist_init(&(p_allocation->nodelist),NULL,0);
      
      p_allocation->allocating_hostname=NULL; /* Hostname where allocation request was submitted */
      p_allocation->allocating_session_id=INVALID_INTEGER_VALUE; /* Session ID on Hostname where allocation request was submitted */
      p_allocation->allocating_pid=INVALID_INTEGER_VALUE; /* Process ID of the allocator */
      
      p_allocation->allocating_session_batchid=NULL; /* batchid value of allocating session ( may be invalid if not a batch session ) */
      
      bridge_idlist_init(&(p_allocation->jobidlist),NULL,0); /* IDs of jobs launched by the allocation */
    }
  return status;
}

int
clean_rm_allocation(bridge_rm_manager_t * p_rm_manager,
		    bridge_rm_allocation_t * p_allocation){
  int status=0;
  
  if(p_allocation==NULL)
    {
      status=-1;
    }
  else
    {
      if(p_allocation->id != NULL)
	free(p_allocation->id);
      if(p_allocation->name != NULL)
	free(p_allocation->name);
      if(p_allocation->username != NULL)
	free(p_allocation->username);
      if(p_allocation->description != NULL)
	free(p_allocation->description);
      if(p_allocation->partition != NULL)
	free(p_allocation->partition);
      if(p_allocation->reason != NULL)
	free(p_allocation->reason);
      
      bridge_nodelist_free_contents(&(p_allocation->nodelist));
      
      bridge_idlist_free_contents(&(p_allocation->jobidlist));
      
      if(p_allocation->allocating_hostname != NULL)
	free(p_allocation->allocating_hostname);
      if(p_allocation->allocating_session_batchid != NULL)
	free(p_allocation->allocating_session_batchid);
    }
  
  return status;
  
}

int
get_rm_allocations(bridge_rm_manager_t * p_rm_manager,
		   bridge_rm_allocation_t ** p_rm_allocations,
		   int* p_rm_allocations_nb,
		   char* rm_allocations_rm_id,
		   char* username,
		   char* rm_partition,
		   char* intersectingNodes,
		   char* includingNodes){

  return get_rm_allocations_base(p_rm_manager,
				 p_rm_allocations,
				 p_rm_allocations_nb,
				 rm_allocations_rm_id,
				 username,
				 rm_partition,
				 intersectingNodes,
				 includingNodes,
				 0,
				 INVALID_TIME_VALUE,INVALID_TIME_VALUE);
}

int
get_terminated_rm_allocations(bridge_rm_manager_t * p_rm_manager,
			      bridge_rm_allocation_t ** p_rm_allocations,
			      int* p_rm_allocations_nb,
			      char* rm_allocations_rm_id,
			      char* username,
			      char* rm_partition,
			      char* intersectingNodes,
			      char* includingNodes,
			      time_t begin_eventTime,time_t end_eventTime){

  return get_rm_allocations_base(p_rm_manager,
				 p_rm_allocations,
				 p_rm_allocations_nb,
				 rm_allocations_rm_id,
				 username,
				 rm_partition,
				 intersectingNodes,
				 includingNodes,
				 1,
				 begin_eventTime,end_eventTime);

}

int
get_rm_allocations_base(bridge_rm_manager_t * p_rm_manager,
			bridge_rm_allocation_t ** p_rm_allocations,
			int* p_rm_allocations_nb,
			char* rm_allocations_rm_id,
			char* username,
			char* rm_partition,
			char* intersectingNodes,
			char* includingNodes,
			int terminated_flag,
			time_t begin_eventTime,time_t end_eventTime){

  int fstatus=-1;

  /* Defines bridge variables */
  bridge_rm_allocation_t* allocations;
  int real_allocations_nb=0;
  int nb_allocations;
  int out_flag;

  time_t current_time;
  job_info_msg_t* pjim;
  job_info_t* pji;
  int j;

  job_step_info_response_msg_t* pjsirm;
  job_step_info_t* pjsi;
  int k;

  struct passwd mypasswd;
  struct passwd* pmypasswd;
  size_t mypasswdbuf_len=256;
  char mypasswdbuf[ mypasswdbuf_len];

  if(slurm_load_jobs(0,&pjim,SHOW_ALL)!=0)
    {
      DEBUG3_LOGGER("unable to get allocations informations");
    }
  else
    {
      /* build storage structures */
      nb_allocations=pjim->record_count;
      if( *p_rm_allocations == NULL)
	{
	  *p_rm_allocations=(bridge_rm_allocation_t*)malloc(nb_allocations*sizeof(bridge_rm_allocation_t));
	}
      else
	{
	  if(nb_allocations>*p_rm_allocations_nb)
	    nb_allocations=*p_rm_allocations_nb;
	}
      allocations=*p_rm_allocations;

      if(allocations!=NULL){
	/* fill in storage structure */
	for(j=0;j<nb_allocations;j++)
	  {
	    pji=pjim->job_array+j;
	    /* initialize and set first stage allocation informations */
	    /* init allocation */
	    init_rm_allocation(p_rm_manager,&allocations[real_allocations_nb]);
		
	    /* Allocation ID & Name */
	    char strid[128];
	    snprintf(strid,128,"%d",pji->job_id);
	    allocations[real_allocations_nb].id=strdup(strid);
	    if(pji->name!=NULL)
	      allocations[real_allocations_nb].name=strdup(pji->name);

	    /* Allocation Description */
	    allocations[real_allocations_nb].description=strdup("-");

	    /* Allocation partition */
	    if(pji->partition!=NULL)
	      {
		allocations[real_allocations_nb].partition=strdup(pji->partition);
	      }
		
	    /* Allocation State */
	    enum job_states js=pji->job_state;
	    if(js==JOB_PENDING)
	      {
		allocations[real_allocations_nb].state=BRIDGE_RM_ALLOCATION_STATE_PENDING;
	      }
	    else if( js == JOB_RUNNING || (js & JOB_COMPLETING))
	      {
		if ( pji->batch_flag )
		  allocations[real_allocations_nb].state=BRIDGE_RM_ALLOCATION_STATE_ALLOCATED;
		else
		  allocations[real_allocations_nb].state=BRIDGE_RM_ALLOCATION_STATE_INUSE;
	      }
	    else if(js == JOB_SUSPENDED)
	      {
		allocations[real_allocations_nb].state=BRIDGE_RM_ALLOCATION_STATE_SUSPENDED;
	      }
	    else if(js == (JOB_COMPLETE | JOB_COMPLETING))
	      {
		allocations[real_allocations_nb].state=BRIDGE_RM_ALLOCATION_STATE_COMPLETED;
	      }
	    else if(js == (JOB_CANCELLED | JOB_COMPLETING))
	      {
		allocations[real_allocations_nb].state=BRIDGE_RM_ALLOCATION_STATE_CANCELLED;
	      }
	    else if(js == (JOB_FAILED | JOB_COMPLETING))
	      {
		allocations[real_allocations_nb].state=BRIDGE_RM_ALLOCATION_STATE_CANCELLED;
	      }
	    else if(js == (JOB_TIMEOUT | JOB_COMPLETING))
	      {
		allocations[real_allocations_nb].state=BRIDGE_RM_ALLOCATION_STATE_TIMEOUT;
	      }
	    else if(js == (JOB_NODE_FAIL | JOB_COMPLETING))
	      {
		allocations[real_allocations_nb].state=BRIDGE_RM_ALLOCATION_STATE_NODE_FAILURE;
	      }
	    else
	      {
		allocations[real_allocations_nb].state=BRIDGE_RM_ALLOCATION_STATE_UNKNOWN;
	      }
	    
	    /* Allocation State Reason */
	    allocations[real_allocations_nb].reason=strdup("-");
	    
	    /* Allocation priority */
	    allocations[real_allocations_nb].priority=pji->priority;
	    
	    /* Allocation start time */
	    allocations[real_allocations_nb].submit_time=pji->submit_time;
	    
	    /* Allocation start time */
	    if ( pji->submit_time == pji->start_time && pji->job_state == JOB_PENDING ) {
	      allocations[real_allocations_nb].start_time=INVALID_TIME_VALUE;
	    }
	    else
	      allocations[real_allocations_nb].start_time=pji->start_time;
	    
	    /* Allocation end time */
	    if ( allocations[real_allocations_nb].state != BRIDGE_RM_ALLOCATION_STATE_PENDING )
	      allocations[real_allocations_nb].end_time=pji->end_time;

	    /* Allocation total cores number */
	    allocations[real_allocations_nb].total_cores_nb=pji->num_cpus;
		
	    /* Allocation nodes list && total nodes number */
	    if(pji->nodes!=NULL) {
	      bridge_nodelist_add_nodes(&(allocations[real_allocations_nb].nodelist),pji->nodes);
	      allocations[real_allocations_nb].total_nodes_nb=
		(int)bridge_nodelist_nodes_quantity(&(allocations[real_allocations_nb].nodelist));
	    }
	    /* use required nodes if no nodes still allocated */
	    else
	      allocations[real_allocations_nb].total_nodes_nb=pji->num_nodes;
	    
	    /* Allocation user name */
	    if( getpwuid_r(pji->user_id,&mypasswd,mypasswdbuf,mypasswdbuf_len,&pmypasswd) == 0 
	       && pmypasswd != NULL ) {
	      allocations[real_allocations_nb].username=strdup(mypasswd.pw_name);
	    }
	    else {
	      char* username = (char*) malloc(128*sizeof(char));
	      if ( username != NULL ) {
		snprintf(username,128,"%u",pji->user_id);
	      }
	      allocations[real_allocations_nb].username = username;
	    }
	    
	    /* Allocation allocating session id */
	    allocations[real_allocations_nb].allocating_session_id=pji->alloc_sid;
	    
	    /* Allocation allocating hostname */
	    if(pji->alloc_node!=NULL)
	      allocations[real_allocations_nb].allocating_hostname=strdup(pji->alloc_node);

	    /* Allocation allocating session batchid */
	    if(pji->batch_flag)
	      allocations[real_allocations_nb].allocating_session_batchid=strdup(strid);;

	    /* Allocation allocating pid */
	    //allocations[real_allocations_nb].allocating_pid=atoi();

	    /* uid & gid */
	    allocations[real_allocations_nb].userid=pji->user_id;
	    allocations[real_allocations_nb].groupid=pji->group_id;

/* 	    if( pji->start_time != 0 &&  */
/* 		( pji->start_time != pji->submit_time && pji->job_state != JOB_PENDING ) ){ */

	    if ( allocations[real_allocations_nb].state == BRIDGE_RM_ALLOCATION_STATE_ALLOCATED ||
		 allocations[real_allocations_nb].state == BRIDGE_RM_ALLOCATION_STATE_INUSE ||
		 allocations[real_allocations_nb].state == BRIDGE_RM_ALLOCATION_STATE_SUSPENDED ) {
	      /* elapsed time */
	      time(&current_time);
	      allocations[real_allocations_nb].elapsed_time=current_time-pji->start_time;
	      
	      /* allocated time */
	      allocations[real_allocations_nb].allocated_time=(current_time-pji->start_time-pji->pre_sus_time)*pji->num_cpus;
	    }

	    /* Allocation system time consumed */
	    /* Allocation user time consumed */

	    /* Allocation suspend time */
	    allocations[real_allocations_nb].suspend_time=pji->pre_sus_time;

	    /* Allocation max average memory consumed */

	    /* Apply required filters */
	    out_flag=0;
	    if(!out_flag)
	      {
		/* old allocations are not required. slurm can keep it in memory, so get rid of it */
		out_flag=1;
		if( allocations[real_allocations_nb].state==BRIDGE_RM_ALLOCATION_STATE_ALLOCATED
		    || allocations[real_allocations_nb].state==BRIDGE_RM_ALLOCATION_STATE_INUSE
		    || allocations[real_allocations_nb].state==BRIDGE_RM_ALLOCATION_STATE_PENDING
		    || allocations[real_allocations_nb].state==BRIDGE_RM_ALLOCATION_STATE_SUSPENDED)
		  out_flag=0;
	      }
	    /* if an id is specified, we only get corresponding allocation */
	    if(!out_flag && rm_allocations_rm_id!=NULL)
	      {
		if(strcmp(rm_allocations_rm_id,allocations[real_allocations_nb].id)!=0)
		  out_flag=1;
	      }
	    /* if an user is specified, we only get corresponding allocation */
	    if(!out_flag && username!=NULL)
	      {
		if(strcmp(username,allocations[real_allocations_nb].username)!=0)
		  out_flag=1;
	      }
	    /* if an partition is specified, we only get corresponding allocation */
	    if(!out_flag && rm_partition!=NULL)
	      {
		if(strcmp(rm_partition,allocations[real_allocations_nb].partition)!=0)
		  out_flag=1;
	      }
	    /* if an intersecting nodes list is specified, we only get corresponding allocation */
	    if(!out_flag && intersectingNodes!=NULL)
	      {
		bridge_nodelist_t list;
		bridge_nodelist_init(&list,NULL,0);
		bridge_nodelist_add_nodes(&list,intersectingNodes);
		if(bridge_nodelist_intersects(&(allocations[real_allocations_nb].nodelist),
						       &list)==0){
		  out_flag=1;
		}
		bridge_nodelist_free_contents(&list);
	      }
	    /* if an inclusion nodes list is specified, we only get corresponding allocation */
	    if(!out_flag && includingNodes!=NULL)
	      {
		bridge_nodelist_t list;
		bridge_nodelist_init(&list,NULL,0);
		bridge_nodelist_add_nodes(&list,includingNodes);
		if(bridge_nodelist_includes(&list,&(allocations[real_allocations_nb].nodelist))==0)
		  {
		    out_flag=1;
		  }
		bridge_nodelist_free_contents(&list);
	      }
		
	    /* get jobs informations if allocation has to be kept */
	    if(out_flag==0){
	      if(slurm_get_job_steps(0,pji->job_id,0,&pjsirm,0)==0)
		{
		  char stepid[128];
		  for(k=0;k<pjsirm->job_step_count;k++)
		    {
		      pjsi=pjsirm->job_steps+k;
		      
		      /* an allocation is in use if at least one step is not */
		      /* the batch one */
		      if ( pjsi->step_id.step_id != SLURM_BATCH_SCRIPT )
			allocations[real_allocations_nb].state=BRIDGE_RM_ALLOCATION_STATE_INUSE;
		      
		      snprintf(stepid,128,"%d",pjsi->step_id.step_id);
		      bridge_idlist_add_ids(&(allocations[real_allocations_nb].jobidlist),
					      stepid);
		    }

		  slurm_free_job_step_info_response_msg(pjsirm);
		}

	      /* update real stored allocations counter */
	      real_allocations_nb++;
	    }
	    /* otherwise clean it */
	    else{
	      clean_rm_allocation(p_rm_manager,&allocations[real_allocations_nb]);
	    }

      }

	/* shrink allocations buffer */
	*p_rm_allocations_nb=real_allocations_nb;
	if(real_allocations_nb!=nb_allocations)
	  *p_rm_allocations=(bridge_rm_allocation_t*)realloc(*p_rm_allocations,real_allocations_nb*sizeof(bridge_rm_allocation_t));
	
	/* set return status */
	if(real_allocations_nb>0)
	  {
	    fstatus=0;
	  }
	
      }
      /* clean slurm jobs infos data */
      slurm_free_job_info_msg(pjim);
    }

  return fstatus;
}



int
init_rm_node(bridge_rm_manager_t * p_rm_manager,
	     bridge_rm_node_t* p_rm_node){
	return 0;
}

int clean_rm_node(bridge_rm_manager_t * p_rm_manager,
		  bridge_rm_node_t* p_rm_node){
	return 0;
}

int get_rm_nodes(bridge_rm_manager_t * p_rm_manager,
		 bridge_rm_node_t** p_rm_nodes,
		 int* p_rm_nodes_nb,
		 char* rm_node_name){
	return 0;
}
