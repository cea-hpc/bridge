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
	
	int con_fd;
	struct batch_status * nstat;
	struct batch_status * cstat;
	struct attrl *attr;

	int node_nb=0;
	int stored_node_nb=0;
	
	bridge_batch_node_t* bn;

	con_fd = *((int*)p_batch_manager->private);
	
	/* get node stats */
	nstat = pbs_statnode(con_fd,batch_node_name,NULL,NULL);
	if ( nstat == NULL ) 
		return fstatus;
	
	/* count nodes quantity */
	cstat = nstat ;
	while ( cstat != NULL ) {
		node_nb++;
		cstat=cstat->next;
	}
	
	/* initialized output buffer if required */
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
	
	/* fill output buffer with stats */
	cstat = nstat ;
	while ( cstat != NULL && node_nb ) {
		
		bn = &(*p_p_batch_nodes)[stored_node_nb];
		
		/* node name filter */
		if(batch_node_name!=NULL){
			if(strcmp(batch_node_name,
				  cstat->name)!=0)
				continue;
		}
		
		/* node Name */
		bn->name=strdup(cstat->name);
		
		/* node groups */
		bn->grouplist = NULL ;
		
		/* node state */
		attr = cstat->attribs ;
		while ( attr != NULL ) {
			if (!strcmp(attr->name, ATTR_NODE_state))
			{
				if (!strcmp(attr->value,ND_free))
					bn->state = 
						BRIDGE_BATCH_NODE_STATE_OPENED;
				else if (!strcmp(attr->value,ND_offline))
					bn->state = 
						BRIDGE_BATCH_NODE_STATE_CLOSED;
				else if (!strcmp(attr->value,ND_down))
					bn->state = 
						BRIDGE_BATCH_NODE_STATE_CLOSED;
				else if (!strcmp(attr->value,ND_reserve))
					bn->state = 
						BRIDGE_BATCH_NODE_STATE_BUSY;
				else if (!strcmp(attr->value,ND_job_exclusive))
					bn->state = 
						BRIDGE_BATCH_NODE_STATE_BUSY;
				else if (!strcmp(attr->value,ND_job_sharing))
					bn->state = 
						BRIDGE_BATCH_NODE_STATE_OPENED;
				else if (!strcmp(attr->value,ND_busy))
					bn->state = 
						BRIDGE_BATCH_NODE_STATE_BUSY;
				else if (!strcmp(attr->value,ND_state_unknown))
					bn->state = 
						BRIDGE_BATCH_NODE_STATE_UNKNOWN;
			}
			else if (!strcmp(attr->name, ATTR_NODE_properties)) {
				bn->grouplist = strdup(attr->value);
			}
			else if (!strcmp(attr->name, ATTR_NODE_status)) {

				/* node description */
				bn->description=NULL;
				extract_field(attr->value,"opsys=",
					      &(bn->description));

				/* system Bload */
				extract_float(attr->value,"loadave=",
					      &(bn->one_min_cpu_load));

				/* jobs number */
				char* info=NULL;
				extract_field(attr->value,"jobs=",
					      &(info));
				if ( info != NULL && strcmp(info,"") != 0 ) {
					int n=1;
					char* p = info ;
					p = index(p,',');
					while ( p != NULL ) {
						n++;
						p++;
						p = index(p,',');
					}
					bn->jobs_nb = n;
				}
				if ( info != NULL )
					free(info);

				/* swap and physical mem info */
				uint32_t gmem, avail_gmem;
				extract_mem(attr->value,"physmem=",
					    &(bn->total_mem));
				extract_mem(attr->value,"totmem=",&(gmem));
				bn->total_swap = gmem - bn->total_mem ;

				/* estimate physical free mem assuming that */
				/* it will be completely used before */
				/* swap is touched */
				extract_mem(attr->value,"availmem=",
					    &(avail_gmem));
				if ( avail_gmem > bn->total_swap ) {
					bn->free_mem = avail_gmem 
						- bn->total_swap ;
					bn->free_swap = bn->total_swap ;
				}
				else {
					bn->free_mem = 0 ;
					bn->free_swap = avail_gmem ;
				}
				
			}

			attr = attr->next ;
		}
		
		bn->running_jobs_nb = bn->jobs_nb ;
		bn->usersuspended_jobs_nb = 0 ;
		bn->syssuspended_jobs_nb = 0 ;
		
		bn->jobs_nb_limit = NO_LIMIT ;
		bn->perUser_jobs_nb_limit = NO_LIMIT ;

		bn->free_tmp = 0 ;
		bn->total_tmp = 0 ;

		cstat=cstat->next;
		node_nb--;
		stored_node_nb++;
	}

	pbs_statfree(nstat);

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

	return fstatus ;

}
