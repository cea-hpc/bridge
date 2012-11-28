/*****************************************************************************\
 *  src/bridge/bsstat.c - 
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

/*!
 * \brief display batch session informations on file stream in an extended form 
 * \internal
 *
 * \param stream FILE* on which to write output
 * \param bs pointer on a batch session structure to display
 *
 * \retval 0 on success
 * \retval -1 on failure
 */
int display_bridge_batch_session_on_file_stream(FILE * stream,bridge_batch_session_t* bs);

/*!
 * \brief display batch session informations on file stream in an classic short or long form 
 * \internal
 *
 * \param stream FILE* on which to write output
 * \param bs pointer on a batch session structure to display
 *
 * \retval 0 on success
 * \retval -1 on failure
 */
int display_classic_bridge_batch_session_on_file_stream(FILE * stream,bridge_batch_session_t* bs,int long_flag);

/*!
 * \brief display required informations about batch session on file stream 
 * \internal
 *
 * \param stream FILE* on which to write output
 * \param bs pointer on a batch session structure to display
 * \param output_fields comma separated list of informations to display
 * \param separator string to write on stream between each required information
 *
 * \retval 0 on success
 * \retval -1 on failure
 */
int display_by_fields_bridge_batch_session_on_file_stream(FILE * stream,bridge_batch_session_t* bs,char* output_fields,char* separator);

/* local functions */
int display_new_classic_bridge_batch_session_on_file_stream(FILE * stream,
							    bridge_batch_session_t* bs,
							    int long_flag);

/*!
 * \brief test if output fields required parallel informations
 * \internal
 *
 * \param parallel_fields comma or blank separated list of fields that requires parallel informations 
 * \param parallel_fields comma separated list of required fields 
 *
 * \retval 0 if they don't require parallel informations
 * \retval 1 if they do require
 */
int output_fields_require_parallel_infos(char* parallel_fields,char* query_fields);


