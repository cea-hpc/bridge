/*****************************************************************************\
 *  src/bridge/rmpstat.c - 
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

#include <limits.h>
#include <time.h>

#include "bridge/bridge.h"

#define PROG_VERSION  "1.0.1"

/* local functions */
int display_classic_bridge_rm_partition_on_file_stream(FILE * stream,
						       bridge_rm_partition_t* rmp);

// name,desc,partition,state,reason,priority,user,uid,gid,
// subtime,starttime,endtime,susptime,etime,atime,stime,utime,
// subhost,subsid,subpid,ucores_nb,cores_nb,unodes_nb,nodes_nb,memused,nodes,jobids

/*
  Print partition contents on given file stream
  --------------------------------------------

  if p_partition==NULL, it prints display header

  Returns :
  0 on success
  -1 on error
*/
int display_rm_partition_on_file_stream(FILE* stream,bridge_rm_partition_t* p_partition){
  int fstatus=-1;

  struct tm * time_value;

  char* nodelist;

  char* state;

  // if NULL display header
  if(p_partition!=NULL){
    switch(p_partition->state){

    case BRIDGE_RM_PARTITION_STATE_IN :
      state="active";
      break;
    case BRIDGE_RM_PARTITION_STATE_OUT :
      state="inactive";
      break;
    case BRIDGE_RM_PARTITION_STATE_DRAIN :
      state="draining";
      break;
    case BRIDGE_RM_PARTITION_STATE_BLOCKED :
      state="blocked";
      break;
    default :
      state="unknown";
    }
    fprintf(stream,"---------------------------------------------------------------------------------");
    fprintf(stream,"-------------------------------------------------------------------------------------------------\n");
    fprintf(stream,"Name \t\t: %s\n",p_partition->name);
    fprintf(stream,"Desc \t\t: %s\n",p_partition->description);
    fprintf(stream,"State \t\t: %s\n",state);
    fprintf(stream,"Reason \t\t: %s\n",p_partition->reason);

    fprintf(stream,"Start time \t: ");
    if(p_partition->start_time!=INVALID_TIME_VALUE){
      time_value=localtime(&(p_partition->start_time));
      fprintf(stream,"%02d/%02d/%4d %02d:%02d:%02d\n",
	      time_value->tm_mday,
	      time_value->tm_mon+1,
	      time_value->tm_year+1900,
	      time_value->tm_hour,
	      time_value->tm_min,
	      time_value->tm_sec);
    }
    else{
      fprintf(stream,"%s","-\n");
    }

    if(p_partition->time_limit!=INVALID_TIME_VALUE)
      fprintf(stream,"Time limit \t: %ld\n",p_partition->time_limit);
    else
      fprintf(stream,"Time limit \t: -\n");

    if(p_partition->memory_limit!=INVALID_INTEGER_VALUE)
      fprintf(stream,"Memory limit \t: %d\n",p_partition->memory_limit);
    else
      fprintf(stream,"Memory limit \t: -\n");

    if(INVALID_INTEGER_VALUE!=p_partition->total_cores_nb)
      fprintf(stream,"Cores #\t\t: %d\n",p_partition->total_cores_nb);
    else
      fprintf(stream,"Cores #\t\t: -\n");

    if(INVALID_INTEGER_VALUE!=p_partition->active_cores_nb)
      fprintf(stream," active\t\t: %d\n",p_partition->active_cores_nb);
    else
      fprintf(stream," active\t\t: -\n");

    if(INVALID_INTEGER_VALUE!=p_partition->used_cores_nb)
      fprintf(stream," used\t\t: %d\n",p_partition->used_cores_nb);
    else
      fprintf(stream," used\t\t: -\n");

    if(INVALID_INTEGER_VALUE!=p_partition->total_nodes_nb)
      fprintf(stream,"Nodes #\t\t: %d\n",p_partition->total_nodes_nb);
    else
      fprintf(stream,"Nodes #\t\t: -\n");

    if(INVALID_INTEGER_VALUE!=p_partition->active_nodes_nb)
      fprintf(stream," active\t\t: %d\n",p_partition->active_nodes_nb);
    else
      fprintf(stream," active\t\t: -\n");

    if(INVALID_INTEGER_VALUE!=p_partition->used_nodes_nb)
      fprintf(stream," used\t\t: %d\n",p_partition->used_nodes_nb);
    else
      fprintf(stream," used\t\t: -\n");

    if(bridge_nodelist_get_compacted_string(&(p_partition->total_nodelist),&nodelist)==0){
      fprintf(stream,"Nodes list \t: %s\n",nodelist);
      free(nodelist);
    }
    else
      fprintf(stream,"Nodes list \t: %s\n","-");

    if(bridge_nodelist_get_compacted_string(&(p_partition->active_nodelist),&nodelist)==0){
      fprintf(stream," active\t\t: %s\n",nodelist);
      free(nodelist);
    }
    else
      fprintf(stream," active\t\t: %s\n","-");

    if(bridge_nodelist_get_compacted_string(&(p_partition->used_nodelist),&nodelist)==0){
      fprintf(stream," used\t\t: %s\n",nodelist);
      free(nodelist);
    }
    else
      fprintf(stream," used\t\t: %s\n","-");
    
    fprintf(stream,"---------------------------------------------------------------------------------");
    fprintf(stream,"-------------------------------------------------------------------------------------------------\n");

    fstatus=0;
  }

  return fstatus;
}


