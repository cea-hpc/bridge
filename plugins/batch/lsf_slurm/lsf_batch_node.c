/*****************************************************************************\
 *  plugins/batch/lsf_slurm/lsf_batch_node.c - 
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

#define DEBUG_LOGGER debug
#define DEBUG2_LOGGER debug2
#define DEBUG3_LOGGER debug3
#define ERROR_LOGGER error

int init_batch_node(bridge_batch_manager_t* p_batch_manager,
		    bridge_batch_node_t* p_batch_node){

  int fstatus=-1;

  if(p_batch_node!=NULL){

    p_batch_node->name=NULL; /* batch node name */
    p_batch_node->description=NULL; /* More info about this node */
    p_batch_node->grouplist=NULL; /* More info about this node */

    p_batch_node->state=BRIDGE_BATCH_NODE_STATE_UNKNOWN; /* Node state (open/closed)*/

    p_batch_node->running_jobs_nb=0; /* number of running jobs on this node */
    p_batch_node->usersuspended_jobs_nb=0; /* number of user suspended jobs on this node */
    p_batch_node->syssuspended_jobs_nb=0; /* number of system suspended jobs on this node */
    p_batch_node->jobs_nb=0; /* number of jobs on this node */

    p_batch_node->jobs_nb_limit=NO_LIMIT; /* max number of jobs on this node */
    p_batch_node->perUser_jobs_nb_limit=NO_LIMIT; /* max number of jobs allowed to run per user */

    p_batch_node->free_swap=0; /*!< available swap space (in Mo) on this node */
    p_batch_node->free_tmp=0; /*!< available tmp space (in Mo) on this node */
    p_batch_node->free_mem=0; /*!< available memory space (in Mo) on this node */

    p_batch_node->total_swap=0; /*!< max swap space (in Mo) on this node */
    p_batch_node->total_tmp=0; /*!< max tmp space (in Mo) on this node */
    p_batch_node->total_mem=0; /*!< max memory space (in Mo) on this node */

    p_batch_node->one_min_cpu_load=0.0; /*!< one minute cpu load average */
  
    fstatus=0;
  }

  return fstatus;

}

