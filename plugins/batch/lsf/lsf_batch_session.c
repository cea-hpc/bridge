#include "config.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <time.h>
#include <math.h>

#include <string.h>

#include "bridge/bridge.h"

#include <lsf/lsbatch.h>

#define DEBUG_LOGGER xdebug
#define DEBUG2_LOGGER xdebug2
#define DEBUG3_LOGGER xdebug3
#define ERROR_LOGGER xerror

#define DEFAULT_PAR_CPUS_NB_LIMIT 0

/* In order to get terminated batch session informations */
#ifndef LSF_WORKDIR
#define LSF_WORKDIR "/usr/share/lsf/work"
#endif
#define LSF_ACCTFILE_SKEL LSF_WORKDIR "/%s/" "logdir/lsb.acct"
#define LSF_ACCTFILE_NB 10

/*
  Initialize bridge_batch_session_t with default values
  -------------------------------------------

  Returns :
  0 on success
  -1 on error

  On succes, you 'll have to free bridge_batch_session_t structure with bridge_rmi_free_node(...)
*/
int init_batch_session(bridge_batch_manager_t* p_batch_manager,
		       bridge_batch_session_t* p_batch_session){

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

    p_batch_session->qos=NULL;
    p_batch_session->queue=NULL;
    p_batch_session->priority=0;

    p_batch_session->executing_hostname=NULL;
    p_batch_session->session_id=0;
    p_batch_session->start_time=INVALID_TIME_VALUE;
    p_batch_session->end_time=INVALID_TIME_VALUE;

    /* init resource usage*/
    //    p_batch_session->used_time=0;
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

    p_batch_session->par_time_used=0; /* must be filled by external call to resources manager system */
    p_batch_session->par_mem_used=0; /* must be filled by external call to resources manager system */
    p_batch_session->par_cores_nb=0; /* must be filled by external call to resources manager system */

    bridge_nodelist_init(&(p_batch_session->par_nodelist),NULL,0);

    fstatus=0;
  }

  return fstatus;

}

/*
  Free a bridge_batch_session_t structure
  -------------------------------

  Returns :
  0 on success
  -1 on error

*/
int clean_batch_session(bridge_batch_manager_t* p_batch_manager,
			bridge_batch_session_t* p_batch_session){

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

    bridge_nodelist_free_contents(&(p_batch_session->par_nodelist));

    fstatus=0;
  }

  return fstatus;

}