int main(int argc,char** argv){

  int fstatus=-1;
  int status;

  int classic_mode=0;
  int long_flag=0;
  int verbosity=0;

  bridge_manager_t manager;

  bridge_rm_partition_t* partitions_array=NULL;
  bridge_rm_partition_t* partition=NULL;

  int partitions_nb=0;
  int partition_id;

  char* resid=NULL;
  char* username=NULL;
  char* partition_name=NULL;
  char* intersectingNodes=NULL;
  char* includingNodes=NULL;

  char* fields_list="\n"
    "\tname          : partition name\n"
    "\tdesc          : partition description\n"
    "\tstate         : partition status\n"
    "\treason        : reason of this status\n"
    "\tstarttime     : start time of the partition\n"
    "\ttlim          : time limit for allocation in the partition\n"
    "\tmlim          : memory limit for allocation in the partition (Mo)\n"
    "\tcoresnb       : # of cores in the partition\n"
    "\tacoresnb      : # of active cores of the partition\n"
    "\tucoresnb      : # of in-use cores of the partition\n"
    "\tnodesnb       : # of nodes in the partition\n"
    "\tanodesnb      : # of active nodes in the partition\n"
    "\tunodesnb      : # of in-use nodes in the partition\n"
    "\tnodes         : partition nodes list\n"
    "\tanodesnb      : partition active nodes list\n"
    "\tunodesnb      : partition in-use nodes list\n";

  char* options_desc=
    "\t-h\t\t\tPrint this message\n"
    "\t-c\t\t\tDisplay results in classic format\n"
    "\t-a\t\t\tGet active partitions only\n"
    "\t-p partition\t\tGet partitions that use given partition\n"
    "\t-n intNodes\t\tGet partitions that owns at least one active node of this nodes list\n"
    "\t-N incNodes\t\tGet partitions which active nodes are all in this nodes list\n"
    "\t-o fields\t\tDisplay informations using fields format (use -o list for available fields, -o all to get infos for all fields)\n"
    "\t-s separator\t\tWhen used with -o options, enables to change results fields separator\n\t\t\t\t(default is a single spaced string)\n"
    "\t-v\t\t\tWhen used with -o options, the first output line displays list of selected fields\n"
    "\t-V\t\t\tPrint bridge and app versions and exit\n";

  char* output_fields=NULL;
  char* separator=NULL;

  char* progname;
  char* cb_version=NULL;

  int finished_jobs_flag=0;
  time_t begin_eventTime;
  time_t end_eventTime;
  char* date;
  int date_nb;
  int active_only_flag=0;

  char * optstring="hcCu:r:p:o:s:f:vn:N:aV";
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
    case 'a' :
      active_only_flag=1;
      break;
    case 'c' :
      classic_mode=1;
      break;
    case 'C' :
      classic_mode=1;
      long_flag=1;
      break;
    case 'p' :
      partition_name=strdup(optarg);
      break;
    case 'n' :
      intersectingNodes=strdup(optarg);
      break;
    case 'N' :
      includingNodes=strdup(optarg);
      break;
    case 'v' :
      verbosity++;
      break;
    case 'o' :
      if(strcmp(optarg,"list")==0){
	fprintf(stdout,"\nAvailable fields are :\n%s\n",fields_list);
	goto exit;
      }
      else if(strcmp(optarg,"all")==0)
	output_fields=strdup("name,desc,state,reason,starttime,tlim,mlim,coresnb,acoresnb,ucoresnb,nodesnb,anodesnb,unodesnb,nodes,anodes,unodes");
      else
	output_fields=strdup(optarg);
      break;
    case 's' :
      separator=strdup(optarg);
      break;
    case 'f' :
      if(bridge_common_string_get_tokens_quantity(optarg,":",&date_nb)==0){
	if(date_nb==2){
	  date=NULL;
	  if(bridge_common_string_get_token(optarg,":",1,&date)==0){
	    begin_eventTime=strtol(date,(char**)NULL,10);
	    free(date);
	    date=NULL;
	    if(begin_eventTime!=LONG_MIN && begin_eventTime!=LONG_MAX){	    
	      if(bridge_common_string_get_token(optarg,":",2,&date)==0){
		end_eventTime=strtol(date,(char**)NULL,10);
		free(date);
		if(end_eventTime!=LONG_MIN && end_eventTime!=LONG_MAX){
		  finished_jobs_flag=1;
		  break;
		}
	      }
	    }
	  }
	}
      }
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
      fprintf(stdout,"\nUsage : %s [-h] [-c] [-p partition]  [-n intNodes] [-N incNodes] [-o fields [-s separator]] \n\n",argv[0]);
      fprintf(stdout,"%s\n",options_desc);
      goto exit;
    }
  }


  // initialize bridge configuration
  if(bridge_init_manager(&manager)==0){
    status=bridge_get_rm_partitions(&manager,&partitions_array,&partitions_nb,
					partition_name,
					intersectingNodes,
					includingNodes);
    if(status==0){
      fstatus=0;
      if(classic_mode){
	display_classic_bridge_rm_partition_on_file_stream(stdout,NULL,long_flag);
	for(partition_id=0;partition_id<partitions_nb;partition_id++){
	  partition=partitions_array+partition_id;
	  if(active_only_flag && partition->state==BRIDGE_RM_PARTITION_STATE_OUT)
	    continue;
	  display_classic_bridge_rm_partition_on_file_stream(stdout,partition,long_flag);
	  bridge_clean_rm_partition(&manager,partition);
	}
      }
      else if(output_fields==NULL){
	for(partition_id=0;partition_id<partitions_nb;partition_id++){
	  partition=partitions_array+partition_id;
	  if(active_only_flag && partition->state==BRIDGE_RM_PARTITION_STATE_OUT)
	    continue;
	  display_rm_partition_on_file_stream(stdout,partition);
	  bridge_clean_rm_partition(&manager,partition);
	}
      }
      else{
	if(verbosity)
	  display_by_fields_bridge_rm_partition_on_file_stream(stdout,NULL,
								    output_fields,separator);

	for(partition_id=0;partition_id<partitions_nb;partition_id++){
	  partition=partitions_array+partition_id;
	  if(active_only_flag && partition->state==BRIDGE_RM_PARTITION_STATE_OUT)
	    continue;
	  display_by_fields_bridge_rm_partition_on_file_stream(stdout,partition,
								   output_fields,separator);
	  bridge_clean_rm_partition(&manager,partition);
	}
      }
    } 

    bridge_clean_manager(&manager);
  }
  else{
    fstatus=1;
  }
  

  exit :

  if(resid!=NULL)
    free(resid);
  if(username!=NULL)
    free(username);
  if(intersectingNodes!=NULL)
    free(intersectingNodes);
  if(includingNodes!=NULL)
    free(includingNodes);
  if(output_fields!=NULL)
    free(output_fields);
  if(separator!=NULL)
    free(separator);

  return fstatus;

}