int main(int argc,char **argv){

  int fstatus=-1;
  int status;

  char* progname;
  char* cb_version;

  bridge_manager_t manager;

  bridge_batch_session_t* p_batch_session_array=NULL;
  int batch_sessions_nb;
  int i;

  int with_parallel_infos=1;

  FILE* output_stream=stdout;

  int finished_jobs_flag=0;
  int classic_mode=0;
  int new_classic_mode=0;
  int long_flag=0;
  int verbosity=0;

  char* batchid=NULL;
  char* jobname=NULL;
  char* username=NULL;
  char* queue_name=NULL;
  char* exec_hostname=NULL;

#define PARALLEL_FIELDS "partimeused,parmemused,parcpunb,parnodeslist,parnodenames,rmid"

  char* fields_list="\n"
    "\tbatchid       : ID of batch session                         name          : Name of batch session\n"
    "\tstate         : Status of batch session                     reason        : More informations about status\n"
    "\tuser          : Name of batch session owner                 group         : Name of batch session owner group\n"
    "\tproject       : Name of batch session owner project         qos           : Name of batch session qos\n"
    "\tsubhost       : Submission host                             subtime       : submition time\n"
    "\tqueue         : Batch queue of batch session                priority      : Batch session priority\n"
    "\texechost      : Execution host                              sid           : Execution session id\n"
    "\tstarttime     : Execution start time                        endtime       : Execution end time\n"
    "\tseqtimeused   : Sequential time used                        seqtimelim    : Sequential time limit\n"
    "\tuseqtimeused  : Sequential time used in user mode           sseqtimeused  : Sequential time used in system mode\n"
    "\tseqmemused    : Sequential memory used                      seqmemlim     : Sequential memory limit\n"
    "\tpartimeused   : Parallel time used                          partimelim    : Parallel time limit\n"
    "\tparmemused    : Parallel memory used (per cpu average)      parmemlim     : Parallel memory limit\n"
    "\tparcpunb      : Parallel CPU quantity used                  parcpunblim   : Parallel CPU maximum quantity\n"
    "\tparnodeslist  : Parallel executing nodes (condensed)        parnodenames  : Parallel executing nodes (extended)\n"
    "\trmid          : Resource manager ID of corresponding job\n";

  char* options_desc=
    "\t-h\t\t\tPrint this message\n"
    "\t-c\t\t\tDisplay results in classic format\n"
    "\t-C\t\t\tDisplay results in classic long format\n"
    "\t-l\t\t\tLight mode, parallel informations are not fetched\n"
    "\t-d\t\t\tDisplay results in new CEA/DAM format\n"
    "\t-D\t\t\tDisplay results in new CEA/DAM long format\n"
    "\t-u user\t\t\tDisplay results concerning this user\n"
    "\t-b batchid\t\tGet a batch session giving its batch Id\n"
    "\t-q queue\t\tDisplay results concerning this batch queue\n"
    "\t-H execHost\t\tDisplay results concerning batch session executed on this this batch host\n"
    "\t-n intNodes\t\tDisplay results concerning batch session that use at least one parallel node of this nodes list\n"
    "\t-N incNodes\t\tDisplay results concerning batch session which parallel nodes are included in this nodes list\n"
    "\t-f begin:end\t\tDisplay finished batch session which finished event was recorded between begin and end date (in seconds, 0 means no limit)\n"
    "\t-o fields\t\tDisplay informations using fields format (use -o list for available fields, -o all to get infos for all fields)\n"
    "\t-s separator\t\tWhen used with -o options, enables to change results fields separator\n\t\t\t\t(default is a single spaced string)\n"
    "\t-v\t\t\tWhen used with -o options, the first output line displays list of selected fields\n"
    "\t-V\t\t\tPrint bridge and app versions and exit\n";

  char* output_fields=NULL;
  char* separator=NULL;

  time_t begin_eventTime = 0;
  time_t end_eventTime = 0;
  char* date;
  int date_nb;

#define NO_FILTERING                   0
#define FILTERING_IN_INTERSECTION_MODE 1 
#define FILTERING_IN_INCLUSION_MODE    2 
  int filter_mode=NO_FILTERING;
  char* filter_nodes=NULL;
  bridge_nodelist_t list;
  
  char * optstring="hvcCdDu:b:q:H:o:s:f:ln:N:V";
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
      filter_mode=FILTERING_IN_INTERSECTION_MODE;
      filter_nodes=strdup(optarg);
      break;
    case 'N' :
      filter_mode=FILTERING_IN_INCLUSION_MODE;
      filter_nodes=strdup(optarg);
      break;
    case 'c' :
      classic_mode=1;
      break;
    case 'C' :
      classic_mode=1;
      long_flag=1;
      break;
    case 'd' :
      new_classic_mode=1;
      break;
    case 'D' :
      new_classic_mode=1;
      long_flag=1;
      break;
    case 'v' :
      verbosity++;
      break;
    case 'l' :
      with_parallel_infos=0;
      break;
    case 'u' :
      username=strdup(optarg);
      break;
    case 'b' :
      batchid=strdup(optarg);
      break;
    case 'q' :
      queue_name=strdup(optarg);
      break;
    case 'H' :
      exec_hostname=strdup(optarg);
      break;
    case 'o' :
      if(strcmp(optarg,"list")==0){
	fprintf(stdout,"\nAvailable fields are :\n%s\n",fields_list);
	goto exit;
      }
      else if(strcmp(optarg,"all")==0){
	output_fields=strdup("batchid,name,state,reason,user,group,project,qos,subhost,subtime,queue,priority,exechost,sid,starttime,endtime,seqtimeused,seqtimelim,useqtimeused,sseqtimeused,seqmemused,seqmemlim,partimeused,partimelim,parmemused,parmemlim,parcpunb,parcpunblim,parnodeslist,rmid");
      }
      else
	output_fields=strdup(optarg);
      with_parallel_infos=output_fields_require_parallel_infos(PARALLEL_FIELDS,output_fields);
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
      fprintf(stdout,"\nUsage : %s [-h] [-cC] [-u user] [-b batchid] [-q queue] [-H execHost] [-n intNodes] [-N incNodes] [-f begin:end] [-o fields [-s separator]] \n\n",argv[0]);
      fprintf(stdout,"%s\n",options_desc);
      goto exit;
    }
  }

  // check that filter mode is ok if active
  if(filter_mode!=NO_FILTERING && filter_nodes==NULL){
    status=-2;
    goto exit;
  }
  else if(filter_mode!=NO_FILTERING){
    bridge_nodelist_init(&list,NULL,0);
    if(bridge_nodelist_add_nodes(&list,filter_nodes)!=0){
      status=-2;
      bridge_nodelist_free_contents(&list);
      goto exit;
    }
    // force parallel information if not set
    if(!with_parallel_infos)
      with_parallel_infos=1;
  }

  // initialize bridge configuration
  if(bridge_init_manager(&manager)==0){

    /*
     * Get Batch Sessions Statistics
     * -----------------------------
     */
    if(finished_jobs_flag==0){
      status=bridge_get_batch_sessions(&manager,
				       &p_batch_session_array,
				       &batch_sessions_nb,
				       batchid,
				       jobname,
				       username,
				       queue_name,
				       exec_hostname,
				       with_parallel_infos);
    }
    else{
      status=bridge_get_terminated_batch_sessions(&manager,
						      &p_batch_session_array,
						      &batch_sessions_nb,
						      batchid,
						      jobname,
						      username,
						      queue_name,
						      exec_hostname,
						      with_parallel_infos,
						      begin_eventTime,end_eventTime);
    }

#define PARALLEL_NODES_FILTERING_MACRO \
	  int cont_flag=0; \
	  switch(filter_mode){ \
	  case FILTERING_IN_INCLUSION_MODE : \
	    if(bridge_nodelist_includes(&list,&((p_batch_session_array+i)->par_nodelist))!=1) \
	      cont_flag=1; \
	    break; \
	  case FILTERING_IN_INTERSECTION_MODE : \
	    if(bridge_nodelist_intersects(&list,&((p_batch_session_array+i)->par_nodelist))!=1) \
	      cont_flag=1; \
	    break; \
	  default : \
	    break; \
	  } \
	  if(cont_flag) \
	    continue; \

	  
    /*
     * Display Batch Sessions Statistics
     * ---------------------------------
     */
    if(status==0){
      /* If new classic display mode */
      if(new_classic_mode){
        display_new_classic_bridge_batch_session_on_file_stream(output_stream,NULL,long_flag);
        for(i=0;i<batch_sessions_nb;i++){
          PARALLEL_NODES_FILTERING_MACRO
          display_new_classic_bridge_batch_session_on_file_stream(output_stream,p_batch_session_array+i,long_flag);
        }
      }
      /* If classic display mode */
      else if(classic_mode){
	display_classic_bridge_batch_session_on_file_stream(output_stream,NULL,long_flag);
	for(i=0;i<batch_sessions_nb;i++){
	  PARALLEL_NODES_FILTERING_MACRO
	  display_classic_bridge_batch_session_on_file_stream(output_stream,p_batch_session_array+i,long_flag);
	}
      }
      /* standard display mode */
      else if(output_fields==NULL){
	for(i=0;i<batch_sessions_nb;i++){
	  PARALLEL_NODES_FILTERING_MACRO
	  display_bridge_batch_session_on_file_stream(output_stream,p_batch_session_array+i);
	}
      }
      /* composite mode */
      else{
	if(verbosity)
	  display_by_fields_bridge_batch_session_on_file_stream(output_stream,NULL,output_fields,separator);
	for(i=0;i<batch_sessions_nb;i++){
	  PARALLEL_NODES_FILTERING_MACRO
	  display_by_fields_bridge_batch_session_on_file_stream(output_stream,p_batch_session_array+i,output_fields,separator);
	}
      }

      /*
       * Clean batch sessions
       */
      for(i=0;i<batch_sessions_nb;i++){
	bridge_clean_batch_session(&manager,p_batch_session_array+i);
      }
      free(p_batch_session_array);
      fstatus=0;
    }

    bridge_clean_manager(&manager);
  }
  else{
    fstatus=1;
  }

  if(filter_mode!=NO_FILTERING){
    bridge_nodelist_free_contents(&list);
  }

  exit :
    
  if(filter_nodes!=NULL)
    free(filter_nodes);
  if(username!=NULL)
    free(username);
  if(batchid!=NULL)
    free(batchid);
  if(jobname!=NULL)
    free(jobname);
  if(queue_name!=NULL)
    free(queue_name);
  if(exec_hostname!=NULL)
    free(exec_hostname);
  if(output_fields!=NULL)
    free(output_fields);
  if(separator!=NULL)
    free(separator);

  return fstatus;

}




