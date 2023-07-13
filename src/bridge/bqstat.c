/*****************************************************************************\
 *  src/bridge/bqstat.c - 
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
 * \brief display batch queue information on file stream in an extended form 
 * \internal
 *
 * \param stream FILE* on which to write output
 * \param bq pointer on a batch queue structure to display
 *
 * \retval 0 on success
 * \retval -1 on failure
 */
int display_bridge_batch_queue_on_file_stream(FILE* stream,bridge_batch_queue_t* bq);

/*!
 * \brief display batch queue information on file stream in an classic form 
 * \internal
 *
 * \param stream FILE* on which to write output
 * \param bq pointer on a batch queue structure to display
 *
 * \retval 0 on success
 * \retval -1 on failure
 */
int display_classic_bridge_batch_queue_on_file_stream(FILE * stream,bridge_batch_queue_t* bq);

/*!
 * \brief display required information about batch queue on file stream 
 * \internal
 *
 * \param stream FILE* on which to write output
 * \param bq pointer on a batch queue structure to display
 * \param output_fields comma separated list of information to display
 * \param separator string to write on stream between each required information
 *
 * \retval 0 on success
 * \retval -1 on failure
 */
int display_by_fields_bridge_batch_queue_on_file_stream(FILE* stream,bridge_batch_queue_t* bq,char* output_fields,char* separator);

int main(int argc,char **argv){

  int fstatus=-1;
  int status;

  char* progname;
  char* cb_version;

  bridge_manager_t manager;

  bridge_batch_queue_t* p_batch_queue_array=NULL;
  int batch_queue_nb=0;
  int i;
  int verbosity=0;

  char* queue_name=NULL;

  char* options_desc=
    "\t-h\t\t\tPrint this message\n"
    "\t-c\t\t\tDisplay results in classic format\n"
    "\t-q queue\t\tDisplay results concerning this batch queue\n"
    "\t-o fields\t\tDisplay information using fields format (use -o list for available fields, -o all to get infos for all fields)\n"
    "\t-s separator\t\tWhen used with -o options, enables to change results fields separator\n\t\t\t\t(default is a single spaced string)\n"
    "\t-v\t\t\tWhen used with -o options, the first output line displays list of selected fields\n"
    "\t-V\t\t\tPrint bridge and app versions and exit\n";

  char* fields_list="\n"
    "\tname          : queue name\n"
    "\tdesc          : queue description\n"
    "\tdefault       : default queue (yes|no)\n"
    "\tstate         : queue state (open|close)\n"
    "\tactivity      : queue activity (active|inactive)\n"
    "\tpriority      : queue priority\n"
    "\tjobs          : current number of jobs in this queue\n"
    "\tjobslim       : max number of jobs in this queue\n"
    "\thostjobslim   : max number of jobs per execution host in this queue\n"
    "\tuserjobslim   : max number of jobs per user in this queue\n"
    "\trun           : number of running jobs in this queue\n"
    "\tpen           : number of pending jobs in this queue\n"
    "\tususp         : number of user suspended jobs in this queue\n"
    "\tssusp         : number of system suspended jobs in this queue\n"
    "\tminseqtime    : min sequential time for jobs in this queue\n"
    "\tmaxseqtime    : max sequential time for jobs in this queue\n"
    "\tminseqmem     : min sequential memory usage (Mo) for jobs in this queue\n"
    "\tmaxseqmem     : max sequential memory usage (Mo) for jobs in this queue\n"
    "\tminpartime    : min parallele time for jobs in this queue\n"
    "\tmaxpartime    : max parallele time for jobs in this queue\n"
    "\tminparmem     : min parallele memory usage (Mo) for jobs in this queue\n"
    "\tmaxparmem     : max parallele memory usage (Mo) for jobs in this queue\n"
    "\tminparcpunb   : min parallele cores number for jobs in this queue\n"
    "\tmaxparcpunb   : max parallele cores number for jobs in this queue\n";

  char* output_fields=NULL;
  char* separator=NULL;

#define EXTENDED_DISPLAY 0
#define COMPOSITE_DISPLAY 1
#define CLASSIC_DISPLAY 2
  int display_mode=EXTENDED_DISPLAY;

  char * optstring="hq:o:s:cvV";
  int option;

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
    case 'q' :
      queue_name=strdup(optarg);
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
	output_fields=strdup("name,desc,default,state,activity,priority,jobs,jobslim,hostjobslim,userjobslim,run,pen,ususp,ssusp,minseqtime,maxseqtime,minseqmem,maxseqmem,minpartime,maxpartime,minparmem,maxparmem,minparcpunb,maxparcpunb");
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
      fprintf(stdout,"\nUsage : %s [-h] [-q queue] [-o fields [-s separator]] \n\n",argv[0]);
      fprintf(stdout,"%s\n",options_desc);
      exit(1);
      break;
    }
  }


  // initialize bridge configuration
  if(bridge_init_manager(&manager)==0){

    status=bridge_get_batch_queues(&manager,&p_batch_queue_array,&batch_queue_nb,queue_name);
    if(status==0){

      switch(display_mode){

      case EXTENDED_DISPLAY :
	for(i=0;i<batch_queue_nb;i++){
	  display_bridge_batch_queue_on_file_stream(stdout,p_batch_queue_array+i);
	}
	break;

      case CLASSIC_DISPLAY :
	display_classic_bridge_batch_queue_on_file_stream(stdout,NULL);
	for(i=0;i<batch_queue_nb;i++){
	  display_classic_bridge_batch_queue_on_file_stream(stdout,p_batch_queue_array+i);
	}
	break;

      case COMPOSITE_DISPLAY :
	if(verbosity)
	  display_by_fields_bridge_batch_queue_on_file_stream(stdout,NULL,
								  output_fields,separator);
	for(i=0;i<batch_queue_nb;i++){
	  display_by_fields_bridge_batch_queue_on_file_stream(stdout,p_batch_queue_array+i,
								  output_fields,separator);
	}
	break;

      }

      /*
	Destroy batch queues structures
	--------------------------------
      */
      for(i=0;i<batch_queue_nb;i++){
	bridge_clean_batch_queue(&manager,p_batch_queue_array+i);
      }
      free(p_batch_queue_array);

      fstatus=0;
    }
    bridge_clean_manager(&manager);
  }
  else{
    fstatus=1;
  }

  /* Get sequential jobs information */

  exit :
    
  if(queue_name!=NULL)
    free(queue_name);
  if(output_fields!=NULL)
    free(output_fields);
  if(separator!=NULL)
    free(separator);

  return fstatus;

}



