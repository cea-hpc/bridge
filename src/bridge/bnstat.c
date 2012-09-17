/*****************************************************************************\
 *  src/bridge/bnstat.c - 
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

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "bridge/bridge.h"

#define PROG_VERSION  "1.0.1"

/*!
 * \brief display batch node informations on file stream in an extended form 
 * \internal
 *
 * \param stream FILE* on which to write output
 * \param bq pointer on a batch node structure to display
 *
 * \retval 0 on success
 * \retval -1 on failure
 */
int display_bridge_batch_node_on_file_stream(FILE* stream,bridge_batch_node_t* bq);

/*!
 * \brief display batch node informations on file stream in an classic form 
 * \internal
 *
 * \param stream FILE* on which to write output
 * \param bq pointer on a batch node structure to display
 *
 * \retval 0 on success
 * \retval -1 on failure
 */
int display_classic_bridge_batch_node_on_file_stream(FILE * stream,bridge_batch_node_t* bq);

/*!
 * \brief display required informations about batch node on file stream 
 * \internal
 *
 * \param stream FILE* on which to write output
 * \param bq pointer on a batch node structure to display
 * \param output_fields comma separated list of informations to display
 * \param separator string to write on stream between each required information
 *
 * \retval 0 on success
 * \retval -1 on failure
 */
int display_by_fields_bridge_batch_node_on_file_stream(FILE* stream,bridge_batch_node_t* bq,char* output_fields,char* separator);

int main(int argc,char **argv){

  int fstatus=-1;
  int status;

  char* progname;
  char* cb_version;

  bridge_manager_t manager;

  bridge_batch_node_t* p_batch_node_array=NULL;
  bridge_batch_node_t* p_batch_node=NULL;
  int batch_node_nb=0;
  int i,j;
  int verbosity=0;

  FILE* output_stream=stdout;

  char* node_name=NULL;

  char* options_desc=
    "\t-h\t\t\tPrint this message\n"
    "\t-c\t\t\tDisplay results in classic format\n"
    "\t-n node\t\t\tDisplay results concerning this batch node\n"
    "\t-o fields\t\tDisplay informations using fields format (use -o list for available fields, -o all to get infos for all fields)\n"
    "\t-s separator\t\tWhen used with -o options, enables to change results fields separator\n\t\t\t\t(default is a single spaced string)\n"
    "\t-v\t\t\tWhen used with -o options, the first output line displays list of selected fields\n"
    "\t-V\t\t\tPrint bridge and app versions and exit\n";

  char* fields_list="\n"
    "\tname          : node name\n"
    "\tdesc          : node description\n"
    "\tgroups        : node groups name\n"
    "\tstate         : node state (open|closed|busy|...)\n"
    "\tjobs          : current number of jobs in this node\n"
    "\tjobslim       : max number of jobs in this node\n"
    "\tuserjobslim   : max number of jobs per user in this node\n"
    "\trun           : number of running jobs in this node\n"
    "\tususp         : number of user suspended jobs in this node\n"
    "\tssusp         : number of system suspended jobs in this node\n"
    "\tfreemem       : free mem space (in Mo) on this node\n"
    "\ttotalmem      : total mem space (in Mo) on this node\n"
    "\tfreeswap      : free swap space (in Mo) on this node\n"
    "\ttotalswap     : total swap space (in Mo) on this node\n"
    "\tfreetmp       : free tmp space (in Mo) on this node\n"
    "\ttotaltmp      : total tmp space (in Mo) on this node\n"
    "\t1mload        : one minute average cpu load on this node\n";


  char* output_fields=NULL;
  char* separator=NULL;

#define EXTENDED_DISPLAY 0
#define COMPOSITE_DISPLAY 1
#define CLASSIC_DISPLAY 2
  int display_mode=EXTENDED_DISPLAY;

  char * optstring="hn:o:s:cvV";
  char option;

  progname=strrchr(argv[0],'/');
  if(progname==NULL)
    {
      progname=argv[0];
    }
  else
    {
      progname++;
    }

  while((option = getopt(argc,argv,optstring)) != -1) {
    switch(option){
    case 'n' :
      node_name=strdup(optarg);
      break;
    case 'c' :
      display_mode=CLASSIC_DISPLAY;
      break;
    case 'v' :
      verbosity++;
      break;
    case 'o' :
      if(strcmp(optarg,"list")==0){
	fprintf(stdout,"\nAvailable fields are :\n%s\n",fields_list);
	goto exit;
      }
      else if(strcmp(optarg,"all")==0){
	output_fields=strdup("name,desc,groups,state,jobs,jobslim,userjobslim,run,ususp,ssusp,freemem,totalmem,freeswap,totalswap,freetmp,totaltmp,1mload");
      }
      else
	output_fields=strdup(optarg);
      display_mode=COMPOSITE_DISPLAY;
      break;
    case 's' :
      separator=strdup(optarg);
      break;
    case 'V' :
      cb_version=bridge_version();
      if(cb_version!=NULL)
	fprintf(stdout,"%s version %s (bridge-%s)\n",progname,PROG_VERSION,cb_version);
      else
	fprintf(stdout,"%s version %s (bridge-unknown)\n",progname,PROG_VERSION);
      goto exit;
      break;
    case 'h' :
    default :
      fprintf(stdout,"\nUsage : %s [-h] [-c] [-n node] [-o fields [-s separator]] \n\n",argv[0]);
      fprintf(stdout,"%s\n",options_desc);
      exit(1);
      break;
    }
  }

  // initialize bridge configuration
  if(bridge_init_manager(&manager)==0){
 
    /* Get sequential jobs informations */
    status=bridge_get_batch_nodes(&manager,&p_batch_node_array,&batch_node_nb,node_name);
    if(status==0){

      switch(display_mode){

      case EXTENDED_DISPLAY :
	for(i=0;i<batch_node_nb;i++){
	  display_bridge_batch_node_on_file_stream(stdout,p_batch_node_array+i);
	}
	break;

      case CLASSIC_DISPLAY :
	display_classic_bridge_batch_node_on_file_stream(stdout,NULL);
	for(i=0;i<batch_node_nb;i++){
	  display_classic_bridge_batch_node_on_file_stream(stdout,p_batch_node_array+i);
	}
	break;

      case COMPOSITE_DISPLAY :
	if(verbosity)
	  display_by_fields_bridge_batch_node_on_file_stream(stdout,NULL,
								 output_fields,separator);
	for(i=0;i<batch_node_nb;i++){
	  display_by_fields_bridge_batch_node_on_file_stream(stdout,p_batch_node_array+i,
								 output_fields,separator);
	}
	break;

      }

      /*
	Destroy batch nodes structures
	--------------------------------
      */
      for(i=0;i<batch_node_nb;i++){
	bridge_clean_batch_node(&manager,p_batch_node_array+i);
      }
      free(p_batch_node_array);

      fstatus=0;
    }
    bridge_clean_manager(&manager);
  }
  else{
    fstatus=1;
  }

  exit :
    
  if(node_name!=NULL)
    free(node_name);
  if(output_fields!=NULL)
    free(output_fields);
  if(separator!=NULL)
    free(separator);

  return fstatus;

}