int display_classic_bridge_rm_partition_on_file_stream(FILE * stream,bridge_rm_partition_t* rmp){
  int fstatus=0;

  char* state;
  struct tm * time_value;
  char* nodelist;

  int total_cores_nb;
  int free_cores_nb = 0;
  
  if(!rmp){
    fprintf(stream,"%-16s %-10s %-19s %-10s %-10s %10s %-16s\n","PARTITION","STATUS","START_TIME","TYPE","TOTAL_CPUS","FREE_CPUS","NODES");
    fprintf(stream,"%-16s %-10s %-19s %-10s %-10s %10s %-16s\n",
	    "---------",
	    "------",
	    "----------",
	    "----",
	    "----------",
	    "---------",
	    "-----");
  }
  else{
    switch(rmp->state){
      
    case BRIDGE_RM_PARTITION_STATE_IN :
      state="active";
      break;
    case BRIDGE_RM_PARTITION_STATE_OUT :
      state="inactive";
      break;
    case BRIDGE_RM_PARTITION_STATE_DRAIN :
      state="draining";
      break;
    case BRIDGE_RM_PARTITION_STATE_BLOCKED :
      state="blocked";
      break;
    default :
      state="unknown";
    }

    if(bridge_nodelist_get_compacted_string(&(rmp->active_nodelist),&nodelist)!=0)
      nodelist=NULL;

    if(rmp->active_cores_nb!=INVALID_INTEGER_VALUE){
      total_cores_nb=rmp->active_cores_nb;
      if(rmp->used_cores_nb!=INVALID_INTEGER_VALUE)
	free_cores_nb=total_cores_nb-rmp->used_cores_nb;
    }
    else{
      total_cores_nb=0;
      free_cores_nb=0;
    }

    if(rmp->start_time!=INVALID_TIME_VALUE){
      time_value=localtime(&(rmp->start_time));
      fprintf(stream,"%-16s %-10s %02d/%02d/%4d %02d:%02d:%02d %-10s %10d %10d %-16s\n",
	      (rmp->name!=NULL)?rmp->name:"-",
	      state,
	      time_value->tm_mday,
	      time_value->tm_mon+1,
	      time_value->tm_year+1900,
	      time_value->tm_hour,
	      time_value->tm_min,
	      time_value->tm_sec,
	      (rmp->description!=NULL)?rmp->description:"-",
	      total_cores_nb,
	      free_cores_nb,
	      (nodelist!=NULL)?nodelist:"-");
    }
    else{
      fprintf(stream,"%-16s %-10s %-19s %-10s %10d %10d %-16s\n",
	      (rmp->name!=NULL)?rmp->name:"-",
	      state,
	      "-",
	      (rmp->description!=NULL)?rmp->description:"-",
	      total_cores_nb,
	      free_cores_nb,
	      (nodelist!=NULL)?nodelist:"-");
    }


  }

  return fstatus;
}



