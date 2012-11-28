/*****************************************************************************\
 *  src/bridge/rmastat.c - 
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
#include <math.h>

#include <limits.h>
#include <time.h>

#include "bridge/bridge.h"

#define PROG_VERSION  "1.0.1"

/* local functions */
int display_classic_bridge_rm_allocation_on_file_stream(FILE * stream,
							bridge_rm_allocation_t* rm,
							int long_flag);
int display_by_fields_bridge_rm_allocation_on_file_stream(FILE * stream,
							  bridge_rm_allocation_t* rm,
							  char* output_fields,
							  char* separator);

// name,desc,partition,state,reason,priority,user,uid,gid,
// subtime,starttime,endtime,susptime,etime,atime,stime,utime,
// subhost,subsid,subpid,ucores_nb,cores_nb,unodes_nb,nodes_nb,memused,nodes,jobids

/*
  Print allocation contents on given file stream
  --------------------------------------------

  if p_allocation==NULL, it prints display header

  Returns :
  0 on success
  -1 on error
*/
int display_rm_allocation_on_file_stream(FILE* stream,bridge_rm_allocation_t* p_allocation){
  int fstatus=-1;

  struct tm * time_value;

  char* nodelist;

  char* state;

  // if NULL display header
  if(p_allocation!=NULL){
    switch(p_allocation->state){

    case BRIDGE_RM_ALLOCATION_STATE_ALLOCATED :
      state="allocated";
      break;
    case BRIDGE_RM_ALLOCATION_STATE_INUSE :
      state="running";
      break;
    case BRIDGE_RM_ALLOCATION_STATE_PENDING :
      state="pending";
      break;
    case BRIDGE_RM_ALLOCATION_STATE_SUSPENDED :
      state="suspended";
      break;
    case BRIDGE_RM_ALLOCATION_STATE_COMPLETED :
      state="completed";
      break;
    case BRIDGE_RM_ALLOCATION_STATE_CANCELLED :
      state="cancelled";
      break;
    case BRIDGE_RM_ALLOCATION_STATE_FAILED :
      state="failed";
      break;
    case BRIDGE_RM_ALLOCATION_STATE_TIMEOUT :
      state="timeout";
      break;
    case BRIDGE_RM_ALLOCATION_STATE_NODE_FAILURE :
      state="node(s) failure(s)";
      break;
    default :
      state="unknown";
    }
    fprintf(stream,"---------------------------------------------------------------------------------");
    fprintf(stream,"-------------------------------------------------------------------------------------------------\n");
    fprintf(stream,"Partition \t\t: %s\n",
	    p_allocation->partition != NULL ? p_allocation->partition : "-" );
    fprintf(stream,"Allocation ID \t\t: %s\n",
	    p_allocation->id != NULL ? p_allocation->id : "-" );
    fprintf(stream,"State \t\t\t: %s\n",state);
    fprintf(stream,"Reason \t\t\t: %s\n",
	    p_allocation->reason != NULL ? p_allocation->reason : "-" );

    fprintf(stream,"Username \t\t: %s\n",
	    p_allocation->username != NULL ? p_allocation->username : "-" );
    fprintf(stream,"User ID \t\t: %d\n",p_allocation->userid);
    fprintf(stream,"Group ID \t\t: %d\n",p_allocation->groupid);

    fprintf(stream,"Allocating Hostname \t: %s\n",(p_allocation->allocating_hostname!=NULL)?p_allocation->allocating_hostname:"-");
    fprintf(stream,"Allocating Session ID \t: %d\n",p_allocation->allocating_session_id);
    fprintf(stream,"Allocating BATCHID \t: %s\n",(p_allocation->allocating_session_batchid==NULL)?"-":p_allocation->allocating_session_batchid);


    fprintf(stream,"Nb Nodes \t\t: %d\n",p_allocation->total_nodes_nb);
    fprintf(stream,"Nb Cores \t\t: %d\n",p_allocation->total_cores_nb);

    if(bridge_nodelist_get_compacted_string(&(p_allocation->nodelist),&nodelist)==0){
      fprintf(stream,"Nodes list \t\t: %s\n",nodelist);
      free(nodelist);
    }
    else
      fprintf(stream,"Nodes list \t\t: %s\n","-");
    //fprintf(stream,"Cores list \t\t: %s\n",p_allocation->cores_list);

    fprintf(stream,"Priority \t\t: %u\n",p_allocation->priority);
    fprintf(stream,"Max Mem/Cpu(Mo) \t: %d\n",p_allocation->memory_usage);

   
    fprintf(stream,"Submit time \t\t: ");
    if(p_allocation->submit_time!=INVALID_TIME_VALUE){
      time_value=localtime(&(p_allocation->submit_time));
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

    fprintf(stream,"Start time \t\t: ");
    if(p_allocation->start_time!=INVALID_TIME_VALUE){
      time_value=localtime(&(p_allocation->start_time));
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
    
    fprintf(stream,"End time \t\t: ");
    if(p_allocation->end_time!=INVALID_TIME_VALUE){
      time_value=localtime(&(p_allocation->end_time));
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


    fprintf(stream,"Elapsed time \t\t: ");
    if(p_allocation->elapsed_time!=INVALID_TIME_VALUE){
      time_t temp=p_allocation->elapsed_time;
      int nb_days=floor(temp/(3600*24));
      temp=temp-nb_days*(3600*24);
      int nb_hours=floor(temp/3600);
      temp=temp-nb_hours*3600;
      int nb_minutes=floor(temp/60);
      temp=temp-nb_minutes*60;
      int nb_secondes=temp;
      fprintf(stream,"%d %02d:%02d:%02d\n",
	      nb_days,
	      nb_hours,
	      nb_minutes,
	      nb_secondes);
    }
    else{
      fprintf(stream,"%s\n","-");
    }

    fprintf(stream,"Allocated time \t\t: ");
    if(p_allocation->allocated_time!=INVALID_TIME_VALUE){
      time_t temp=p_allocation->allocated_time;
      int nb_days=floor(temp/(3600*24));
      temp=temp-nb_days*(3600*24);
      int nb_hours=floor(temp/3600);
      temp=temp-nb_hours*3600;
      int nb_minutes=floor(temp/60);
      temp=temp-nb_minutes*60;
      int nb_secondes=temp;
      fprintf(stream,"%d %02d:%02d:%02d\n",
	      nb_days,
	      nb_hours,
	      nb_minutes,
	      nb_secondes);
    }
    else{
      fprintf(stream,"%s\n","-");
    }

    fprintf(stream,"System time \t\t: ");
    if(p_allocation->system_time!=INVALID_TIME_VALUE){
      time_t temp=p_allocation->system_time;
      int nb_days=floor(temp/(3600*24));
      temp=temp-nb_days*(3600*24);
      int nb_hours=floor(temp/3600);
      temp=temp-nb_hours*3600;
      int nb_minutes=floor(temp/60);
      temp=temp-nb_minutes*60;
      int nb_secondes=temp;
      fprintf(stream,"%d %02d:%02d:%02d\n",
	      nb_days,
	      nb_hours,
	      nb_minutes,
	      nb_secondes);
    }
    else{
      fprintf(stream,"%s\n","-");
    }  

    fprintf(stream,"User time \t\t: ");
    if(p_allocation->user_time!=INVALID_TIME_VALUE){
      time_t temp=p_allocation->user_time;
      int nb_days=floor(temp/(3600*24));
      temp=temp-nb_days*(3600*24);
      int nb_hours=floor(temp/3600);
      temp=temp-nb_hours*3600;
      int nb_minutes=floor(temp/60);
      temp=temp-nb_minutes*60;
      int nb_secondes=temp;
      fprintf(stream,"%d %02d:%02d:%02d\n",
	      nb_days,
	      nb_hours,
	      nb_minutes,
	      nb_secondes);
    }
    else{
      fprintf(stream,"%s\n","-");
    }

    fprintf(stream,"Efficience \t\t: ");
    if(p_allocation->allocated_time!=INVALID_TIME_VALUE){
      if(p_allocation->user_time!=INVALID_TIME_VALUE){
	double eff=(double) (p_allocation->user_time)*100/p_allocation->allocated_time;
	fprintf(stream,"%.2f%%\n",eff);
      }
      else{
	fprintf(stream,"%s\n","~0%");
      }
    }
    else{
      fprintf(stream,"%s\n","-");
    }

    char* output_string;

    if(bridge_idlist_get_compacted_string(&(p_allocation->jobidlist),&output_string)==0){
      fprintf(stream,"Job IDs \t\t: %s\n",output_string);
      free(output_string);
    }
    else{
      fprintf(stream,"Job IDs \t\t: -\n");
    }

    fprintf(stream,"---------------------------------------------------------------------------------");
    fprintf(stream,"-------------------------------------------------------------------------------------------------\n");

    fstatus=0;
  }

  return fstatus;
}


int main(int argc,char** argv){

  int fstatus=-1;
  int status;

  char* progname;
  char* cb_version;

  int classic_mode=0;
  int long_flag=0;
  int verbosity=0;

  bridge_manager_t manager;

  bridge_rm_allocation_t* allocations_array=NULL;
  bridge_rm_allocation_t* allocation=NULL;

  int allocations_nb=0;
  int allocation_id;

  int non_batch_flag=0;
  char* resid=NULL;
  char* username=NULL;
  char* partition_name=NULL;
  char* intersectingNodes=NULL;
  char* includingNodes=NULL;

  char* fields_list="\n"
    "\tid            : ID of allocation                            desc          : Description\n"
    "\tpartition     : name of the partition that is used          batchid       : Id of batch session (optional)\n"
    "\tstate         : Status of allocation                        reason        : More informations about status\n"
    "\tuser          : User name of the allocation\n"
    "\tuid           : uid of allocation owner                     gid           : Name of allocation owner group\n"
    "\tsubhost       : Submission host                             subtime       : Submission time\n"
    "\tsubsid        : Submission session id                       subpid        : Pid of the submission command\n"
    
    "\tstarttime     : Execution start time                        endtime       : Execution end time\n"
    "\tsusptime      : Suspended time                              priority      : Allocation priority\n"
    "\tetime         : Elapsed time                                atime         : Allocated time\n"
    "\tstime         : System mode used time                       utime         : User mode used time\n"
    "\tmemused       : Memory used (per cpu average)\n"
    "\tnodes_nb      : Number of allocated nodes                   unodes_nb     : Number of in-use allocated nodes\n"
    "\tcores_nb      : Number of allocated cores                   ucores_nb     : Number of in-use allocated cores\n"
    "\tnodes         : Allocated nodes list                        jobids        : IDs list\n";

  char* options_desc=
    "\t-h\t\t\tPrint this message\n"
    "\t-c\t\t\tDisplay results in classic format\n"
    "\t-C\t\t\tDisplay results in classic long format\n"
    "\t-i\t\t\tDisplay informations about non-batch allocations only\n"
    "\t-I\t\t\tDisplay informations about batch allocations only\n"
    "\t-r resid\t\tDisplay informations of a allocation giving its id\n"
    "\t-u user\t\t\tGet allocations of this user\n"
    "\t-p partition\t\tGet allocations that use given partition\n"
    "\t-n intNodes\t\tGet allocations that use at least one node of this nodes list\n"
    "\t-N incNodes\t\tGet allocations which nodes are included in this nodes list\n"
    "\t-o fields\t\tDisplay informations using fields format (use -o list for available fields, -o all to get infos for all fields)\n"
    "\t-s separator\t\tWhen used with -o options, enables to change results fields separator\n\t\t\t\t(default is a single spaced string)\n"
    "\t-v\t\t\tWhen used with -o options, the first output line displays list of selected fields\n"
    "\t-V\t\t\tPrint bridge version and exit\n";

  char* output_fields=NULL;
  char* separator=NULL;

  int finished_jobs_flag=0;
  time_t begin_eventTime = 0;
  time_t end_eventTime = 0;
  char* date;
  int date_nb;

  char * optstring="hcCu:r:p:n:N:o:s:f:viIV";
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
    case 'c' :
      classic_mode=1;
      break;
    case 'C' :
      classic_mode=1;
      long_flag=1;
      break;
    case 'i' :
      non_batch_flag=1;
      break;
    case 'I' :
      non_batch_flag=2;
      break;
    case 'u' :
      username=strdup(optarg);
      break;
    case 'r' :
      resid=strdup(optarg);
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
    case 'V' :
      cb_version=bridge_version();
      if(cb_version!=NULL)
	fprintf(stdout,"%s version %s (bridge-%s)\n",progname,PROG_VERSION,cb_version);
      else
	fprintf(stdout,"%s version %s (bridge-unknown)\n",progname,PROG_VERSION);
      goto exit;
      break;
    case 'o' :
      if(strcmp(optarg,"list")==0){
	fprintf(stdout,"\nAvailable fields are :\n%s\n",fields_list);
	goto exit;
      }
      else if(strcmp(optarg,"all")==0)
	output_fields=strdup("id,desc,partition,batchid,state,reason,user,uid,gid,subhost,subtime,subsid,subpid,starttime,endtime,susptime,priority,etime,atime,stime,utime,memused,nodes_nb,unodes_nb,cores_nb,ucores_nb,nodes,jobids");
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
    case 'h' :
    default :
      fprintf(stdout,"\nUsage : %s [-h] [-cC] [-f begin:end] [-u user] [-r resid] [-p partition] [-n intNodes] [-N incNodes] [-o fields [-s separator]] \n\n",argv[0]);
      fprintf(stdout,"%s\n",options_desc);
      goto exit;
    }
  }


  // initialize bridge configuration
  if(bridge_init_manager(&manager)==0){
    if(manager.rm_system_flag){
      if(finished_jobs_flag==0)
	status=bridge_get_rm_allocations(&manager,&allocations_array,&allocations_nb,
					     resid,
					     username,
					     partition_name,
					     intersectingNodes,
					     includingNodes);
      else
	status=bridge_get_terminated_rm_allocations(&manager,&allocations_array,&allocations_nb,
							resid,
							username,
							partition_name,
							intersectingNodes,
							includingNodes,
							begin_eventTime,end_eventTime);    
      if(status==0){
	fstatus=0;
	if(classic_mode){
	  display_classic_bridge_rm_allocation_on_file_stream(stdout,NULL,long_flag);
	  for(allocation_id=0;allocation_id<allocations_nb;allocation_id++){
	    allocation=allocations_array+allocation_id;
	    switch(non_batch_flag){
	    case 1 :
	      if(allocation->allocating_session_batchid!=NULL)
		continue;
	      break;
	    case 2 :
	      if(allocation->allocating_session_batchid==NULL)
		continue;
	      break;
	    default :
	      break;
	    }
	    display_classic_bridge_rm_allocation_on_file_stream(stdout,allocation,long_flag);
	    //	  bridge_clean_rm_allocation(&manager,allocation);
	  }
	}
	else if(output_fields==NULL){
	  for(allocation_id=0;allocation_id<allocations_nb;allocation_id++){
	    allocation=allocations_array+allocation_id;
	    switch(non_batch_flag){
	    case 1 :
	      if(allocation->allocating_session_batchid!=NULL)
		continue;
	      break;
	    case 2 :
	      if(allocation->allocating_session_batchid==NULL)
		continue;
	      break;
	    default :
	      break;
	    }
	    display_rm_allocation_on_file_stream(stdout,allocation);
	    bridge_clean_rm_allocation(&manager,allocation);
	  }
	}
	else{
	  if(verbosity)
	    display_by_fields_bridge_rm_allocation_on_file_stream(stdout,NULL,
								      output_fields,separator);

	  for(allocation_id=0;allocation_id<allocations_nb;allocation_id++){
	    allocation=allocations_array+allocation_id;
	    switch(non_batch_flag){
	    case 1 :
	      if(allocation->allocating_session_batchid!=NULL)
		continue;
	      break;
	    case 2 :
	      if(allocation->allocating_session_batchid==NULL)
		continue;
	      break;
	    default :
	      break;
	    }
	    display_by_fields_bridge_rm_allocation_on_file_stream(stdout,allocation,
								      output_fields,separator);
	    bridge_clean_rm_allocation(&manager,allocation);
	  }
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




int display_classic_bridge_rm_allocation_on_file_stream(FILE * stream,bridge_rm_allocation_t* rm,int long_flag){
  int fstatus=0;

  char* interspace="  ";
  
  char* output_string;
  char* state;

  if(rm==NULL){
    fprintf(stream,"RESID   USER      RES_STAT   NCORES NNODES  ELAPSED TIME   EFF      BATCHID   SUBHOST     MEM       PARTITION    ");
    if(long_flag){
      fprintf(stream,"  JOBS and NODES\n");
    }
    else{
      fprintf(stream,"  NODES\n");
    }
    fprintf(stream,"------  --------  ---------  ------ ------  -------------  -------  --------  ----------  --------  -------------");
    if(long_flag){
      fprintf(stream,"  --------------\n");
    }
    else{
      fprintf(stream,"  -----\n");
    }
  }
  else{

    switch(rm->state){

    case BRIDGE_RM_ALLOCATION_STATE_ALLOCATED :
      state="allocated";
      break;
    case BRIDGE_RM_ALLOCATION_STATE_INUSE :
      state="running";
      break;
    case BRIDGE_RM_ALLOCATION_STATE_PENDING :
      state="pending";
      break;
    case BRIDGE_RM_ALLOCATION_STATE_SUSPENDED :
      state="suspended";
      break;
    case BRIDGE_RM_ALLOCATION_STATE_COMPLETED :
      state="completed";
      break;
    case BRIDGE_RM_ALLOCATION_STATE_CANCELLED :
      state="cancelled";
      break;
    case BRIDGE_RM_ALLOCATION_STATE_FAILED :
      state="failed";
      break;
    case BRIDGE_RM_ALLOCATION_STATE_TIMEOUT :
      state="timeout";
      break;
    case BRIDGE_RM_ALLOCATION_STATE_NODE_FAILURE :
      state="pb_hard";
      break;
    default :
      state="unknown";
    }


    //    printf("%5d  %-8s %-9s  %4d %11s %12s %12s  %3d %5d  %-8s  %6s  %8d  %2d %3d %s\n",

    fprintf(stream,"%-6s%s",rm->id != NULL ? rm->id : "-" ,interspace);
    fprintf(stream,"%-8s%s",rm->username != NULL ? rm->username : "-" ,interspace);
    fprintf(stream,"%-9s%s",state,interspace);
    fprintf(stream,"%5d%s",rm->total_cores_nb,interspace);
    fprintf(stream,"%6d%s",rm->total_nodes_nb,interspace);

    if(rm->elapsed_time!=INVALID_TIME_VALUE){
      time_t temp=rm->elapsed_time;
      int nb_days=floor(temp/(3600*24));
      temp=temp-nb_days*(3600*24);
      int nb_hours=floor(temp/3600);
      temp=temp-nb_hours*3600;
      int nb_minutes=floor(temp/60);
      temp=temp-nb_minutes*60;
      int nb_secondes=temp;
      fprintf(stream,"%4d-%02d:%02d:%02d%s",
	      nb_days,
	      nb_hours,
	      nb_minutes,
	      nb_secondes,interspace);
    }
    else{
      fprintf(stream,"%13s%s","-",interspace);
    }

    if(rm->allocated_time!=INVALID_TIME_VALUE){
      if(rm->user_time!=INVALID_TIME_VALUE){
	double eff=(double) (rm->user_time)*100/rm->allocated_time; //rm->system_time+
	if(eff>100.0)
	  eff=100.0;
	fprintf(stream,"%6.2f%%%s",eff,interspace);
      }
      else{
	fprintf(stream,"%7s%s","~0%",interspace);
      }
    }
    else{
      fprintf(stream,"%7s%s","-",interspace);
    }

    fprintf(stream,"%8s%s",(rm->allocating_session_batchid==NULL)?"-":rm->allocating_session_batchid,interspace);

    fprintf(stream,"%-10s%s",rm->allocating_hostname,interspace);

/*     if(rm->allocating_session_id!=INVALID_INTEGER_VALUE) */
/*       fprintf(stream,"%6d%s",rm->allocating_session_id,interspace); */
/*     else */
/*       fprintf(stream,"%s%s","     -",interspace); */

/*     if(rm->allocating_pid!=INVALID_INTEGER_VALUE) */
/*       fprintf(stream,"%6d%s",rm->allocating_pid,interspace); */
/*     else */
/*       fprintf(stream,"%s%s","     -",interspace); */

/*     fprintf(stream,"%3u%s",rm->priority,interspace); */

    if(rm->memory_usage!=INVALID_INTEGER_VALUE)
      fprintf(stream,"%8d%s",rm->memory_usage,interspace);
    else
      fprintf(stream,"%s%s","       -",interspace);

    fprintf(stream,"%-13s%s",rm->partition,interspace);

    if(long_flag)
      if(bridge_idlist_get_compacted_string(&(rm->jobidlist),&output_string)==0){
	fprintf(stream,"Job(s) %s on ",output_string);
	free(output_string);
      }
      /* else */
      /* 	fprintf(stream,""); */


    if(bridge_nodelist_get_compacted_string(&(rm->nodelist),&output_string)==0){
      fprintf(stream,"%s",output_string);
      free(output_string);
    }
    else
      fprintf(stream,"-");

    fprintf(stream,"\n");
    return 0;

  }
  return fstatus;
}





int display_by_fields_bridge_rm_allocation_on_file_stream(FILE * stream,bridge_rm_allocation_t* rm,char* output_fields,char* separator){

  int status=0;

  int i;
  int display_header=0;

  char* token=NULL;

  int token_nb;

  if(separator==NULL)
    separator=" ";

  if(rm==NULL)
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

	/* RESOURCE ID */
	else if(strcmp(token,"id")==0){
	  if(rm->id!=NULL)
	    fprintf(stream,"%s",rm->id);
	  else
	    fprintf(stream,"-");
	}
	/* RESOURCE ID */
	else if(strcmp(token,"batchid")==0){
	  if(rm->allocating_session_batchid!=NULL)
	    fprintf(stream,"%s",rm->allocating_session_batchid);
	  else
	    fprintf(stream,"-");
	}
	/* RESOURCE DESCRIPTION */
	else if(strcmp(token,"desc")==0){
	  if(rm->description!=NULL)
	    fprintf(stream,"%s",rm->description);
	  else
	    fprintf(stream,"-");
	}
	/* RESOURCE PARTITION */
	else if(strcmp(token,"partition")==0){
	  if(rm->partition!=NULL)
	    fprintf(stream,"%s",rm->partition);
	  else
	    fprintf(stream,"-");
	}

	/* JOB STATE */
	else if(strcmp(token,"state")==0){
	  char* status;
	  switch(rm->state){

	  case BRIDGE_RM_ALLOCATION_STATE_ALLOCATED :
	    status="allocated";
	    break;
	  case BRIDGE_RM_ALLOCATION_STATE_INUSE :
	    status="running";
	    break;
	  case BRIDGE_RM_ALLOCATION_STATE_PENDING :
	    status="pending";
	    break;
	  case BRIDGE_RM_ALLOCATION_STATE_SUSPENDED :
	    status="suspended";
	    break;
	  case BRIDGE_RM_ALLOCATION_STATE_COMPLETED :
	    status="completed";
	    break;
	  case BRIDGE_RM_ALLOCATION_STATE_CANCELLED :
	    status="cancelled";
	    break;
	  case BRIDGE_RM_ALLOCATION_STATE_FAILED :
	    status="failed";
	    break;
	  case BRIDGE_RM_ALLOCATION_STATE_TIMEOUT :
	    status="timeout";
	    break;
	  case BRIDGE_RM_ALLOCATION_STATE_NODE_FAILURE :
	    status="node(s) failure(s)";
	    break;
	  default :
	    status="unknown";
	  }
	  fprintf(stream,"%s",status);
	}
	/* RESOURCE STATE REASON */
	else if(strcmp(token,"reason")==0){
	  if(rm->reason!=NULL)
	    fprintf(stream,"%s",rm->reason);
	  else
	    fprintf(stream,"-");
	}
	/* RESOURCE PRIORITY */
	else if(strcmp(token,"priority")==0){
	  if(rm->priority!=INVALID_INTEGER_VALUE)
	    fprintf(stream,"%u",rm->priority);
	  else
	    fprintf(stream,"-");
	}
	/* USER NAME */
	else if(strcmp(token,"user")==0){
	  if(rm->username!=NULL)
	    fprintf(stream,"%s",rm->username);
	  else
	    fprintf(stream,"-");
	}
	/* USER ID */
	else if(strcmp(token,"uid")==0){
	  if(rm->userid!=INVALID_INTEGER_VALUE)
	    fprintf(stream,"%d",rm->userid);
	  else
	    fprintf(stream,"-");
	}
	/* GROUP ID */
	else if(strcmp(token,"gid")==0){
	  if(rm->groupid!=INVALID_INTEGER_VALUE)
	    fprintf(stream,"%d",rm->groupid);
	  else
	    fprintf(stream,"-");
	}
	/* SUBMISSION TIME */
	else if(strcmp(token,"subtime")==0){
	  if(rm->submit_time!=INVALID_TIME_VALUE){
	    fprintf(stream,"%ld",rm->submit_time);
	  }
	  else
	    fprintf(stream,"-");
	}
	/* START TIME */
	else if(strcmp(token,"starttime")==0){
	  if(rm->start_time!=INVALID_TIME_VALUE){
	    fprintf(stream,"%ld",rm->start_time);
	  }
	  else
	    fprintf(stream,"-");
	}
	/* END TIME */
	else if(strcmp(token,"endtime")==0){
	  if(rm->end_time!=INVALID_TIME_VALUE){
	    fprintf(stream,"%ld",rm->end_time);
	  }
	  else
	    fprintf(stream,"-");
	}
	/* SUBMISSION TIME */
	else if(strcmp(token,"susptime")==0){
	  if(rm->suspend_time!=INVALID_TIME_VALUE){
	    fprintf(stream,"%ld",rm->suspend_time);
	  }
	  else
	    fprintf(stream,"-");
	}
	/* ELAPSED TIME */
	else if(strcmp(token,"etime")==0){
	  if(rm->elapsed_time!=INVALID_TIME_VALUE){
	    fprintf(stream,"%ld",rm->elapsed_time);
	  }
	  else
	    fprintf(stream,"-");
	}
	/* ALLOCATED TIME */
	else if(strcmp(token,"atime")==0){
	  if(rm->allocated_time!=INVALID_TIME_VALUE){
	    fprintf(stream,"%ld",rm->allocated_time);
	  }
	  else
	    fprintf(stream,"-");
	}
	/* SYSTEM TIME */
	else if(strcmp(token,"stime")==0){
	  if(rm->system_time!=INVALID_TIME_VALUE){
	    fprintf(stream,"%ld",rm->system_time);
	  }
	  else
	    fprintf(stream,"-");
	}
	/* USER TIME */
	else if(strcmp(token,"utime")==0){
	  if(rm->user_time!=INVALID_TIME_VALUE){
	    fprintf(stream,"%ld",rm->user_time);
	  }
	  else
	    fprintf(stream,"-");
	}

	/* SUBMISSION HOST */
	else if(strcmp(token,"subhost")==0){
	  if(rm->allocating_hostname!=NULL)
	    fprintf(stream,"%s",rm->allocating_hostname);
	  else
	    fprintf(stream,"-");
	}
	/* SUBMISSION SID */
	else if(strcmp(token,"subsid")==0){
	  if(rm->allocating_session_id>0)
	    fprintf(stream,"%d",rm->allocating_session_id);
	  else
	    fprintf(stream,"-");
	}
	/* SUBMISSION PID */
	else if(strcmp(token,"subpid")==0){
	  if(rm->allocating_pid>0)
	    fprintf(stream,"%d",rm->allocating_pid);
	  else
	    fprintf(stream,"-");
	}
	/* TOTAL CORES NB */
	else if(strcmp(token,"cores_nb")==0){
	  if(rm->total_cores_nb==INVALID_INTEGER_VALUE)
	    fprintf(stream,"-");
	  else
	  fprintf(stream,"%d",rm->total_cores_nb);
	}
	/* USED CORES NB */
	else if(strcmp(token,"ucores_nb")==0){
	  if(rm->used_cores_nb==INVALID_INTEGER_VALUE)
	    fprintf(stream,"-");
	  else
	    fprintf(stream,"%d",rm->used_cores_nb);
	}
	/* TOTAL NODES NB */
	else if(strcmp(token,"nodes_nb")==0){
	  if(rm->total_nodes_nb==INVALID_INTEGER_VALUE)
	    fprintf(stream,"-");
	  else
	    fprintf(stream,"%d",rm->total_nodes_nb);
	}
	/* USED NODES NB */
	else if(strcmp(token,"unodes_nb")==0){
	  if(rm->used_nodes_nb==INVALID_INTEGER_VALUE)
	    fprintf(stream,"-");
	  else
	    fprintf(stream,"%d",rm->used_nodes_nb);
	}
	/* USED MEMORY */
	else if(strcmp(token,"memused")==0){
	  if(rm->memory_usage!=INVALID_INTEGER_VALUE)
	    fprintf(stream,"%d",rm->memory_usage);
	  else
	    fprintf(stream,"-");
	}
	/* ALLOCATED NODES */
	else if(strcmp(token,"nodes")==0){
	  char* list=NULL;
	  if(bridge_nodelist_get_compacted_string(&(rm->nodelist),&list)==0){
	    fprintf(stream,"%s",list);
	    free(list);
	  }
	  else
	    fprintf(stream,"-");
	}
	/* IDS of launched jobs */
	else if(strcmp(token,"jobids")==0){
	  char* list=NULL;
	  if(bridge_idlist_get_compacted_string(&(rm->jobidlist),&list)==0){
	    fprintf(stream,"%s",list);
	    free(list);
	  }
	  else
	    fprintf(stream,"-");
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