int display_bridge_batch_node_on_file_stream(FILE* stream,bridge_batch_node_t* bq){

  int i;
  
  fprintf(stream,
	  "-------------------------------------------------------\n");
  if(bq->name!=NULL)
    fprintf(stream,"Name \t\t\t: %s\n",bq->name);
  else
    fprintf(stream,"Name \t\t\t: -\n");

  if(bq->description!=NULL)
    fprintf(stream,"Desc \t\t\t: %s\n",bq->description);
  else
    fprintf(stream,"Desc \t\t\t: -\n");

  if(bq->grouplist!=NULL)
    fprintf(stream,"Groups \t\t\t: %s\n",bq->grouplist);
  else
    fprintf(stream,"Groups \t\t\t: -\n");

  char* status;
  switch(bq->state){
  case BRIDGE_BATCH_NODE_STATE_OPENED :
    status="open";
    break;
  case BRIDGE_BATCH_NODE_STATE_CLOSED :
    status="closed";
    break;
  case BRIDGE_BATCH_NODE_STATE_BUSY :
    status="busy";
    break;
  case BRIDGE_BATCH_NODE_STATE_UNAVAILABLE :
    status="unavailable";
    break;
  case BRIDGE_BATCH_NODE_STATE_UNREACHABLE :
    status="unreachable";
    break;
  case BRIDGE_BATCH_NODE_STATE_UNLICENSED :
    status="unlicensed";
    break;
  default:
    status="unknown";
  }
  fprintf(stream,"State \t\t\t: %s\n",status);

  if(bq->jobs_nb_limit!=NO_LIMIT)
    fprintf(stream,"Max Jobs Nb \t\t: %u\n",bq->jobs_nb_limit);
  else
    fprintf(stream,"Max Jobs Nb \t\t: -\n");

  if(bq->perUser_jobs_nb_limit!=NO_LIMIT)
    fprintf(stream,"Per User Job limit \t: %u\n",bq->perUser_jobs_nb_limit);
  else
    fprintf(stream,"Per User Job limit \t: -\n");

  fprintf(stream,"Jobs Nb \t\t: %u\n",bq->jobs_nb);

  fprintf(stream,"  RUNNING \t\t: %u\n",bq->running_jobs_nb);

  fprintf(stream,"  USUSPENDED \t\t: %u\n",bq->usersuspended_jobs_nb);

  fprintf(stream,"  SSUSPENDED \t\t: %u\n",bq->syssuspended_jobs_nb);

  if(bq->free_mem!=NO_LIMIT)
    fprintf(stream,"Free mem \t\t: %u\n",bq->free_mem);
  else
    fprintf(stream,"Free mem \t\t: -\n");

  if(bq->total_mem!=NO_LIMIT)
    fprintf(stream,"Total mem \t\t: %u\n",bq->total_mem);
  else
    fprintf(stream,"Total mem \t\t: -\n");

  if(bq->free_swap!=NO_LIMIT)
    fprintf(stream,"Free swap \t\t: %u\n",bq->free_swap);
  else
    fprintf(stream,"Free swap \t\t: -\n");

  if(bq->total_swap!=NO_LIMIT)
    fprintf(stream,"Total swap \t\t: %u\n",bq->total_swap);
  else
    fprintf(stream,"Total swap \t\t: -\n");

  if(bq->free_tmp!=NO_LIMIT)
    fprintf(stream,"Free tmp \t\t: %u\n",bq->free_tmp);
  else
    fprintf(stream,"Free tmp \t\t: -\n");

  if(bq->total_tmp!=NO_LIMIT)
    fprintf(stream,"Total tmp \t\t: %u\n",bq->total_tmp);
  else
    fprintf(stream,"Total tmp \t\t: -\n");

    fprintf(stream,"1 min cpu load \t\t: %f\n",bq->one_min_cpu_load);
  

  fprintf(stream,
	  "-------------------------------------------------------\n");
  return 0;
}