int clean_batch_node(bridge_batch_manager_t* p_batch_manager,
		     bridge_batch_node_t* p_batch_node){

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



/*
  Get batch nodes information
  -------------------------------
  You don't have to create all bridge_batch_node_t structure, you just have to set parameters
  according to the following rules :

  if batch_nodes_batch_ids equals NULL or "" or "all", get all current nodes, otherwise get only batch_nodes by
  given batch_id

  if p_batch_nodes==NULL :
  - set total batch nodes number in p_batch_nodes_nb
  - allocate a bridge_batch_node_t** containing *p_batch_nodes_nb bridge_batch_node_t*
  - fill the *p_batch_nodes_nb bridge_batch_node_t
  else :
  - get max batch nodes number in *p_batch_nodes_nb
  - fill the *p_batch_nodes_nb bridge_batch_node_t if possible
  - update value of *p_batch_nodes_nb according to 


  Returns :
  0 on success
  1 on succes, but p_nodes_nb contains a new valid value for nodes_nb
  -1 on error

  On succes, you 'll have to clean all nodes with bridge_rmi_clean_node(...) before
  freeing *p_nodes

*/
int get_batch_nodes(bridge_batch_manager_t* p_batch_manager,
		    bridge_batch_node_t** p_p_batch_nodes,
		    int* p_batch_nodes_nb, char* batch_node_name){

  int fstatus=-1;
  int status;

  int i,j,k;

  char buffer[256];

  char* node_array[1];

  char* reason;

  struct hostInfoEnt* p_nodeInfo=NULL;
  struct groupInfoEnt* p_grpInfo=NULL;
  int node_nb=0;
  int grp_nb=0;
  int stored_node_nb=0;

  char* node_grouplist_item;
  size_t node_grouplist_default_length=128;
  size_t node_grouplist_length;

  node_array[0]=NULL;
  
  /*
   * Check that batch system is running or exit with error 1
   */
  if(!ls_getclustername()){
    DEBUG3_LOGGER("unable to get cluster information\n");
    return 1;
  }

  p_nodeInfo=lsb_hostinfo(NULL,&node_nb);
  p_grpInfo=lsb_hostgrpinfo(NULL,&grp_nb,GRP_ALL);
  
  if(p_nodeInfo==NULL)
    DEBUG3_LOGGER("unable to get nodes information\n");
  else{
    if(*p_p_batch_nodes!=NULL){
      if(*p_batch_nodes_nb<node_nb)
	node_nb=*p_batch_nodes_nb;
    }
    else{
      *p_p_batch_nodes=(bridge_batch_node_t*)malloc(node_nb*(sizeof(bridge_batch_node_t)+1));
      if(*p_p_batch_nodes==NULL){
	*p_batch_nodes_nb=0;
	node_nb=*p_batch_nodes_nb;
      }
      else{
	*p_batch_nodes_nb=node_nb;
      }
    }
    
    stored_node_nb=0;

    for(i=0;i<node_nb;i++){

      if(batch_node_name!=NULL){
	if(strcmp(batch_node_name,p_nodeInfo[i].host)!=0)
	  continue;
      }

      init_batch_node(p_batch_manager,(*p_p_batch_nodes)+stored_node_nb);

      /* Node Name */
      (*p_p_batch_nodes)[stored_node_nb].name=strdup(p_nodeInfo[i].host);

      /* Node description */
      (*p_p_batch_nodes)[stored_node_nb].description=strdup("Batch node");

      /* Node groups */
      node_grouplist_length=node_grouplist_default_length;
      (*p_p_batch_nodes)[stored_node_nb].grouplist=(char*)malloc(node_grouplist_length);
      if((*p_p_batch_nodes)[stored_node_nb].grouplist!=NULL){
	(*p_p_batch_nodes)[stored_node_nb].grouplist[0]='\0';
	for(k=0;k<grp_nb;k++){
	  node_grouplist_item=strstr(p_grpInfo[k].memberList,(*p_p_batch_nodes)[stored_node_nb].name);
	  if(node_grouplist_item!=NULL)
	    if(*(node_grouplist_item+strlen((*p_p_batch_nodes)[stored_node_nb].name)) == '\0' ||
	       *(node_grouplist_item+strlen((*p_p_batch_nodes)[stored_node_nb].name)) == ' '){
	      bridge_common_string_appends_and_extends(&((*p_p_batch_nodes)[stored_node_nb].grouplist),
						    &node_grouplist_length,128,p_grpInfo[k].group," ");
	    }
	}
      }
      if(strlen((*p_p_batch_nodes)[stored_node_nb].grouplist)==0){
	free((*p_p_batch_nodes)[stored_node_nb].grouplist);
	(*p_p_batch_nodes)[stored_node_nb].grouplist=NULL;
      }

      /* Node state */
      if(p_nodeInfo[i].hStatus==HOST_STAT_OK ||
	 (p_nodeInfo[i].hStatus & HOST_STAT_LOCKED)
	 ){
	(*p_p_batch_nodes)[stored_node_nb].state=BRIDGE_BATCH_NODE_STATE_OPENED;
      }
      else if( (p_nodeInfo[i].hStatus & HOST_STAT_DISABLED) || 
	       (p_nodeInfo[i].hStatus & HOST_STAT_WIND)
	       ){
	(*p_p_batch_nodes)[stored_node_nb].state=BRIDGE_BATCH_NODE_STATE_CLOSED;
      }
      else if( (p_nodeInfo[i].hStatus & HOST_STAT_BUSY) || 
	       (p_nodeInfo[i].hStatus & HOST_STAT_FULL)
	       ){
	(*p_p_batch_nodes)[stored_node_nb].state=BRIDGE_BATCH_NODE_STATE_BUSY;
      }
      else if( (p_nodeInfo[i].hStatus & HOST_STAT_UNAVAIL) ||
	       (p_nodeInfo[i].hStatus & HOST_STAT_NO_LIM)
	       ){
	(*p_p_batch_nodes)[stored_node_nb].state=BRIDGE_BATCH_NODE_STATE_UNAVAILABLE;
      }
      else if( (p_nodeInfo[i].hStatus & HOST_STAT_UNREACH)){
	(*p_p_batch_nodes)[stored_node_nb].state=BRIDGE_BATCH_NODE_STATE_UNREACHABLE;
      }
      else if( (p_nodeInfo[i].hStatus & HOST_STAT_UNLICENSED)){
	(*p_p_batch_nodes)[stored_node_nb].state=BRIDGE_BATCH_NODE_STATE_UNLICENSED;
      }
      else{
	(*p_p_batch_nodes)[stored_node_nb].state=BRIDGE_BATCH_NODE_STATE_UNKNOWN;
      }

      /* Get information only if host is open or closed or busy */
      if((*p_p_batch_nodes)[stored_node_nb].state == BRIDGE_BATCH_NODE_STATE_OPENED ||
	 (*p_p_batch_nodes)[stored_node_nb].state == BRIDGE_BATCH_NODE_STATE_CLOSED ||
	 (*p_p_batch_nodes)[stored_node_nb].state == BRIDGE_BATCH_NODE_STATE_BUSY){
	/* running jobs number */
	(*p_p_batch_nodes)[stored_node_nb].running_jobs_nb=p_nodeInfo[i].numRUN;

	/* user suspended jobs number */
	(*p_p_batch_nodes)[stored_node_nb].usersuspended_jobs_nb=p_nodeInfo[i].numUSUSP;

	/* system suspended jobs number */
	(*p_p_batch_nodes)[stored_node_nb].syssuspended_jobs_nb=p_nodeInfo[i].numSSUSP;

	/* total jobs number */
	(*p_p_batch_nodes)[stored_node_nb].jobs_nb=p_nodeInfo[i].numJobs;

	/* max jobs number */
	(*p_p_batch_nodes)[stored_node_nb].jobs_nb_limit=(p_nodeInfo[i].maxJobs==INT_MAX)?NO_LIMIT:p_nodeInfo[i].maxJobs;

	/* max jobs number per user */
	(*p_p_batch_nodes)[stored_node_nb].perUser_jobs_nb_limit=(p_nodeInfo[i].userJobLimit==INT_MAX)?NO_LIMIT:p_nodeInfo[i].userJobLimit;

	/* free swap space (in Mo) */
	(*p_p_batch_nodes)[stored_node_nb].free_swap=p_nodeInfo[i].realLoad[SWP];

	/* free tmp space (in Mo) */
	(*p_p_batch_nodes)[stored_node_nb].free_tmp=p_nodeInfo[i].realLoad[TMP];

	/* free mem space (in Mo) */
	(*p_p_batch_nodes)[stored_node_nb].free_mem=p_nodeInfo[i].realLoad[MEM];

	/* one minute cpu load */
	(*p_p_batch_nodes)[stored_node_nb].one_min_cpu_load=p_nodeInfo[i].realLoad[R1M]*100;
      }

      stored_node_nb++;

    }
    
    fstatus=0;
  }

  if(stored_node_nb<node_nb){
    *p_p_batch_nodes=(bridge_batch_node_t*)realloc(*p_p_batch_nodes,stored_node_nb*(sizeof(bridge_batch_node_t)+1));
    if(*p_p_batch_nodes==NULL)
      *p_batch_nodes_nb=0;
    else
      *p_batch_nodes_nb=stored_node_nb;
  }

  return fstatus;

}