int display_bridge_batch_queue_on_file_stream(FILE* stream,bridge_batch_queue_t* bq){

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

  fprintf(stream,"Default \t\t: %s\n",(bq->default_queue==1)? "yes" : "no");

  fprintf(stream,"Priority \t\t: %d\n",bq->priority);

  char* status;
  switch(bq->state){
  case BRIDGE_BATCH_QUEUE_STATE_OPENED :
    status="open";
    break;
  case BRIDGE_BATCH_QUEUE_STATE_CLOSED :
    status="close";
    break;
  default:
    status="unknown";
  }
  fprintf(stream,"State \t\t\t: %s\n",status);

  char* activity;
  switch(bq->activity){
  case BRIDGE_BATCH_QUEUE_ACTIVITY_ACTIVE :
    activity="active";
    break;
  case BRIDGE_BATCH_QUEUE_ACTIVITY_INACTIVE :
    activity="inactive";
    break;
  default:
    activity="unknown";
  }
  fprintf(stream,"Activity \t\t: %s\n",activity);

  if(bq->jobs_nb_limit!=NO_LIMIT)
    fprintf(stream,"Max Jobs Nb \t\t: %u\n",bq->jobs_nb_limit);
  else
    fprintf(stream,"Max Jobs Nb \t\t: -\n");

  if(bq->perHost_jobs_nb_limit!=NO_LIMIT)
    fprintf(stream,"Per Host Job limit \t: %u\n",bq->perHost_jobs_nb_limit);
  else
    fprintf(stream,"Per Host Job limit \t: -\n");

  if(bq->perUser_jobs_nb_limit!=NO_LIMIT)
    fprintf(stream,"Per User Job limit \t: %u\n",bq->perUser_jobs_nb_limit);
  else
    fprintf(stream,"Per User Job limit \t: -\n");

  fprintf(stream,"Jobs Nb \t\t: %u\n",bq->jobs_nb);

  fprintf(stream,"  RUNNING \t\t: %u\n",bq->running_jobs_nb);

  fprintf(stream,"  PENDING \t\t: %u\n",bq->pending_jobs_nb);

  fprintf(stream,"  USUSPENDED \t\t: %u\n",bq->usersuspended_jobs_nb);

  fprintf(stream,"  SSUSPENDED \t\t: %u\n",bq->syssuspended_jobs_nb);

  if(bq->seq_time_min!=NO_LIMIT)
    fprintf(stream,"Seq time min \t\t: %ld\n",bq->seq_time_min);
  else
    fprintf(stream,"Seq time min \t\t: -\n");

  if(bq->seq_time_max!=NO_LIMIT)
    fprintf(stream,"Seq time max \t\t: %ld\n",bq->seq_time_max);
  else
    fprintf(stream,"Seq time max \t\t: -\n");

  if(bq->seq_mem_min!=NO_LIMIT)
    fprintf(stream,"Seq mem min \t\t: %u\n",bq->seq_mem_min);
  else
    fprintf(stream,"Seq mem min \t\t: -\n");

  if(bq->seq_mem_max!=NO_LIMIT)
    fprintf(stream,"Seq mem max \t\t: %u\n",bq->seq_mem_max);
  else
    fprintf(stream,"Seq mem max \t\t: -\n");

  if(bq->par_time_min!=NO_LIMIT)
    fprintf(stream,"Par time min \t\t: %ld\n",bq->par_time_min);
  else
    fprintf(stream,"Par time min \t\t: -\n");

  if(bq->par_time_max!=NO_LIMIT)
    fprintf(stream,"Par time max \t\t: %ld\n",bq->par_time_max);
  else
    fprintf(stream,"Par time max \t\t: -\n");

  if(bq->par_mem_min!=NO_LIMIT)
    fprintf(stream,"Par mem min \t\t: %u\n",bq->par_mem_min);
  else
    fprintf(stream,"Par mem min \t\t: -\n");

  if(bq->par_mem_max!=NO_LIMIT)
    fprintf(stream,"Par mem max \t\t: %u\n",bq->par_mem_max);
  else
    fprintf(stream,"Par mem max \t\t: -\n");

  if(bq->par_cores_nb_min!=NO_LIMIT)
    fprintf(stream,"Par cpu nb min \t\t: %u\n",bq->par_cores_nb_min);
  else
    fprintf(stream,"Par cpu nb min \t\t: -\n");

  if(bq->par_cores_nb_max!=NO_LIMIT)
    fprintf(stream,"Par cpu nb max \t\t: %u\n",bq->par_cores_nb_max);
  else
    fprintf(stream,"Par cpu nb max \t\t: -\n");


  fprintf(stream,
	  "-------------------------------------------------------\n");
  return 0;
}