int get_batch_sessions(bridge_batch_manager_t* p_batch_manager,
		       bridge_batch_session_t** p_p_batch_sessions, int* p_batch_sessions_nb, char* batch_sessions_batch_ids,
		       char* jobname,char* username,char* batch_queue,char* execHost,int parallel_infos_flag
		       ){

  int fstatus=-1;
  int status;

  int jobs_number=0;
  int stored_jobs_number=0;
  int i,j;

  char buffer[256];

  char* reason;

  char* queue_array[1];
  queue_array[0]=NULL;
  struct queueInfoEnt* p_queueInfo=NULL;
  int queue_nb=0;
  int q;

  struct jobInfoEnt* p_jobInfo;
  
  int jobid=0;

  int cores_nb;
  int mem_value;

  int value_1;
  int value_2;

  char* string_a;
  char* string_b;

  /* check that batch system is running or exit with error 1 */
  if(!ls_getclustername())
    return 1;
  
  /* get queues informations because it's the only way to get
   * time limit if user didn't supply it */
  p_queueInfo=lsb_queueinfo(queue_array,&queue_nb,NULL,NULL,ALL_QUEUE);
  if(p_queueInfo==NULL)
    DEBUG3_LOGGER("unable to get queues informations\n");

  jobs_number=lsb_openjobinfo((batch_sessions_batch_ids==NULL) ? 0 : atoi(batch_sessions_batch_ids), /* Jobid or 0 */
			      jobname, /* Job name or NULL */
			      (username == NULL) ? "all" : username, /* User/Group name or all */
			      batch_queue, /* Queue name or NULL */
			      execHost, /* Execution Host name or NULL */
			      CUR_JOB /* Type of Job */
			      );
  if(jobs_number<0)
    {
      /* this could be because no jobs are currently active... 
       * no good way to handle this problem... */
      *p_batch_sessions_nb=0;
      fstatus=0;
      DEBUG3_LOGGER("unable to get sessions informations\n");
    }
  else
    {
      /* if no jobs curently active, set 0 to fstatus and stored_jobs_number */
      if(jobs_number==0)
	{
	  fstatus=0;
	  stored_jobs_number=0;
	}
      /* at least one job is currently active */
      else
	{
	  /* if input batch session array is not NULL, use it to store the first */
	  /* *p_batch_sessions_nb sessions */
	  if(*p_p_batch_sessions!=NULL)
	    {
	      jobs_number=*p_batch_sessions_nb;
	    }
	  /*allocate a new batch session array */
	  else
	    {
	      *p_p_batch_sessions=(bridge_batch_session_t*)malloc(sizeof(bridge_batch_session_t)*jobs_number);
	      /* unable to allocate memory, set jobs number to 0 in order to jump directly to the */
	      /* end of the function (fstatus equals -1) */
	      if(*p_p_batch_sessions==NULL)
		jobs_number=0;
	    }
	}
      
      /* Fill bridge_batch_session_t structure */
      for(i=0;i<jobs_number;i++){
	/* extract a job from LSF job array */
	p_jobInfo=lsb_readjobinfo(&j);
	if(p_jobInfo==NULL)
	  {
	    DEBUG3_LOGGER("unable to extract job (%d/%d) information from LSF data",i+1,jobs_number);
	    /* problem during extraction, break the loop */
	    break;
	  }
	else
	  {
	    /* init batch session structure */
	    init_batch_session(p_batch_manager,(*p_p_batch_sessions)+stored_jobs_number);

	    /* set batch session id */
	    sprintf(buffer,"%d",p_jobInfo->jobId);
	    if((*p_p_batch_sessions)[stored_jobs_number].batch_id!=NULL)
	      free((*p_p_batch_sessions)[stored_jobs_number].batch_id);
	    (*p_p_batch_sessions)[stored_jobs_number].batch_id=strdup(buffer);

	    /* set batch session name */
	    if(p_jobInfo->submit.jobName!=NULL){
	      if((*p_p_batch_sessions)[stored_jobs_number].name!=NULL)
		free((*p_p_batch_sessions)[stored_jobs_number].name);
	      (*p_p_batch_sessions)[stored_jobs_number].name=strdup(p_jobInfo->submit.jobName);
	    }

	    /* batch session description */
	    /* 	if((*p_p_batch_sessions)[stored_jobs_number].description!=NULL) */
	    /* 	  free((*p_p_batch_sessions)[stored_jobs_number].description); */
	    
	    /* set batch session state */
	    switch(p_jobInfo->status)
	      {
	      case JOB_STAT_RUN  :
		(*p_p_batch_sessions)[stored_jobs_number].state=BRIDGE_BATCH_SESSION_STATE_RUNNING;
		break;
	      case JOB_STAT_PEND  :
		(*p_p_batch_sessions)[stored_jobs_number].state=BRIDGE_BATCH_SESSION_STATE_PENDING;
		break;
	      case JOB_STAT_EXIT  :
		(*p_p_batch_sessions)[stored_jobs_number].state=BRIDGE_BATCH_SESSION_STATE_FAILED;
		break;
	      case JOB_STAT_DONE  :
		(*p_p_batch_sessions)[stored_jobs_number].state=BRIDGE_BATCH_SESSION_STATE_DONE;
		break;
	      case JOB_STAT_USUSP :
	      case JOB_STAT_PSUSP :
	      case JOB_STAT_SSUSP :
		(*p_p_batch_sessions)[stored_jobs_number].state=BRIDGE_BATCH_SESSION_STATE_SUSPENDED;
		break;
	      case JOB_STAT_UNKWN :
	      default :
		(*p_p_batch_sessions)[stored_jobs_number].state=BRIDGE_BATCH_SESSION_STATE_UNKNOWN;
		break;
	      }

	    /* set batch session state reason */
	    if((*p_p_batch_sessions)[stored_jobs_number].reason!=NULL)
	      free((*p_p_batch_sessions)[stored_jobs_number].reason);
	    (*p_p_batch_sessions)[stored_jobs_number].reason=NULL;
	    switch((*p_p_batch_sessions)[stored_jobs_number].state)
	      {
	      case BRIDGE_BATCH_SESSION_STATE_RUNNING :
		sprintf(buffer,"R%02d",p_jobInfo->runRusage.npids);
		(*p_p_batch_sessions)[stored_jobs_number].reason=strdup(buffer);
		break;
	      case BRIDGE_BATCH_SESSION_STATE_PENDING :
		reason=lsb_pendreason(1,&(p_jobInfo->reasonTb[p_jobInfo->numReasons-1]),NULL,NULL,0);
		if(reason!=NULL)
		  reason[strlen(reason)-2]='\0';
		(*p_p_batch_sessions)[stored_jobs_number].reason=strdup(reason);
		break;
	      case BRIDGE_BATCH_SESSION_STATE_SUSPENDED :
		reason=lsb_suspreason(p_jobInfo->reasons,p_jobInfo->subreasons,NULL);
		if(reason!=NULL)
		  reason[strlen(reason)-2]='\0';
		(*p_p_batch_sessions)[stored_jobs_number].reason=strdup(reason);
		break;
	      case BRIDGE_BATCH_SESSION_STATE_DONE :
	      case BRIDGE_BATCH_SESSION_STATE_FAILED :
	      case BRIDGE_BATCH_SESSION_STATE_UNKNOWN :
	      default :
		break;
	      }

	    /* set batch session username */
	    if(p_jobInfo->user!=NULL)
	      {
		if((*p_p_batch_sessions)[stored_jobs_number].username!=NULL)
		  free((*p_p_batch_sessions)[stored_jobs_number].username);
		(*p_p_batch_sessions)[stored_jobs_number].username=strdup(p_jobInfo->user);
	      }
	    
	    /* set batch session user group */
	    if(p_jobInfo->submit.userGroup!=NULL)
	      {
		if((*p_p_batch_sessions)[stored_jobs_number].usergroup!=NULL)
		  free((*p_p_batch_sessions)[stored_jobs_number].usergroup);
		(*p_p_batch_sessions)[stored_jobs_number].usergroup=strdup(p_jobInfo->submit.userGroup);
	      }

	    /* set batch session project name */
	    if(p_jobInfo->submit.projectName!=NULL)
	      {
		if((*p_p_batch_sessions)[stored_jobs_number].projectname!=NULL)
		  free((*p_p_batch_sessions)[stored_jobs_number].projectname);
		(*p_p_batch_sessions)[stored_jobs_number].projectname=strdup(p_jobInfo->submit.projectName);
	      }
	    
	    /* set batch session allocating hostname */
	    if(p_jobInfo->fromHost!=NULL)
	      {
		if((*p_p_batch_sessions)[stored_jobs_number].submit_hostname!=NULL)
		  free((*p_p_batch_sessions)[stored_jobs_number].submit_hostname);
		(*p_p_batch_sessions)[stored_jobs_number].submit_hostname=strdup(p_jobInfo->fromHost);
	      }

	    /* set batch session submit time */
	    if(p_jobInfo->submitTime!=INVALID_TIME_VALUE)
	      (*p_p_batch_sessions)[stored_jobs_number].submit_time=(time_t)p_jobInfo->submitTime;

	    /* set batch session queue */
	    if(p_jobInfo->submit.queue!=NULL)
	      {
		if((*p_p_batch_sessions)[stored_jobs_number].queue!=NULL)
		  free((*p_p_batch_sessions)[stored_jobs_number].queue);
		(*p_p_batch_sessions)[stored_jobs_number].queue=strdup(p_jobInfo->submit.queue);
	      }
	    
	    /* set batch session priority */

	    /* set batch session execution start time */
	    if(p_jobInfo->startTime!=INVALID_TIME_VALUE)
	      (*p_p_batch_sessions)[stored_jobs_number].start_time=(time_t)p_jobInfo->startTime;

	    /* set batch session execution end time */
	    if(p_jobInfo->endTime!=INVALID_TIME_VALUE)
	      (*p_p_batch_sessions)[stored_jobs_number].end_time=(time_t)p_jobInfo->endTime;
	    
	    /* set batch session system time */
/* 	    if ( p_jobInfo->runRusage.stime > 0.0 ) */
/* 		    (*p_p_batch_sessions)[stored_jobs_number].system_time=(uint32_t)floor(p_jobInfo->runRusage.stime); */
/* 	    else */
	    (*p_p_batch_sessions)[stored_jobs_number].system_time=(uint32_t) 0;
	
	    /* set batch session user time */
/* 	    if ( p_jobInfo->runRusage.utime > 0.0 ) */
/* 		    (*p_p_batch_sessions)[stored_jobs_number].user_time=(uint32_t)floor(p_jobInfo->runRusage.utime); */
/* 	    else  */
	    if ( p_jobInfo->runTime > 0.0 )
		    (*p_p_batch_sessions)[stored_jobs_number].user_time=(uint32_t)floor(p_jobInfo->runTime);
	    else
		    (*p_p_batch_sessions)[stored_jobs_number].user_time=0;

	    (*p_p_batch_sessions)[stored_jobs_number].par_time_used=(*p_p_batch_sessions)[stored_jobs_number].user_time +
		    (*p_p_batch_sessions)[stored_jobs_number].system_time;
	
	    /* set batch session executing hostname and memory usage */
	    if(p_jobInfo->numExHosts>0) {
		    if((*p_p_batch_sessions)[stored_jobs_number].executing_hostname!=NULL)
			    free((*p_p_batch_sessions)[stored_jobs_number].executing_hostname);
		    (*p_p_batch_sessions)[stored_jobs_number].executing_hostname=strdup(p_jobInfo->exHosts[0]);

		    (*p_p_batch_sessions)[stored_jobs_number].seq_mem_used=(uint32_t)floor(p_jobInfo->runRusage.swap/
											   (p_jobInfo->numExHosts*1024));
	    }
	    else
		    (*p_p_batch_sessions)[stored_jobs_number].seq_mem_used=(uint32_t)floor(p_jobInfo->runRusage.swap/
											   1024);
	    (*p_p_batch_sessions)[stored_jobs_number].par_mem_used=(*p_p_batch_sessions)[stored_jobs_number].seq_mem_used;

	    /* set batch session session_id */
	    if((*p_p_batch_sessions)[stored_jobs_number].state!=BRIDGE_BATCH_SESSION_STATE_PENDING)
	      (*p_p_batch_sessions)[stored_jobs_number].session_id=(pid_t)p_jobInfo->jobPid;
	    else
	      (*p_p_batch_sessions)[stored_jobs_number].session_id=0;

	    /* set batch session parallel limit */
	    /* first, we get queue 's limit */
	    if(p_queueInfo!=NULL)
	      {
		for(q=0;q<queue_nb;q++)
		  {
		    if(strcmp(p_queueInfo[q].queue,p_jobInfo->submit.queue)==0)
		      {
			if(p_queueInfo[q].rLimits[LSF_RLIMIT_RUN]>0)
			  (*p_p_batch_sessions)[stored_jobs_number].par_time_limit=p_queueInfo[q].rLimits[LSF_RLIMIT_RUN];
			else if(p_queueInfo[q].rLimits[LSF_RLIMIT_CPU]>0)
			  (*p_p_batch_sessions)[stored_jobs_number].par_time_limit=p_queueInfo[q].rLimits[LSF_RLIMIT_CPU];
			if(p_queueInfo[q].rLimits[LSF_RLIMIT_RSS]>0)
			  (*p_p_batch_sessions)[stored_jobs_number].par_mem_limit=p_queueInfo[q].rLimits[LSF_RLIMIT_RSS]/1024;
		      }
		  }
	      }
	    /* then, we get job 's limit */
	    if(p_jobInfo->submit.rLimits[LSF_RLIMIT_RUN]>0)
	      (*p_p_batch_sessions)[stored_jobs_number].par_time_limit=p_jobInfo->submit.rLimits[LSF_RLIMIT_RUN];
	    else if(p_jobInfo->submit.rLimits[LSF_RLIMIT_CPU]>0)
	      (*p_p_batch_sessions)[stored_jobs_number].par_time_limit=p_jobInfo->submit.rLimits[LSF_RLIMIT_CPU];
	    if(p_jobInfo->submit.rLimits[LSF_RLIMIT_RSS]>0)
	      (*p_p_batch_sessions)[stored_jobs_number].par_mem_limit=(p_jobInfo->submit.rLimits[LSF_RLIMIT_RSS]/1024);
	    (*p_p_batch_sessions)[stored_jobs_number].par_cores_nb_limit=p_jobInfo->submit.maxNumProcessors;

	    /* we finaly set sequential limit using parallel limit (LSF doesn't support the 2 kinds) */
	    (*p_p_batch_sessions)[stored_jobs_number].seq_time_limit=(*p_p_batch_sessions)[stored_jobs_number].par_time_limit;
	    (*p_p_batch_sessions)[stored_jobs_number].seq_mem_limit=(*p_p_batch_sessions)[stored_jobs_number].par_mem_limit;

	    /* get job's reserved core nb (meaningful when job is pending) */
	    (*p_p_batch_sessions)[stored_jobs_number].par_cores_nb=p_jobInfo->numExHosts;

	    /* batch session parallele id */
	    (*p_p_batch_sessions)[stored_jobs_number].rm_id=strdup((*p_p_batch_sessions)[stored_jobs_number].batch_id);

	    /* batch session parallele nodes */
	    int y;
	    for (y=0;y<p_jobInfo->numExHosts;y++)
		    bridge_nodelist_add_nodes(&((*p_p_batch_sessions)[stored_jobs_number].par_nodelist),p_jobInfo->exHosts[y]);
	    
	    /* incremente total number of stored jobs */
	    stored_jobs_number++;

	    /* set fstatus to 0 */
	    fstatus=0;
	  }
	
      } /* end of for */

      /* shrink if possible bridge_batch_session_t structure memory area */
      if( 0 < jobs_number && jobs_number >  stored_jobs_number)
	{
	  *p_p_batch_sessions=(bridge_batch_session_t*)realloc(*p_p_batch_sessions,stored_jobs_number*sizeof(bridge_batch_session_t));
	  if(*p_p_batch_sessions==NULL)
	    stored_jobs_number=0;
	}

      /* set the output number of stored batch sessions */
      *p_batch_sessions_nb=stored_jobs_number;

      /* Free LSF data */
      lsb_closejobinfo();

  }

  /* return function status */
  return fstatus;

}