int display_classic_bridge_batch_node_on_file_stream(FILE * stream,bridge_batch_node_t* bn){
  int fstatus=0;
  
  if(!bn){
    fprintf(stream,"%-16s %-16s %-16s %-16s %-16s %-16s\n",
	    "----------------",
	    "----------------",
	    "----------------",
	    "----------------",
	    "----------------",
	    "----------------");
    fprintf(stream,"%-16s %-16s %-16s %-16s %-16s %-16s\n","Name","State","B.session #","B.session # lim","1m load","Groups");
    fprintf(stream,"%-16s %-16s %-16s %-16s %-16s %-16s\n",
	    "----------------",
	    "----------------",
	    "----------------",
	    "----------------",
	    "----------------",
	    "----------------");
  }
  else{
    char* status;
    switch(bn->state){
    case BRIDGE_BATCH_NODE_STATE_OPENED :
      status="open";
      break;
    case BRIDGE_BATCH_NODE_STATE_CLOSED :
      status="closed";
      break;
    case BRIDGE_BATCH_NODE_STATE_BUSY :
      status="busy";
      break;
    default:
      status="unknown";
    }
    fprintf(stream,"%-16s %-16s %16d %16d %16f %-16s\n",
	    (bn->name!=NULL)?bn->name:"-",
	    status,
	    bn->jobs_nb,
	    bn->jobs_nb_limit,
	    bn->one_min_cpu_load,
	    (bn->grouplist!=NULL)?bn->grouplist:"-");
  }



  return fstatus;
}