int display_classic_bridge_batch_queue_on_file_stream(FILE * stream,bridge_batch_queue_t* bq){
  int fstatus=0;
  
  /* if bs equals NULL, display header */
  if(bq==NULL){
    fprintf(stdout,"%-12s %8s %-8s\n","QUEUE","DEFAULT","CONDITION");
  }
  else{
    fprintf(stdout,"%-12s %8s ",(bq->name!=NULL)?bq->name:"-",(bq->default_queue==1)? "D" : "");

    if ( bq->description != NULL ) {
      fprintf(stdout,"%-s\n",bq->description);
    }
    else {

      if(bq->par_time_min!=NO_LIMIT)
	fprintf(stream,"time_limit>%ld ",bq->par_time_min-1);

      if(bq->par_time_max!=NO_LIMIT)
	fprintf(stream,"time_limit<%ld ",bq->par_time_max+1);

      if(bq->par_mem_min!=NO_LIMIT)
	fprintf(stream,"mem_limit>%u ",bq->par_mem_min-1);

      if(bq->par_mem_max!=NO_LIMIT)
	fprintf(stream,"mem_limit<%u ",bq->par_mem_max+1);

      if(bq->par_cores_nb_max!=NO_LIMIT)
	fprintf(stream,"proc_limit<%u",bq->par_cores_nb_max+1);

      if(bq->par_cores_nb_min!=NO_LIMIT && \
	 bq->par_cores_nb_min != bq->par_cores_nb_max )
	fprintf(stream,"proc_limit>%u ",bq->par_cores_nb_min-1);
      
      fprintf(stream,"\n");

    }

  }
  return fstatus;
}