//--------------------------------------------------------------------------------------------------------------------------

/*
  Get batch sessions informations
  -------------------------------
  You don't have to create all bridge_batch_session_t structure, you just have to set parameters
  according to the following rules :

  if batch_sessions_batch_ids equals NULL or "" or "all", get all current sessions, otherwise get only batch_sessions by
  given batch_id

  if p_batch_sessions==NULL :
  - set total batch sessions number in p_batch_sessions_nb
  - allocate a bridge_batch_session_t** containing *p_batch_sessions_nb bridge_batch_session_t*
  - fill the *p_batch_sessions_nb bridge_batch_session_t
  else :
  - get max batch sessions number in *p_batch_sessions_nb
  - fill the *p_batch_sessions_nb bridge_batch_session_t if possible
  - update value of *p_batch_sessions_nb according to 


  Returns :
  0 on success
  1 on succes, but p_nodes_nb contains a new valid value for nodes_nb
  -1 on error

  On succes, you 'll have to clean all nodes with bridge_rmi_clean_node(...) before
  freeing *p_nodes

*/
int get_terminated_batch_sessions(bridge_batch_manager_t* p_batch_manager,
				  bridge_batch_session_t** p_p_batch_sessions, int* p_batch_sessions_nb, char* batch_sessions_batch_ids,
				  char* jobname,char* username,char* batch_queue,char* execHost /* Constraints */,int parallel_infos_flag,
				  time_t begin_eventTime,time_t end_eventTime
				  ){

  int fstatus=-1;
  int status;

  int jobs_number=0;
  int stored_jobs_number=0;
  int i,j;

  char buffer[256];

  char* reason;

  //struct jobInfoEnt* p_jobInfo;
  
  int jobid=0;

  /*
    Defines variables relative to accounting infos
    ----------------------------------------------
  */
  struct eventRec* lsb_acct_record;
  struct jobFinishLog* p_jobInfo;

  int fdnb_max=LSF_ACCTFILE_NB;
  int fdnb_work_min;
  int fdnb_work_max;
  int fdnb;

  char* queue_array[1];
  queue_array[0]=NULL;
  struct queueInfoEnt* p_queueInfo=NULL;
  int queue_nb=0;
  int q;

  FILE** lsb_acct_file_array;
  char* lsb_acct_filename;
  size_t tail_length=4;
  char tail[tail_length];
  char* filename;
  size_t lsb_acct_size;
  FILE* lsb_acct_file=NULL;
  int lsb_acct_file_line_nb;

  int finished_job_events_nb=0;
  int min_lsb_acct_record_timestamp=0;
  int max_lsb_acct_record_timestamp=0;



  char* clustername;

  int value_1;
  int value_2;

  char* string_a;
  char* string_b;

  time_t current_time=0;

  /* all the required are not provided (limits, time usages) */
  /* so abort now */
  return -1;
  
  /*
   * Inverse begin and end time if inverted
   * but not for null value that means no limit
   */
  if((begin_eventTime>end_eventTime) && end_eventTime!=0){
    current_time=end_eventTime;
    end_eventTime=begin_eventTime;
    begin_eventTime=current_time;
  }

  /*
    Build accounting filename
    -------------------------
  */
  clustername=ls_getclustername();
  if(clustername==NULL)
    return 1;
  lsb_acct_size=strlen(clustername)+strlen(LSF_ACCTFILE_SKEL)+1;

  lsb_acct_filename=(char*)malloc(lsb_acct_size*sizeof(char));
  if(lsb_acct_filename==NULL)
    return fstatus;
  else
    snprintf(lsb_acct_filename,lsb_acct_size,LSF_ACCTFILE_SKEL,clustername);
  
  lsb_acct_file_array=(FILE**)malloc(fdnb_max*sizeof(FILE*));
  if(lsb_acct_file_array==NULL){
    free(lsb_acct_filename);
    return fstatus;
  }

  /*
    Check event time filtering
    --------------------------
  */
  int eventTime_filtering_flag=0;
  if(begin_eventTime!=0 || end_eventTime!=0)
    eventTime_filtering_flag=1;

  /*
   * ---------------------------------------------------------
   * Get queues informations because it's the only way to get
   * time limit if user didn't supply it
   * ---------------------------------------------------------
   */
  p_queueInfo=lsb_queueinfo(queue_array,&queue_nb,NULL,NULL,ALL_QUEUE);
  if(p_queueInfo==NULL)
    DEBUG3_LOGGER("unable to get queues informations\n");

  /*
    Open accts file in a FILE* array
    --------------------------------
  */
  for(fdnb=0;fdnb<fdnb_max;fdnb++)
    lsb_acct_file_array[fdnb]=NULL;
  for(fdnb=0;fdnb<fdnb_max;fdnb++){
    if(fdnb>0)
      snprintf(tail,tail_length,".%d",fdnb);
    else
      snprintf(tail,tail_length,"");
    lsb_acct_size=strlen(tail)+strlen(lsb_acct_filename)+1;
    filename=(char*)malloc(lsb_acct_size*sizeof(char));
    if(filename==NULL)
      lsb_acct_file_array[fdnb]=NULL;
    else{
      snprintf(filename,lsb_acct_size,"%s%s",lsb_acct_filename,tail);
      lsb_acct_file_array[fdnb]=fopen(filename,"r");
      free(filename);filename=NULL;
    }
  }

  /*
   * Count requested events in acct files and get id of min and max
   * relevant file in the array
   */
  fdnb_work_min=0;
  fdnb_work_max=fdnb_max;
  for(fdnb=0;fdnb<fdnb_work_max;fdnb++){
    lsb_acct_file=lsb_acct_file_array[fdnb];
    lsb_acct_file_line_nb=0;
    if(lsb_acct_file!=NULL){
      while((lsb_acct_record=lsb_geteventrec(lsb_acct_file,&lsb_acct_file_line_nb))!=NULL){
	if(lsb_acct_record->type==EVENT_JOB_FINISH){
	  /*
	    EventTime filtering
	  */
	  if(eventTime_filtering_flag==1){
	    /*
	     * if we are currently processing a file that stores older events
	     * just change fdnb_work_max in order not to stop process the older
	     * accounting files
	     */
	    if((time_t)lsb_acct_record->eventTime<begin_eventTime && begin_eventTime!=0){
	      fdnb_work_max=fdnb+1;
	      continue;
	    }
	    /*
	     * if we are currently processing a file that stores newer events
	     * just change fdnb_work_min in order not to process the newer
	     * accounting files
	     */
	    if((time_t)lsb_acct_record->eventTime>end_eventTime && end_eventTime!=0){
	      fdnb_work_min=fdnb;
	      continue;
	    }
	  }
	  /*
	   * We store timestamp of former and later events
	   */
	  if(lsb_acct_record->eventTime<min_lsb_acct_record_timestamp || min_lsb_acct_record_timestamp==0)
	    min_lsb_acct_record_timestamp=lsb_acct_record->eventTime;
	  if(lsb_acct_record->eventTime>max_lsb_acct_record_timestamp || max_lsb_acct_record_timestamp==0)
	    max_lsb_acct_record_timestamp=lsb_acct_record->eventTime;
	  finished_job_events_nb++;
	}
      }
    }

  }

  jobs_number=finished_job_events_nb;

  /*
   * Create bridge_batch_session array
   */
  if(jobs_number<0){
    *p_batch_sessions_nb=0;
  }
  else{

    if(*p_p_batch_sessions!=NULL){
      jobs_number=*p_batch_sessions_nb;
    }
    else{
      *p_p_batch_sessions=(bridge_batch_session_t*)malloc(sizeof(bridge_batch_session_t)*jobs_number);
      if(*p_p_batch_sessions==NULL)
	jobs_number=0;
    }

    /*
      Get batch session infos using acct file between min and max ID
      --------------------------------------------------------------
    */
    i=0;
    for(fdnb=fdnb_work_min;fdnb<fdnb_work_max;fdnb++){
      lsb_acct_file=lsb_acct_file_array[fdnb];
      if(lsb_acct_file!=NULL){
	/*
	 * Rewind acct file
	 */
	fseek(lsb_acct_file, 0L, SEEK_SET);
	/*
	  -----------------------------------------
	  Fill bridge_batch_session_t structure
	  -----------------------------------------
	*/
	for(i;i<jobs_number;i++){
	  lsb_acct_file_line_nb=0;
	  lsb_acct_record=lsb_geteventrec(lsb_acct_file,&lsb_acct_file_line_nb);
	  if(lsb_acct_record==NULL || lsb_acct_record->type!=EVENT_JOB_FINISH)
	    break;
	  else{
	    /*
	      EventTime filtering
	      -------------------
	    */
	    if(eventTime_filtering_flag==1){
	      if(((time_t)lsb_acct_record->eventTime<begin_eventTime && begin_eventTime!=0) ||
		 ((time_t)lsb_acct_record->eventTime>end_eventTime && end_eventTime!=0)
		 ){
		i--;
		continue;
	      }
	    }
	    /**/

	    /*
	      We just store counted finished jobs
	      -----------------------------------
	    */
	    if(lsb_acct_record->eventTime<min_lsb_acct_record_timestamp ||
	       lsb_acct_record->eventTime>max_lsb_acct_record_timestamp){
	      i--;
	      continue;
	    }

	    /*
	      Save pointer on jobFinishLog structure
	      --------------------------------------
	    */
	    p_jobInfo=&(lsb_acct_record->eventLog.jobFinishLog);

	    /*
	      Filter finished jobs according to parameters
	      --------------------------------------------
	    */
	    if((username!=NULL && strncmp(username,p_jobInfo->userName,strlen(username))!=0) && strncmp(username,"all",4)!=0)
	      continue;
	    sprintf(buffer,"%d",p_jobInfo->jobId);
	    if(batch_sessions_batch_ids!=NULL && strncmp(batch_sessions_batch_ids,buffer,strlen(batch_sessions_batch_ids))!=0)
	      continue;
	    if(batch_queue!=NULL && strncmp(batch_queue,p_jobInfo->queue,strlen(batch_queue))!=0)
	      continue;
	    if(execHost!=NULL){
	      if(p_jobInfo->numExHosts>0){
		if(strncmp(execHost,p_jobInfo->execHosts[0],strlen(execHost))!=0)
		  continue;
	      }
	      else
		continue;
	    }
	    /*
	      Initialize batch session structure
	      ----------------------------------
	    */
	    init_batch_session(p_batch_manager,(*p_p_batch_sessions)+stored_jobs_number);

	    /* batch session id */
	    sprintf(buffer,"%d",p_jobInfo->jobId);
	    if((*p_p_batch_sessions)[stored_jobs_number].batch_id!=NULL)
	      free((*p_p_batch_sessions)[stored_jobs_number].batch_id);
	    (*p_p_batch_sessions)[stored_jobs_number].batch_id=strdup(buffer);
	    (*p_p_batch_sessions)[stored_jobs_number].rm_id=strdup(
		    (*p_p_batch_sessions)[stored_jobs_number].batch_id);

	    /* batch session name */
	    if(p_jobInfo->jobName!=NULL){
	      if((*p_p_batch_sessions)[stored_jobs_number].name!=NULL)
		free((*p_p_batch_sessions)[stored_jobs_number].name);
	      (*p_p_batch_sessions)[stored_jobs_number].name=strdup(p_jobInfo->jobName);
	    }

	    /* 	  /\* batch session description *\/ */
	    /* 	  if((*p_p_batch_sessions)[stored_jobs_number].description!=NULL) */
	    /* 	    free((*p_p_batch_sessions)[stored_jobs_number].description); */
	  
	    /* batch session state */
	    switch(p_jobInfo->jStatus){
	    case JOB_STAT_RUN  :
	      (*p_p_batch_sessions)[stored_jobs_number].state=BRIDGE_BATCH_SESSION_STATE_RUNNING;
	      break;
	    case JOB_STAT_PEND  :
	      (*p_p_batch_sessions)[stored_jobs_number].state=BRIDGE_BATCH_SESSION_STATE_PENDING;
	      break;
	    case JOB_STAT_EXIT  :
	      (*p_p_batch_sessions)[stored_jobs_number].state=BRIDGE_BATCH_SESSION_STATE_FAILED;
	      break;
	    case JOB_STAT_DONE  :
	      (*p_p_batch_sessions)[stored_jobs_number].state=BRIDGE_BATCH_SESSION_STATE_DONE;
	      //	    fprintf(stdout,"Status : %d|%d\n",(*p_p_batch_sessions)[stored_jobs_number].state,JOB_STAT_DONE);
	      break;
	    case JOB_STAT_USUSP :
	    case JOB_STAT_PSUSP :
	    case JOB_STAT_SSUSP :
	      (*p_p_batch_sessions)[stored_jobs_number].state=BRIDGE_BATCH_SESSION_STATE_SUSPENDED;
	      break;
	    case JOB_STAT_UNKWN :
	    default :
	      (*p_p_batch_sessions)[stored_jobs_number].state=BRIDGE_BATCH_SESSION_STATE_UNKNOWN;
	      break;
	    }

	    /* batch session username */
	    if(p_jobInfo->userName!=NULL){
	      if((*p_p_batch_sessions)[stored_jobs_number].username!=NULL)
		free((*p_p_batch_sessions)[stored_jobs_number].username);
	      (*p_p_batch_sessions)[stored_jobs_number].username=strdup(p_jobInfo->userName);
	    }

	    /* 	/\* batch session user group *\/ */
	    /* 	if(p_jobInfo->submit.userGroup!=NULL){ */
	    /* 	  if((*p_p_batch_sessions)[stored_jobs_number].usergroup!=NULL) */
	    /* 	    free((*p_p_batch_sessions)[stored_jobs_number].usergroup); */
	    /* 	  (*p_p_batch_sessions)[stored_jobs_number].usergroup=strdup(p_jobInfo->submit.userGroup); */
	    /* 	} */

	    /* batch session project name */
	    if(p_jobInfo->projectName!=NULL){
	      if((*p_p_batch_sessions)[stored_jobs_number].projectname!=NULL)
		free((*p_p_batch_sessions)[stored_jobs_number].projectname);
	      (*p_p_batch_sessions)[stored_jobs_number].projectname=strdup(p_jobInfo->projectName);
	    }
	
	    /* batch session allocating hostname */
	    if(p_jobInfo->fromHost!=NULL){
	      if((*p_p_batch_sessions)[stored_jobs_number].submit_hostname!=NULL)
		free((*p_p_batch_sessions)[stored_jobs_number].submit_hostname);
	      (*p_p_batch_sessions)[stored_jobs_number].submit_hostname=strdup(p_jobInfo->fromHost);
	    }

	    /* batch session submit time */
	    if(p_jobInfo->submitTime!=INVALID_TIME_VALUE)
	      (*p_p_batch_sessions)[stored_jobs_number].submit_time=(time_t)p_jobInfo->submitTime;

	    /* batch session queue */
	    if(p_jobInfo->queue!=NULL){
	      if((*p_p_batch_sessions)[stored_jobs_number].queue!=NULL)
		free((*p_p_batch_sessions)[stored_jobs_number].queue);
	      (*p_p_batch_sessions)[stored_jobs_number].queue=strdup(p_jobInfo->queue);
	    }

	    /* 	/\* batch session priority *\/ */

	    /* batch session executing hostname */
	    if(p_jobInfo->numExHosts>0){
	      if((*p_p_batch_sessions)[stored_jobs_number].executing_hostname!=NULL)
		free((*p_p_batch_sessions)[stored_jobs_number].executing_hostname);
	      (*p_p_batch_sessions)[stored_jobs_number].executing_hostname=strdup(p_jobInfo->execHosts[0]);
	    }

	    /* batch session execution start time */
	    if(p_jobInfo->startTime!=INVALID_TIME_VALUE)
	      (*p_p_batch_sessions)[stored_jobs_number].start_time=(time_t)p_jobInfo->startTime;

	    /* batch session execution end time */
	    if(p_jobInfo->endTime!=INVALID_TIME_VALUE)
	      (*p_p_batch_sessions)[stored_jobs_number].end_time=(time_t)p_jobInfo->endTime;

	    /* batch session used time */
	    //	(*p_p_batch_sessions)[stored_jobs_number].used_time=(float)p_jobInfo->cpuTime;

	    /* batch session system time */
	    (*p_p_batch_sessions)[stored_jobs_number].system_time=(uint32_t)floor(p_jobInfo->lsfRusage.ru_stime);
	
	    /* batch session user time */
	    (*p_p_batch_sessions)[stored_jobs_number].user_time=(uint32_t)floor(p_jobInfo->lsfRusage.ru_utime);

	    /* batch session parallel time */
	    (*p_p_batch_sessions)[stored_jobs_number].par_time_used=(uint32_t)p_jobInfo->runtimeEstimation;
	
	    /* batch session memory usage */
	    // unavailable
	    if ( p_jobInfo->numExHosts > 0 )
		    (*p_p_batch_sessions)[stored_jobs_number].seq_mem_used=(uint32_t)floor(p_jobInfo->maxRSwap/
											   (p_jobInfo->numExHosts*1024));
	    else
		    (*p_p_batch_sessions)[stored_jobs_number].seq_mem_used=(uint32_t)floor(p_jobInfo->maxRSwap/
											   1024);
	    (*p_p_batch_sessions)[stored_jobs_number].par_mem_used=(*p_p_batch_sessions)[stored_jobs_number].seq_mem_used;

	    /* MOD CCRT MOD CCRT MOD CCRT MOD CCRT MOD CCRT MOD CCRT MOD CCRT MOD CCRT MOD CCRT MOD CCRT MOD CCRT MOD CCRT MOD CCRT */
	    /* MOD CCRT MOD CCRT MOD CCRT MOD CCRT MOD CCRT MOD CCRT MOD CCRT MOD CCRT MOD CCRT MOD CCRT MOD CCRT MOD CCRT MOD CCRT */
	    /* MOD CCRT MOD CCRT MOD CCRT MOD CCRT MOD CCRT MOD CCRT MOD CCRT MOD CCRT MOD CCRT MOD CCRT MOD CCRT MOD CCRT MOD CCRT */

	    if(p_queueInfo!=NULL){
	      for(q=0;q<queue_nb;q++){
		if(strcmp(p_queueInfo[q].queue,p_jobInfo->queue)==0){
		  if(p_queueInfo[q].rLimits[LSF_RLIMIT_RUN]>0)
		    (*p_p_batch_sessions)[stored_jobs_number].par_time_limit=p_queueInfo[q].rLimits[LSF_RLIMIT_RUN];
		  else if(p_queueInfo[q].rLimits[LSF_RLIMIT_CPU]>0)
		    (*p_p_batch_sessions)[stored_jobs_number].par_time_limit=p_queueInfo[q].rLimits[LSF_RLIMIT_CPU];
		  if(p_queueInfo[q].rLimits[LSF_RLIMIT_RSS]>0)
		    (*p_p_batch_sessions)[stored_jobs_number].par_mem_limit=p_queueInfo[q].rLimits[LSF_RLIMIT_RSS]/1024;
		}
	      }
	    }
	    /*
	     * batch session sequential limit
	     * ------------------------------
	     */	
	    (*p_p_batch_sessions)[stored_jobs_number].seq_time_limit=(*p_p_batch_sessions)[stored_jobs_number].par_time_limit;
	    (*p_p_batch_sessions)[stored_jobs_number].seq_mem_limit=(*p_p_batch_sessions)[stored_jobs_number].par_mem_limit;
	
	    /*
	     * Batch session parallele usage
	     * -----------------------------
	     */
	    (*p_p_batch_sessions)[stored_jobs_number].par_cores_nb=p_jobInfo->numExHosts;
	    /* cores nb limit */
	    (*p_p_batch_sessions)[stored_jobs_number].par_cores_nb_limit=p_jobInfo->numProcessors;


	    /* batch session parallele nodes */
	    int y;
	    for (y=0;y<p_jobInfo->numExHosts;y++)
		    bridge_nodelist_add_nodes(&((*p_p_batch_sessions)[stored_jobs_number].par_nodelist),p_jobInfo->execHosts[y]);

	    /* FIN MOD CCRT FIN MOD CCRT FIN MOD CCRT FIN MOD CCRT FIN MOD CCRT FIN MOD CCRT FIN MOD CCRT FIN MOD CCRT FIN MOD CCRT */
	    /* FIN MOD CCRT FIN MOD CCRT FIN MOD CCRT FIN MOD CCRT FIN MOD CCRT FIN MOD CCRT FIN MOD CCRT FIN MOD CCRT FIN MOD CCRT */
	    /* FIN MOD CCRT FIN MOD CCRT FIN MOD CCRT FIN MOD CCRT FIN MOD CCRT FIN MOD CCRT FIN MOD CCRT FIN MOD CCRT FIN MOD CCRT */
	    stored_jobs_number++;
	  }

	}

      }

    }

    /*
      -------------------------------------------------------------------
      Shrink if possible bridge_batch_session_t structure memory area 
      -------------------------------------------------------------------
    */
    if( 0 < jobs_number && jobs_number >  stored_jobs_number){
      *p_p_batch_sessions=(bridge_batch_session_t*)realloc(*p_p_batch_sessions,stored_jobs_number*sizeof(bridge_batch_session_t));
      if(*p_p_batch_sessions==NULL)
	stored_jobs_number=0;
    }

    *p_batch_sessions_nb=stored_jobs_number;

    lsb_closejobinfo();
    fstatus=0;

  }

  for(fdnb=0;fdnb<fdnb_max;fdnb++){
    if(lsb_acct_file_array[fdnb]!=NULL)
      fclose(lsb_acct_file_array[fdnb]);
  }

  if(lsb_acct_filename!=NULL)
    free(lsb_acct_filename);

  return fstatus;

}