int display_bridge_batch_session_on_file_stream(FILE * stream,bridge_batch_session_t* bs){

  char time_string[128];
  char* nodelist;
  
  fprintf(stream,
	  "-------------------------------------------------------\n");
  
  /* batch session id */
  if(bs->batch_id!=NULL)
    fprintf(stream,"Batch ID \t: %s\n",bs->batch_id);
  else
    fprintf(stream,"Batch ID \t: -\n");

  /* batch session name */
  if(bs->name!=NULL)
    fprintf(stream,"Name \t\t: %s\n",bs->name);
  else
    fprintf(stream,"Name \t\t: -\n");

  /* batch session description */
    
  /* Job Status */
  char* status;
  switch(bs->state){
  case BRIDGE_BATCH_SESSION_STATE_RUNNING :
    status="running";
    break;
  case BRIDGE_BATCH_SESSION_STATE_PENDING :
    status="pending";
    break;
  case BRIDGE_BATCH_SESSION_STATE_SUSPENDED :
    status="suspended";
    break;
  case BRIDGE_BATCH_SESSION_STATE_DONE :
    status="done";
    break;
  case BRIDGE_BATCH_SESSION_STATE_FAILED :
    status="failed";
    break;
  case BRIDGE_BATCH_SESSION_STATE_UNKNOWN :
  default :
    status="unknown";
    break;
  }
  fprintf(stream,"State \t\t: %s\n",status);
  if(bs->reason!=NULL)
    fprintf(stream,"Reason \t\t: %s\n",bs->reason);
  else
    fprintf(stream,"Reason \t\t: -\n");
  
  /* User name */
  if(bs->username!=NULL)
    fprintf(stream,"User name \t: %s\n",bs->username);
  else
    fprintf(stream,"User name \t: -\n");

  /* Group name */
  if(bs->usergroup!=NULL)
    fprintf(stream,"User group \t: %s\n",bs->usergroup);
  else
    fprintf(stream,"User group \t: -\n");

  /* Project Name */
  if(bs->projectname!=NULL)
    fprintf(stream,"Project name \t: %s\n",bs->projectname);
  else
    fprintf(stream,"Project name \t: -\n");

  /* QOS Name */
  if(bs->qos!=NULL)
    fprintf(stream,"QOS name   \t: %s\n",bs->qos);
  else
    fprintf(stream,"QOS name   \t: -\n");

  /* Submit Host */
  if(bs->submit_hostname!=NULL)
    fprintf(stream,"Submit Host \t: %s\n",bs->submit_hostname);
  else
    fprintf(stream,"Submit Host \t: -\n");

  /* Submit Time */
  if(bs->submit_time!=INVALID_TIME_VALUE){
    ctime_r(&bs->submit_time,time_string);
    fprintf(stream,"Submit Time \t: %s",time_string);
  }
  else
    fprintf(stream,"Submit Time \t: -\n");

  /* Queue Name */
  if(bs->queue!=NULL)
    fprintf(stream,"Queue name \t: %s\n",bs->queue);
  else
    fprintf(stream,"Queue name \t: -\n");

  /* Priority */
  fprintf(stream,"Priority \t: %u\n",bs->priority);

  /* Exec Host */
  if(bs->executing_hostname!=NULL)
    fprintf(stream,"Exec Host \t: %s\n",bs->executing_hostname);
  else
    fprintf(stream,"Exec Host \t: -\n");

  /* Session ID */
  if(bs->session_id!=INVALID_SESSIONID)
    fprintf(stream,"Session ID \t: %d\n",bs->session_id);
  else
    fprintf(stream,"Session ID \t: -\n");

  /* Start Time */
  if(bs->start_time!=INVALID_TIME_VALUE){
    ctime_r(&bs->start_time,time_string);
    fprintf(stream,"Start Time \t: %s",time_string);
  }
  else
    fprintf(stream,"Start Time \t: -\n");

  /* End Time */
  if(bs->end_time!=INVALID_TIME_VALUE){
    ctime_r(&bs->end_time,time_string);
    fprintf(stream,"End Time \t: %s",time_string);
  }
  else
    fprintf(stream,"End Time \t: -\n");

  /* batch session sequential time limit */
  if((bs->system_time+bs->user_time)>0)
    fprintf(stream,"Seq time used \t: %ld (system:%ld,user:%ld)\n",
	    bs->system_time+bs->user_time,bs->system_time,bs->user_time);
  else
    fprintf(stream,"Seq time used \t: 0\n");
  fprintf(stream,"Seq time limit \t: %ld\n",bs->seq_time_limit);

  /* Mem used */
  fprintf(stream,"Seq mem used \t: %u\n",bs->seq_mem_used);
  /* batch session sequential memory limit */
  fprintf(stream,"Seq mem limit \t: %u\n",bs->seq_mem_limit);

  /* batch session parallel time limit */
  fprintf(stream,"Par time used \t: %ld\n",bs->par_time_used);
  fprintf(stream,"Par time limit \t: %ld\n",bs->par_time_limit);

  /* batch session memory limit */
  fprintf(stream,"Par mem used \t: %u\n",bs->par_mem_used);
  fprintf(stream,"Par mem limit \t: %u\n",bs->par_mem_limit);

  /* batch session parallele cores number limit */
  fprintf(stream,"Cores nb \t: %u\n",bs->par_cores_nb);
  fprintf(stream,"Cores nb limit \t: %u\n",bs->par_cores_nb_limit);

  /* batch session parallele executing hostnames */
  if(bridge_nodelist_get_compacted_string(&(bs->par_nodelist),&nodelist)==0){
    fprintf(stream,"Nodes list \t: %s\n",nodelist);
    free(nodelist);
  }
  else
    fprintf(stream,"Nodes list \t: %s\n","-");

    fprintf(stream,
	  "-------------------------------------------------------\n");

    return 0;
}

