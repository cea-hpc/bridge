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
init_batch_node(bridge_batch_manager_t* p_batch_manager,
		bridge_batch_node_t* p_batch_node)
{
	int fstatus=-1;
	
	if(p_batch_node!=NULL){
		
		p_batch_node->name=NULL;
		p_batch_node->description=NULL;
		p_batch_node->grouplist=NULL;
		
		p_batch_node->state=BRIDGE_BATCH_NODE_STATE_UNKNOWN;
		
		p_batch_node->running_jobs_nb=0;
		p_batch_node->usersuspended_jobs_nb=0;
		p_batch_node->syssuspended_jobs_nb=0;
		p_batch_node->jobs_nb=0;
		
		p_batch_node->jobs_nb_limit=NO_LIMIT;
		p_batch_node->perUser_jobs_nb_limit=NO_LIMIT;
		
		p_batch_node->free_swap=0;
		p_batch_node->free_tmp=0;
		p_batch_node->free_mem=0;
		
		p_batch_node->total_swap=0;
		p_batch_node->total_tmp=0;
		p_batch_node->total_mem=0;
		
		p_batch_node->one_min_cpu_load=0.0;
  
		fstatus=0;
	}

	return fstatus;
}

int
clean_batch_node(bridge_batch_manager_t* p_batch_manager,
		 bridge_batch_node_t* p_batch_node)
{
	int fstatus=-1;

	if(p_batch_node!=NULL){

		if(p_batch_node->name!=NULL)
			free(p_batch_node->name);
		if(p_batch_node->description!=NULL)
			free(p_batch_node->description);
		if(p_batch_node->grouplist!=NULL)
			free(p_batch_node->grouplist);

		fstatus=0;
	}

	return fstatus;
}

int
get_batch_nodes(bridge_batch_manager_t* p_batch_manager,
		bridge_batch_node_t** p_p_batch_nodes,
		int* p_batch_nodes_nb, char* batch_node_name)
{
	int fstatus=-1;

	int i,j;

	node_info_msg_t* pnim;
	node_info_t* pni;

	job_info_msg_t* pjim;
	job_info_t* pji;

	int node_nb=0;
	int stored_node_nb=0;
	
	bridge_batch_node_t* bn;

	/* get jobs stats */
	if (slurm_load_jobs(0,&pjim,SHOW_ALL) != 0) {
		DEBUG3_LOGGER("unable to get slurm jobs infos");
		pjim=NULL;
		goto exit;
	}

	/* get node stats */
	if (slurm_load_node(0,&pnim,SHOW_ALL) != 0) {
		DEBUG3_LOGGER("unable to get slurm nodes infos");
		pnim=NULL;
		slurm_free_job_info_msg(pjim);
		goto exit;
	}
	node_nb = pnim->record_count;

	/* build/initialize storage structures */
	if(*p_p_batch_nodes!=NULL){
		if(*p_batch_nodes_nb<node_nb)
			node_nb=*p_batch_nodes_nb;
	}
	else{
		*p_p_batch_nodes=(bridge_batch_node_t*)
			malloc(node_nb*(sizeof(bridge_batch_node_t)+1));
		if(*p_p_batch_nodes==NULL){
			*p_batch_nodes_nb=0;
			node_nb=*p_batch_nodes_nb;
		}
		else{
			*p_batch_nodes_nb=node_nb;
		}
	}
	stored_node_nb=0;
	
	/* fill structures */
	for (i=0;i<pnim->record_count && stored_node_nb<node_nb;i++) {
		
		/* get node pointer */
		pni=pnim->node_array+i;
		
		if (pni->name == NULL)
			continue;
		
		/* queue name filter */
		if (batch_node_name != NULL &&
		    strcmp(batch_node_name,pni->name) != 0)
			continue;
		
		bn = &(*p_p_batch_nodes)[stored_node_nb];
		
		/* put default values */
		init_batch_node(p_batch_manager,bn);

		/* queue Name */
		bn->name=strdup(pni->name);

		switch(pni->node_state & NODE_STATE_BASE) {
			
		case NODE_STATE_ALLOCATED :
			if (pni->node_state & NODE_STATE_DRAIN )
				bn->state=BRIDGE_BATCH_NODE_STATE_CLOSED;
			else
				bn->state=BRIDGE_BATCH_NODE_STATE_BUSY;
			break;
			
		case NODE_STATE_IDLE :
			if (pni->node_state & NODE_STATE_DRAIN )
				bn->state=BRIDGE_BATCH_NODE_STATE_CLOSED;
			else
				bn->state=BRIDGE_BATCH_NODE_STATE_OPENED;
			break;

		case NODE_STATE_DOWN :
			bn->state=BRIDGE_BATCH_NODE_STATE_UNAVAILABLE;
			break;

		default :
		case NODE_STATE_UNKNOWN :
			bn->state=BRIDGE_BATCH_NODE_STATE_UNKNOWN;
			break;

		}
		if ( pni->node_state & NODE_STATE_NO_RESPOND )
			bn->state=BRIDGE_BATCH_NODE_STATE_UNREACHABLE;
		
		bn->free_swap=0;
		bn->free_tmp=0;
		bn->free_mem=0;
		
		bn->total_swap = 0;
		bn->total_tmp = pni->tmp_disk;
		bn->total_mem = pni->real_memory;
		
		bn->one_min_cpu_load=0.0;

		bn->running_jobs_nb=0;
		bn->usersuspended_jobs_nb=0;
		bn->syssuspended_jobs_nb=0;
		bn->jobs_nb=0;
		
		bn->jobs_nb_limit=NO_LIMIT;
		bn->perUser_jobs_nb_limit=NO_LIMIT;

		/* slurm */
		for ( j=0 ; j < pjim->record_count ; j++ ) {
		  
		  pji=pjim->job_array+j;
		  
		  if ( ( pji->job_state & JOB_STATE_BASE ) == JOB_PENDING )
		          continue;

		  if ( pji->nodes == NULL )
		          continue;

		  hostlist_t hl = slurm_hostlist_create(pji->nodes);
		  if ( slurm_hostlist_find(hl,pni->name) == -1 ) {
			  slurm_hostlist_destroy(hl);
		          continue;
		  }
		  slurm_hostlist_destroy(hl);
		  		  
		  switch ( pji->job_state & JOB_STATE_BASE ) {
		  case JOB_RUNNING :
		    bn->running_jobs_nb++;
		    bn->jobs_nb++;
		    break;
		  case JOB_SUSPENDED :
		    bn->syssuspended_jobs_nb++;
		    bn->jobs_nb++;
		    break;
		  default :
		    break;
		  }
		  
		}

		stored_node_nb++;
	}

	fstatus=0;

	if(stored_node_nb<node_nb){
		*p_p_batch_nodes=(bridge_batch_node_t*)
			realloc(*p_p_batch_nodes,
				stored_node_nb*(sizeof(bridge_batch_node_t)+1));
		if(*p_p_batch_nodes==NULL)
			*p_batch_nodes_nb=0;
		else
			*p_batch_nodes_nb=stored_node_nb;
	}

	/* free slurm partition informations */
	slurm_free_node_info_msg(pnim);
	slurm_free_job_info_msg(pjim);

exit:
	return fstatus ;
}