#define display_string_field(a) \
 if(a!=NULL) \
  fprintf(stream,"%s",a); \
 else \
  fprintf(stream,"%s","-"); \


#define display_integer_field(a) \
 if(a!=INVALID_INTEGER_VALUE) \
  fprintf(stream,"%d",a); \
 else \
  fprintf(stream,"-"); \


#define display_time_field(a) \
 if(a!=INVALID_TIME_VALUE) \
  fprintf(stream,"%ld",a); \
 else \
  fprintf(stream,"-"); \

#define display_nodelist_field(a) \
 char* nodelist_to_display; \
 if(bridge_nodelist_get_compacted_string(&a,&nodelist_to_display)==0) \
  { fprintf(stream,"%s",nodelist_to_display); \
  free(nodelist_to_display); } \
 else \
  fprintf(stream,"-"); \

int display_by_fields_bridge_rm_partition_on_file_stream(FILE * stream,bridge_rm_partition_t* rmp,char* output_fields,char* separator){

  int status=0;

  int i;
  int display_header=0;

  char* token=NULL;

  int token_nb;

  char* string_a;

  char* state;
  char* nodelist;

  if(separator==NULL)
    separator=" ";

  if(rmp==NULL)
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
	else if(strcmp(token,"name")==0){
	  display_string_field(rmp->name);
	}
	else if(strcmp(token,"desc")==0){
	  display_string_field(rmp->description);
	}
	else if(strcmp(token,"state")==0){
	  switch(rmp->state){
	  case BRIDGE_RM_PARTITION_STATE_IN :
	    state="in";
	    break;
	  case BRIDGE_RM_PARTITION_STATE_OUT :
	    state="out";
	    break;
	  case BRIDGE_RM_PARTITION_STATE_DRAIN :
	    state="draining";
	    break;
	  case BRIDGE_RM_PARTITION_STATE_BLOCKED :
	    state="blocked";
	    break;
	  default :
	    state="unknown";
	  }
	  fprintf(stream,state);
	}
	else if(strcmp(token,"reason")==0){
	  display_string_field(rmp->reason);	  
	}
	else if(strcmp(token,"starttime")==0){
	  display_time_field(rmp->start_time);
	}
	else if(strcmp(token,"tlim")==0){
	  display_time_field(rmp->time_limit);
	}
	else if(strcmp(token,"mlim")==0){
	  display_integer_field(rmp->memory_limit);
	}
	else if(strcmp(token,"coresnb")==0){
	  display_integer_field(rmp->total_cores_nb);
	}
	else if(strcmp(token,"acoresnb")==0){
	  display_integer_field(rmp->active_cores_nb);
	}
	else if(strcmp(token,"ucoresnb")==0){
	  display_integer_field(rmp->used_cores_nb);
	}
	else if(strcmp(token,"nodesnb")==0){
	  display_integer_field(rmp->total_nodes_nb);
	}
	else if(strcmp(token,"anodesnb")==0){
	  display_integer_field(rmp->active_nodes_nb);
	}
	else if(strcmp(token,"unodesnb")==0){
	  display_integer_field(rmp->used_nodes_nb);
	}
	else if(strcmp(token,"nodes")==0){
	  display_nodelist_field(rmp->total_nodelist);
	}
	else if(strcmp(token,"anodes")==0){
	  display_nodelist_field(rmp->active_nodelist);
	}
	else if(strcmp(token,"unodes")==0){
	  display_nodelist_field(rmp->used_nodelist);
	}
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