int display_new_classic_bridge_batch_session_on_file_stream(FILE * stream,bridge_batch_session_t* bs,int long_flag){
  int fstatus=0;
  char* project;

  /* if bs equals NULL, display header */
  if(bs==NULL){
    if(long_flag){
      printf("BATCHID  USER       PROJECT          QUEUE     QOS     PRIO   SUBHOST      EXEHOST      STA    TUSED     TLIM    MLIM   CLIM NAME\n");
      printf("-------  ----       -------          -----     ------  ------ -------      -------      --- -------- -------- ------- ------ ----\n");
    }
    else{
      printf("BATCHID  NAME     USER       PROJECT          QUEUE     QOS     PRIO   SUBHOST      EXEHOST      STA    TUSED     TLIM    MLIM   CLIM\n");
      printf("-------  ----     ----       -------          -----     ------  ------ -------      -------      --- -------- -------- ------- ------\n");
    }
  }
  else{
    char batchid[32] ;
    sprintf(batchid,"%s",bs->batch_id != NULL ? bs->batch_id : "unknown",
                            bs->submit_hostname != NULL ? bs->submit_hostname : "unknown");

    char status[4];
    switch(bs->state){
    case BRIDGE_BATCH_SESSION_STATE_RUNNING :
      sprintf(status,"%.3s",bs->reason);
      break;
    case BRIDGE_BATCH_SESSION_STATE_PENDING :
      sprintf(status,"PEN");
      break;
    case BRIDGE_BATCH_SESSION_STATE_SUSPENDED :
      sprintf(status,"SUS");
      break;
    case BRIDGE_BATCH_SESSION_STATE_DONE :
      sprintf(status,"DON");
      break;
    case BRIDGE_BATCH_SESSION_STATE_FAILED :
      sprintf(status,"FAI");
      break;
    case BRIDGE_BATCH_SESSION_STATE_UNKNOWN :
    default :
      sprintf(status,"UNK");
      break;
    }

    if ( bs->projectname != NULL ) {
/*	    project = index(bs->projectname,'@');
	    if ( project != NULL ) 
		    project++;
	    else */
		    project = bs->projectname;
    } else
	    project = "NA";

    /* Long version 'classic' display */
    if (long_flag) {
      fprintf(stream,"%-7s  %-10.10s %-16.16s %-9s %-7s %6d %-12s %-12s %-.3s %8ld %8ld %7u %6d %s\n",
              batchid,
              bs->username != NULL ? bs->username : "unknown",
	      project,
              (bs->queue != NULL ? bs->queue : "NA"),
              bs->qos ,
              bs->priority,
              bs->submit_hostname != NULL ? bs->submit_hostname : "unknown",
              bs->executing_hostname != NULL ? bs->executing_hostname : bs->submit_hostname,
              status,
              bs->par_time_used,
              bs->par_time_limit,
              (bs->par_cores_nb==0)?bs->seq_mem_limit:bs->par_mem_limit,
              bs->par_cores_nb_limit,
              bs->name);
    }
    /* Short version 'classic' display */
   
     else {
      fprintf(stream,"%-7s  %-8.8s %-10.10s %-16.16s %-9s %-7s %6d %-12s %-12s %-.3s %8ld %8ld %7u %6d\n",
              batchid,
              bs->name != NULL ? bs->name : "unknown",
              bs->username != NULL ? bs->username : "unknown",
	      project,
              bs->queue != NULL ? bs->queue : "NA",
              bs->qos ,
              bs->priority,
              bs->submit_hostname != NULL ? bs->submit_hostname : "unkown",
              bs->executing_hostname != NULL ? bs->executing_hostname : bs->submit_hostname,
              status,
              bs->par_time_used,
              bs->par_time_limit,
              (bs->par_cores_nb==0)?bs->seq_mem_limit:bs->par_mem_limit,
              bs->par_cores_nb_limit);
    }
  }
  return fstatus;
}