int display_by_fields_bridge_batch_node_on_file_stream(FILE* stream,bridge_batch_node_t* bq,char* output_fields,char* separator){

  int status=0;

  int i;
  int display_header=0;

  char* token=NULL;

  int token_nb;

  if(separator==NULL)
    separator=" ";

  if(bq==NULL)
    display_header=1;

  status=bridge_common_string_get_tokens_quantity(output_fields,",",&token_nb);
  if(status==0){
    for(i=1;i<=token_nb;i++){
      if(bridge_common_string_get_token(output_fields,",",i,&token)<0){
	continue;
      }
      else{
	if(display_header==1){
	  fprintf(stream,"%s",token);
	}

	/* Node name */
	else if(strcmp(token,"name")==0){
	  if(bq->name!=NULL)
	    fprintf(stream,"%s",bq->name);
	  else
	    fprintf(stream,"-");
	}
	/* Node description */
	else if(strcmp(token,"desc")==0){
	  if(bq->description!=NULL)
	    fprintf(stream,"%s",bq->description);
	  else
	    fprintf(stream,"-");
	}
	/* Node description */
	else if(strcmp(token,"groups")==0){
	  if(bq->grouplist!=NULL)
	    fprintf(stream,"%s",bq->grouplist);
	  else
	    fprintf(stream,"-");
	}
	/* Node state */
	else if(strcmp(token,"state")==0){
	  char* status;
	  switch(bq->state){
	  case BRIDGE_BATCH_NODE_STATE_OPENED :
	    status="open";
	    break;
	  case BRIDGE_BATCH_NODE_STATE_CLOSED :
	    status="closed";
	    break;
	  case BRIDGE_BATCH_NODE_STATE_BUSY :
	    status="busy";
	    break;
	  case BRIDGE_BATCH_NODE_STATE_UNAVAILABLE :
	    status="unavailable";
	    break;
	  case BRIDGE_BATCH_NODE_STATE_UNREACHABLE :
	    status="unreachable";
	    break;
	  case BRIDGE_BATCH_NODE_STATE_UNLICENSED :
	    status="unlicensed";
	    break;
	  default :
	    status="unknown";
	  }
	  fprintf(stream,"%s",status);
	}
	/* Node jobs nb */
	else if(strcmp(token,"jobs")==0){
	  fprintf(stream,"%u",bq->jobs_nb);
	}
	/* Node jobs nb limit */
	else if(strcmp(token,"jobslim")==0){
	  if(bq->jobs_nb_limit!=NO_LIMIT)
	    fprintf(stream,"%u",bq->jobs_nb_limit);
	  else
	    fprintf(stream,"-");
	}
	/* Node per user jobs nb limit */
	else if(strcmp(token,"userjobslim")==0){
	  if(bq->perUser_jobs_nb_limit!=NO_LIMIT)
	    fprintf(stream,"%u",bq->perUser_jobs_nb_limit);
	  else
	    fprintf(stream,"-");
	}
	/* Node running job */
	else if(strcmp(token,"run")==0){
	  fprintf(stream,"%u",bq->running_jobs_nb);
	}
	/* Node user suspended job */
	else if(strcmp(token,"ususp")==0){
	  fprintf(stream,"%u",bq->usersuspended_jobs_nb);
	}
	/* Node system suspended job */
	else if(strcmp(token,"ssusp")==0){
	  fprintf(stream,"%u",bq->syssuspended_jobs_nb);
	}
	/* Node free swap space (in Mo) */
	else if(strcmp(token,"freeswap")==0){
	  fprintf(stream,"%u",bq->free_swap);
	}
	/* Node total swap space (in Mo) */
	else if(strcmp(token,"totalswap")==0){
	  fprintf(stream,"%u",bq->total_swap);
	}
	/* Node free mem space (in Mo) */
	else if(strcmp(token,"freemem")==0){
	  fprintf(stream,"%u",bq->free_mem);
	}
	/* Node total mem space (in Mo) */
	else if(strcmp(token,"totalmem")==0){
	  fprintf(stream,"%u",bq->total_mem);
	}
	/* Node free tmp space (in Mo) */
	else if(strcmp(token,"freetmp")==0){
	  fprintf(stream,"%u",bq->free_tmp);
	}
	/* Node total tmp space (in Mo) */
	else if(strcmp(token,"totaltmp")==0){
	  fprintf(stream,"%u",bq->total_tmp);
	}
	/* Node 1 minute CPU load */
	else if(strcmp(token,"1mload")==0){
	  fprintf(stream,"%f",bq->one_min_cpu_load);
	}
	/* error */
	else{
	  fprintf(stream,"?");
	}

	free(token);
	token=NULL;
	if(i!=token_nb)
	  fprintf(stream,"%s",separator);
      }
    }
  }
  
  fprintf(stream,"\n");

  return 0;
}