int display_by_fields_bridge_batch_queue_on_file_stream(FILE* stream,bridge_batch_queue_t* bq,char* output_fields,char* separator){

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

	/* Queue name */
	else if(strcmp(token,"name")==0){
	  if(bq->name!=NULL)
	    fprintf(stream,"%s",bq->name);
	  else
	    fprintf(stream,"-");
	}
	/* Queue description */
	else if(strcmp(token,"desc")==0){
	  if(bq->description!=NULL)
	    fprintf(stream,"%s",bq->description);
	  else
	    fprintf(stream,"-");
	}
	/* Queue default flag */
	else if(strcmp(token,"default")==0){
	  fprintf(stream,"%s",(bq->default_queue==1)? "yes" : "no");
	}
	/* Queue state */
	else if(strcmp(token,"state")==0){
	  char* status;
	  switch(bq->state){
	  case BRIDGE_BATCH_QUEUE_STATE_OPENED :
	    status="open";
	    break;
	  case BRIDGE_BATCH_QUEUE_STATE_CLOSED :
	    status="close";
	    break;
	  default :
	    status="unknown";
	  }
	  fprintf(stream,"%s",status);
	}
	/* Queue activity */
	else if(strcmp(token,"activity")==0){
	  char* activity;
	  switch(bq->activity){
	  case BRIDGE_BATCH_QUEUE_ACTIVITY_ACTIVE :
	    activity="active";
	    break;
	  case BRIDGE_BATCH_QUEUE_ACTIVITY_INACTIVE :
	    activity="inactive";
	    break;
	  default :
	    activity="unknown";
	  }
	  fprintf(stream,"%s",activity);
	}
	/* Queue activity */
	else if(strcmp(token,"priority")==0){
	  fprintf(stream,"%u",bq->priority);
	}
	/* Queue jobs nb */
	else if(strcmp(token,"jobs")==0){
	  fprintf(stream,"%u",bq->jobs_nb);
	}
	/* Queue jobs nb limit */
	else if(strcmp(token,"jobslim")==0){
	  if(bq->jobs_nb_limit!=NO_LIMIT)
	    fprintf(stream,"%u",bq->jobs_nb_limit);
	  else
	    fprintf(stream,"-");
	}
	/* Queue per Host jobs nb limit */
	else if(strcmp(token,"hostjobslim")==0){
	  if(bq->perHost_jobs_nb_limit!=NO_LIMIT)
	    fprintf(stream,"%u",bq->perHost_jobs_nb_limit);
	  else
	    fprintf(stream,"-");
	}
	/* Queue per user jobs nb limit */
	else if(strcmp(token,"userjobslim")==0){
	  if(bq->perUser_jobs_nb_limit!=NO_LIMIT)
	    fprintf(stream,"%u",bq->perUser_jobs_nb_limit);
	  else
	    fprintf(stream,"-");
	}
	/* Queue running job */
	else if(strcmp(token,"run")==0){
	  fprintf(stream,"%u",bq->running_jobs_nb);
	}
	/* Queue pending job */
	else if(strcmp(token,"pen")==0){
	  fprintf(stream,"%u",bq->pending_jobs_nb);
	}
	/* Queue user suspended job */
	else if(strcmp(token,"ususp")==0){
	  fprintf(stream,"%u",bq->usersuspended_jobs_nb);
	}
	/* Queue system suspended job */
	else if(strcmp(token,"ssusp")==0){
	  fprintf(stream,"%u",bq->syssuspended_jobs_nb);
	}

	/* Queue minimum sequential time */
	else if(strcmp(token,"minseqtime")==0){
	  if(bq->seq_time_min!=NO_LIMIT)
	    fprintf(stream,"%ld",bq->seq_time_min);
	  else
	    fprintf(stream,"-");
	}
	/* Queue max sequential time */
	else if(strcmp(token,"maxseqtime")==0){
	  if(bq->seq_time_max!=NO_LIMIT)
	    fprintf(stream,"%ld",bq->seq_time_max);
	  else
	    fprintf(stream,"-");
	}
	/* Queue min sequential mem usage */
	else if(strcmp(token,"minseqmem")==0){
	  if(bq->seq_mem_min!=NO_LIMIT)
	    fprintf(stream,"%u",bq->seq_mem_min);
	  else
	    fprintf(stream,"-");
	}
	/* Queue max sequential mem usage */
	else if(strcmp(token,"maxseqmem")==0){
	  if(bq->seq_mem_max!=NO_LIMIT)
	    fprintf(stream,"%u",bq->seq_mem_max);
	  else
	    fprintf(stream,"-");
	}

	/* Queue minimum parallele time */
	else if(strcmp(token,"minpartime")==0){
	  if(bq->par_time_min!=NO_LIMIT)
	    fprintf(stream,"%ld",bq->par_time_min);
	  else
	    fprintf(stream,"-");
	}
	/* Queue max parallele time */
	else if(strcmp(token,"maxpartime")==0){
	  if(bq->par_time_max!=NO_LIMIT)
	    fprintf(stream,"%ld",bq->par_time_max);
	  else
	    fprintf(stream,"-");
	}
	/* Queue min parallele mem usage */
	else if(strcmp(token,"minparmem")==0){
	  if(bq->par_mem_min!=NO_LIMIT)
	    fprintf(stream,"%u",bq->par_mem_min);
	  else
	    fprintf(stream,"-");
	}
	/* Queue max parallele mem usage */
	else if(strcmp(token,"maxparmem")==0){
	  if(bq->par_mem_max!=NO_LIMIT)
	    fprintf(stream,"%u",bq->par_mem_max);
	  else
	    fprintf(stream,"-");
	}
	/* Queue min parallele cores usage */
	else if(strcmp(token,"minparcpunb")==0){
	  if(bq->par_cores_nb_min!=NO_LIMIT)
	    fprintf(stream,"%u",bq->par_cores_nb_min);
	  else
	    fprintf(stream,"-");
	}
	/* Queue max parallele cores usage*/
	else if(strcmp(token,"maxparcpunb")==0){
	  if(bq->par_cores_nb_max!=NO_LIMIT)
	    fprintf(stream,"%u",bq->par_cores_nb_max);
	  else
	    fprintf(stream,"-");
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