int display_classic_bridge_batch_session_on_file_stream(FILE * stream,bridge_batch_session_t* bs,int long_flag){
  int fstatus=0;
  
  /* if bs equals NULL, display header */
  if(bs==NULL){
    if(long_flag){
      printf("                                                                        Seq.Time(sec)     Par.Time(sec)   Memory(Mo)       PEs\n");
      printf("BATCHID             USER      QUEUE                   PRIORITY STA      Used    Limit     Used    Limit   Usage   Limit Usage Limit NAME\n");
      printf("-------             ----      -----                   -------- ---      ----    -----     ----    -----   -----   ----- ----- ----- ----\n");
    }
    else{
      printf("                                                                                  Seq.Time(sec)     Par.Time(sec)   Memory(Mo)       PEs\n");
      printf("BATCHID             NAME      USER      QUEUE                   PRIORITY STA      Used    Limit     Used    Limit   Usage   Limit Usage Limit\n");
      printf("-------             ----      ----      -----                   -------- ---      ----    -----     ----    -----   -----   ----- ----- -----\n");
    }
  }
  else{
    char batchid[32] ;
    sprintf(batchid,"%s.%s",bs->batch_id != NULL ? bs->batch_id : "unknown",
			    bs->submit_hostname != NULL ? bs->submit_hostname : "unkown");

    char status[4];
    switch(bs->state){
    case BRIDGE_BATCH_SESSION_STATE_RUNNING :
      sprintf(status,"%.3s",bs->reason);
      break;
    case BRIDGE_BATCH_SESSION_STATE_PENDING :
      sprintf(status,"PEN");
      break;
    case BRIDGE_BATCH_SESSION_STATE_SUSPENDED :
      sprintf(status,"SUS");
      break;
    case BRIDGE_BATCH_SESSION_STATE_DONE :
      sprintf(status,"DON");
      break;
    case BRIDGE_BATCH_SESSION_STATE_FAILED :
      sprintf(status,"FAI");
      break;
    case BRIDGE_BATCH_SESSION_STATE_UNKNOWN :
    default :
      sprintf(status,"UNK");
      break;
    }
    /* Long version 'classic' display */
    if (long_flag) {
      fprintf(stream,"%-19s %-8.8s %10s@%-12s %9u %.3s  %8ld %8ld %8ld %8ld %7u %7u  %4d  %4d %s\n",
	      batchid,
	      bs->username != NULL ? bs->username : "unknown",
	      (bs->queue != NULL ? bs->queue : "NA"),
	      (bs->executing_hostname != NULL ? bs->executing_hostname : bs->submit_hostname),
	      bs->priority,
	      status,
	      bs->system_time+bs->user_time,
	      bs->seq_time_limit,
	      bs->par_time_used,
	      bs->par_time_limit,
	      (bs->par_cores_nb==0)?bs->seq_mem_used:bs->par_mem_used,
	      (bs->par_cores_nb==0)?bs->seq_mem_limit:bs->par_mem_limit,
	      bs->par_cores_nb,
	      bs->par_cores_nb_limit,
	      bs->name);
    }
    /* Short version 'classic' display */
    else {
      fprintf(stream,"%-19s %-8.8s  %-8.8s %10s@%-12s %9d %-.3s  %8ld %8ld %8ld %8ld %7u %7u  %4d  %4d\n",
	      batchid,
	      bs->name != NULL ? bs->name : "unknown",
	      bs->username != NULL ? bs->username : "unknown",
	      bs->queue != NULL ? bs->queue : "NA",
	      (bs->executing_hostname != NULL ? bs->executing_hostname : bs->submit_hostname),
	      bs->priority,
	      status,
	      bs->system_time+bs->user_time,
	      bs->seq_time_limit,
	      bs->par_time_used,
	      bs->par_time_limit,
	      (bs->par_cores_nb==0)?bs->seq_mem_used:bs->par_mem_used,
	      (bs->par_cores_nb==0)?bs->seq_mem_limit:bs->par_mem_limit,
	      bs->par_cores_nb,
	      bs->par_cores_nb_limit);
    }
  }
  return fstatus;
}


int display_by_fields_bridge_batch_session_on_file_stream(FILE * stream,bridge_batch_session_t* bs,char* output_fields,char* separator){

  int status=0;

  int i;
  int display_header=0;

  char* token=NULL;

  int token_nb;

  char* string_a;

  char* nodelist;

  if(separator==NULL)
    separator=" ";

  if(bs==NULL)
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

	/* BATCHID */
	else if(strcmp(token,"batchid")==0){
	  if(bs->batch_id!=NULL)
	    fprintf(stream,"%s",bs->batch_id);
	  else
	    fprintf(stream,"-");
	}
	/* JOB NAME */
	else if(strcmp(token,"name")==0){
	  if(bs->username!=NULL)
	    fprintf(stream,"%s",bs->name);
	  else
	    fprintf(stream,"-");
	}
	/* JOB STATE */
	else if(strcmp(token,"state")==0){
	  char* status;
	  switch(bs->state){
	  case BRIDGE_BATCH_SESSION_STATE_RUNNING :
	    status="running";
	    break;
	  case BRIDGE_BATCH_SESSION_STATE_PENDING :
	    status="pending";
	    break;
	  case BRIDGE_BATCH_SESSION_STATE_SUSPENDED :
	    status="suspended";
	    break;
	  case BRIDGE_BATCH_SESSION_STATE_DONE :
	    status="done";
	    break;
	  case BRIDGE_BATCH_SESSION_STATE_FAILED :
	    status="failed";
	    break;
	  case BRIDGE_BATCH_SESSION_STATE_UNKNOWN :
	  default :
	    status="unknown";
	    break;
	  }
	  fprintf(stream,"%s",status);
	}
	/* JOB STATE REASON */
	else if(strcmp(token,"reason")==0){
	  if(bs->reason!=NULL)
	    fprintf(stream,"%s",bs->reason);
	  else
	    fprintf(stream,"-");
	}
	/* USER NAME */
	else if(strcmp(token,"user")==0){
	  if(bs->username!=NULL)
	    fprintf(stream,"%s",bs->username);
	  else
	    fprintf(stream,"-");
	}
	/* GROUP NAME */
	else if(strcmp(token,"group")==0){
	  if(bs->usergroup!=NULL)
	    if(strnlen(bs->usergroup,1)>0)
	      fprintf(stream,"%s",bs->usergroup);
	    else
	      fprintf(stream,"-");
	  else
	    fprintf(stream,"-");
	}
	/* PROJECT NAME */
	else if(strcmp(token,"project")==0){
	  if(bs->projectname!=NULL)
	    fprintf(stream,"%s",bs->projectname);
	  else
	    fprintf(stream,"-");
	}
	/* QOS NAME */
	else if(strcmp(token,"qos")==0){
	  if(bs->qos!=NULL)
	    fprintf(stream,"%s",bs->qos);
	  else
	    fprintf(stream,"-");
	}
	/* SUBMISSION HOST */
	else if(strcmp(token,"subhost")==0){
	  if(bs->submit_hostname!=NULL)
	    fprintf(stream,"%s",bs->submit_hostname);
	  else
	    fprintf(stream,"-");
	}
	/* SUBMISSION TIME */
	else if(strcmp(token,"subtime")==0){
	  if(bs->submit_time!=INVALID_TIME_VALUE){
	    fprintf(stream,"%ld",bs->submit_time);
	  }
	  else
	    fprintf(stream,"-");
	}
	/* QUEUE NAME */
	else if(strcmp(token,"queue")==0){
	  if(bs->queue!=NULL)
	    fprintf(stream,"%s",bs->queue);
	  else
	    fprintf(stream,"-");
	}
	/* JOB PRIORITY */
	else if(strcmp(token,"priority")==0){
	  fprintf(stream,"%u",bs->priority);
	}
	/* EXECUTION HOST */
	else if(strcmp(token,"exechost")==0){
	  if(bs->executing_hostname!=NULL)
	    fprintf(stream,"%s",bs->executing_hostname);
	  else
	    fprintf(stream,"-");
	}
	/* SESSION ID */
	else if(strcmp(token,"sid")==0){
	  if(bs->session_id!=INVALID_SESSIONID)
	    fprintf(stream,"%d",bs->session_id);
	  else
	    fprintf(stream,"-");
	}
	/* START TIME */
	else if(strcmp(token,"starttime")==0){
	  if(bs->start_time!=INVALID_TIME_VALUE){
	    fprintf(stream,"%ld",bs->start_time);
	  }
	  else
	    fprintf(stream,"-");
	}
	/* END TIME */
	else if(strcmp(token,"endtime")==0){
	  if(bs->end_time!=INVALID_TIME_VALUE){
	    fprintf(stream,"%ld",bs->end_time);
	  }
	  else
	    fprintf(stream,"-");
	}
	/* SEQ USER TIME USED */
	else if(strcmp(token,"useqtimeused")==0){
	  fprintf(stream,"%ld",bs->user_time);
	}
	/* SEQ SYSTEM TIME USED */
	else if(strcmp(token,"sseqtimeused")==0){
	  fprintf(stream,"%ld",bs->system_time);
	}
	/* SEQ TIME USED */
	else if(strcmp(token,"seqtimeused")==0){
	  fprintf(stream,"%ld",bs->user_time+bs->system_time);
	}
	/* SEQ TIME LIMITE */
	else if(strcmp(token,"seqtimelim")==0){
	  fprintf(stream,"%ld",bs->seq_time_limit);
	}
	/* SEQ MEM USED */
	else if(strcmp(token,"seqmemused")==0){
	  fprintf(stream,"%u",bs->seq_mem_used);
	}
	/* SEQ MEM LIMITE */
	else if(strcmp(token,"seqmemlim")==0){
	  fprintf(stream,"%u",bs->seq_mem_limit);
	}
	/* PAR TIME USED */
	else if(strcmp(token,"partimeused")==0){
	  fprintf(stream,"%ld",bs->par_time_used);
	}
	/* PAR TIME LIMITE */
	else if(strcmp(token,"partimelim")==0){
	  fprintf(stream,"%ld",bs->par_time_limit);
	}
	/* PAR MEM USED */
	else if(strcmp(token,"parmemused")==0){
	  fprintf(stream,"%u",bs->par_mem_used);
	}
	/* PAR MEM LIMITE */
	else if(strcmp(token,"parmemlim")==0){
	  fprintf(stream,"%u",bs->par_mem_limit);
	}
	/* PAR CPU NUMBER USED */
	else if(strcmp(token,"parcpunb")==0){
	  fprintf(stream,"%u",bs->par_cores_nb);
	}
	/* PAR CPU NUMBER LIMITE */
	else if(strcmp(token,"parcpunblim")==0){
	  fprintf(stream,"%u",bs->par_cores_nb_limit);
	}
	else if(strcmp(token,"parnodeslist")==0){
	  /* batch session parallele executing hostnames */
	  if(bridge_nodelist_get_compacted_string(&(bs->par_nodelist),&nodelist)==0){
	    fprintf(stream,"%s",nodelist);
	    free(nodelist);
	  }
	  else
	    fprintf(stream,"-");
	}
	else if(strcmp(token,"parnodenames")==0){
	  /* batch session parallele executing hostnames */
	  if(bridge_nodelist_get_extended_string(&(bs->par_nodelist),&nodelist)==0){
	    if(strlen(nodelist)>0)
	      fprintf(stream,"%s",nodelist);
	    else
	      fprintf(stream,"-");
	    free(nodelist);
	  }
	  else
	    fprintf(stream,"-");
	}
	else if(strcmp(token,"rmid")==0){
	  /* batch session corresponding parallel job id */
	  if(bs->rm_id!=NULL){
	    fprintf(stream,"%s",bs->rm_id);
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

int output_fields_require_parallel_infos(char* parallel_fields,char* query_fields){

  int fstatus=0;

  char** string_array=NULL;
  int parallel_fields_nb;
  char* string;

  int i,j;

  int query_fields_nb;

  if(bridge_common_string_get_tokens_quantity(parallel_fields,", ",&parallel_fields_nb)==0){
    string_array=(char**)malloc(parallel_fields_nb*sizeof(char*));
    if(string_array!=NULL){
      for(i=1;i<=parallel_fields_nb;i++){
	string_array[i-1]=NULL;
      }
      for(i=1;i<=parallel_fields_nb;i++){
	string=NULL;
	if(bridge_common_string_get_token(parallel_fields,", ",i,&string)==0)
	  string_array[i-1]=string;
      }
      if(bridge_common_string_get_tokens_quantity(query_fields,",",&query_fields_nb)==0){
	for(i=1;i<=query_fields_nb;i++){
	  string=NULL;
	  if(bridge_common_string_get_token(query_fields,",",i,&string)==0){
	    for(j=1;j<=parallel_fields_nb;j++){
	      if(string_array[j-1]!=NULL)
		if(strcmp(string_array[j-1],string)==0)
		  fstatus=1;
	    }
	    free(string);
	    string=NULL;
	  }
	  if(fstatus==1)
	    break;
	}
      }
      for(i=1;i<=parallel_fields_nb;i++){
	if(string_array[i-1]!=NULL)
	  free(string_array[i-1]);
      }
    }
  }

  return fstatus;

}
