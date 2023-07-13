/*****************************************************************************\
 *  src/bridged/bridged.c - 
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

#include <getopt.h>
#include <signal.h>

#include <limits.h>

#include "xternal/xqueue.h"
#include "xternal/xstream.h"
#include "xternal/xlogger.h"
#include "xternal/xmessage.h"

#include "bridged/bridge_rus.h"
#include "bridged/bridge_request.h"

#include "bridged_engine.h"

#include "worker.h"

#ifndef PTHREAD_STACK_MIN
#define PTHREAD_STACK_MIN 16384
#endif

#define VERBOSE xverbose
#define VERBOSE2 xverbose2
#define VERBOSE3 xverbose3
#define ERROR VERBOSE
#define ERROR2 VERBOSE
#define ERROR3 VERBOSE

#define XFREE(a) if( a != NULL) { free(a); a=NULL;};

/* static volatile variables for signal handler messages */
static volatile int eof_main_loop_flag;
static volatile int eof_worker_flag;
static volatile int print_stats_flag;
static volatile int exit_flag;
static volatile int reload_flag;

/* signal handler */
void signal_handler(int signum){
  switch(signum){
  case SIGTERM :
    exit_flag=1;
    eof_main_loop_flag=1;
    break;
  case SIGUSR2 :
    print_stats_flag=1;
    break;
  case SIGHUP :
    reload_flag=1;
    eof_main_loop_flag=1;
    print_stats_flag=1;
    break;
  default:
    break;
  }
}

int
dispatcher_main_function(bridged_engine_t* engine,xqueue_t* socket_queue)
{
  int fstatus=-1;

  int socket;
  int incoming_socket;

  char* hostname;
  char* port;

  unsigned long successful_dispatch=0;
  int queued_item_nb;

  hostname=engine->address;
  port=engine->port;

  /* create stream */
  socket=xstream_create(hostname,port);
  if(socket<0){
    ERROR("dispatcher: unable to create stream on %s:%s",hostname,port);
  }
  else{
    VERBOSE("dispatcher: bridged stream created on %s:%s (fd is %d)",hostname,port,socket);
  
    if(xstream_listen(socket,engine->queue_size)){
      ERROR("dispatcher: unable to specify socket %d listening queue",socket);
    }
    else{
      VERBOSE("dispatcher: socket %d listening queue successfully specified",socket);
	
      while(!eof_main_loop_flag){

	incoming_socket=xstream_accept(socket);
	if(incoming_socket<0 && !(eof_main_loop_flag || print_stats_flag)){
	  ERROR("dispatcher: error while waiting for incoming socket");
	}
	else if(incoming_socket<0 && eof_main_loop_flag){
	  VERBOSE("dispatcher: asked to no longer accept requests");
	}
	else if(incoming_socket<0 && print_stats_flag){
	  VERBOSE("dispatcher: %u connections dispatched",successful_dispatch);
	  xqueue_get_length(socket_queue,&queued_item_nb);
	  VERBOSE("dispatcher: %d connections pending",queued_item_nb);
	}
	else{

	  fstatus=xqueue_enqueue(socket_queue,&incoming_socket,sizeof(int));
	  if(fstatus){
	    ERROR("dispatcher: unable to add incoming connection (%d) to pending queue",incoming_socket);
	    xstream_close(incoming_socket);
	  }
	  else{
	    VERBOSE3("dispatcher: incoming connection (%d) successfully added to pending queue",incoming_socket);
	    successful_dispatch++;
	  }
	  
	}
	/*_*/ /* accept event */

      }
      /*_*/ /* accept loop */

      /* print stats on exit */
      VERBOSE("dispatcher: %u connections dispatched",successful_dispatch);
      fstatus=0;

    }
    /*_*/ /* socket listening */
      
    xstream_close(socket);
  }
  /*_*/ /* stream creation */
    

  return fstatus;
}


void * worker_main_function(void* p_args){

  int fstatus;

  bridged_worker_args_t* wargs;

  bridged_engine_t* engine;
  xqueue_t* squeue;
  bridge_rus_mgr_t* rmgr;

  int incoming_socket;
  int old_cancel_state;
  int old_cancel_state_bis;

  unsigned long items_nb;

  time_t start,end;

  wargs=(bridged_worker_args_t*)p_args;
  if(wargs!=NULL){

    engine=wargs->engine;
    squeue=wargs->socket_queue;
    rmgr=wargs->rus_mgr;

    /* the first worker is in charge of resource usage collection */
    if(wargs->id==0){

      while(!eof_worker_flag)
	{
	  time(&start);
	  fstatus=bridge_rus_mgr_synchronise(rmgr,&items_nb);
	  time(&end);
	  end=end-start;
	  if(fstatus){
	    ERROR("worker[%d] : unable to sync records (~%ds elapsed)",wargs->id,end);
	  }
	  else{
	    VERBOSE2("worker[%d] : sync done in ~%ds, %u records",wargs->id,end,items_nb);
	  }
	  sleep(rmgr->refresh_interval);
	}

    }
    /* the other workers are in charge of request processing */
    else{

      do{

	/* dequeue an incoming socket file descriptor */
	fstatus=xqueue_dequeue(squeue,&incoming_socket,sizeof(int));
	if(fstatus==0){
	  VERBOSE3("worker[%d] : incoming socket %d successfully dequeued",wargs->id,incoming_socket);
	  
	  /* disable cancellation and get the request */
	  fstatus=pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,&old_cancel_state);
	  if(fstatus){
	    ERROR2("worker[%d] : unable to disable cancellation");
	  }
	  else{
	    
	    /* process request */
	    fstatus=worker_process_request(p_args,incoming_socket);
	    if(fstatus){
	      ERROR2("worker[%d] : incoming socket %d processing failed",wargs->id,incoming_socket);
	    }
	    else{
	      VERBOSE3("worker[%d] : incoming socket %d processing succeed",wargs->id,incoming_socket);
	    }
	    
	    /* reenable cancellation */
	    fstatus=pthread_setcancelstate(old_cancel_state,&old_cancel_state_bis);
	    if(fstatus){
	      ERROR2("worker[%d] : unable to reenable old cancellation state ");
	  }
	    
	  }
	  /*_*/ /* disable cancellation */
	  
	  /* close incoming socket */
	  xstream_close(incoming_socket);
	}
	/*_*/ /* dequeue */
	
      }
      while(!eof_worker_flag);
      
      
    }
    
  }
  
}


/*!
 *
 *  -2 : memory allocation error
 *  -3 : pthread initialization problem
 *  -4 : socket queue init problem
 *  -5 : resource usage manager init failed
 */
int bridged_main_loop(bridged_engine_t* engine){

  int fstatus=-1;
  int status;

  int i;

  bridge_rus_mgr_t rus_mgr;

  xqueue_t socket_queue;

  int queue_size;
  int worker_nb;

  /**/
  int queued_item_nb;
  int launched_worker_nb=0;
  pthread_attr_t worker_attr;
  size_t worker_stacksize=PTHREAD_STACK_MIN;
  bridged_worker_args_t* worker_args;

  /* nullify message flags */
  eof_main_loop_flag=0;
  eof_worker_flag=0;

  /* initialize resource usage manager */
  fstatus=bridge_rus_mgr_init(&rus_mgr,engine->config_file);
  if(fstatus){
    ERROR("bridged: unable to initialize resource usage manager");
    fstatus=-5;
  }
  else{

    /* set queue size and worker quantity according to configuration */
    queue_size=engine->queue_size;
    worker_nb=engine->worker_nb;

    /* initialize socket queue */
    fstatus=xqueue_init(&socket_queue,queue_size,sizeof(int));
    if(fstatus){
      ERROR("bridged: unable to initialize workers socket queue");
      fstatus=-4;
    }
    else{
  
      /* initialize worker threads attributes */
      if(pthread_attr_init(&worker_attr)){
	ERROR("bridged: unable to initialize worker threads attributes");
	fstatus=-3;
      }
      else{
      
	/* set worker thread attributes */
	status=pthread_attr_setdetachstate(&worker_attr,PTHREAD_CREATE_JOINABLE);
	if(status){
	  ERROR("bridged: unable to set joinable detach state to worker threads attributes");
	}
	fstatus=status;
	status=pthread_attr_setstacksize(&worker_attr,worker_stacksize);
	if(status){
	  ERROR("bridged: unable to set worker threads stack size (%d) attribute",worker_stacksize);
	}
	fstatus+=status;
      
	/* continue if previous set succeed */
	if(fstatus){
	  fstatus=-3;
	}
	else{
	
	  /* initialize worker args array */
	  worker_args=(bridged_worker_args_t*)malloc(worker_nb*sizeof(bridged_worker_args_t));
	  if(worker_args==NULL){
	    ERROR("bridged: unable to allocate worker args array");
	    fstatus=-2;
	  }
	  else{
	    VERBOSE("bridged: worker args array successfully allocated");
	  
	    /* initialize and launch workers */
	    for(i=0;i<worker_nb;i++){
	      worker_args[i].id=i;
	      worker_args[i].engine=engine;
	      worker_args[i].socket_queue=&socket_queue;
	      worker_args[i].rus_mgr=&rus_mgr;
	      fstatus=pthread_create(&(worker_args[i].thread),
				     &worker_attr,
				     &worker_main_function,worker_args+i);
	      if(fstatus){
		ERROR2("bridged: unable to launch worker[%d]",i);
	      }
	      else{
		VERBOSE2("bridged: worker[%d] successfully launched",i);
		launched_worker_nb++;
	      }
	    }
	  
	    VERBOSE("bridged: %d/%d workers launched",worker_nb,launched_worker_nb);
	  
	    /* let the dispatcher takes over */
	    fstatus=dispatcher_main_function(engine,&socket_queue);
	  
	    /* look for pending connections */
	    if(xqueue_get_length(&socket_queue,&queued_item_nb)){
	      ERROR("bridged: unable to get pending connections number");
	    }
	    else{
	      VERBOSE("bridged: %d connections pending",queued_item_nb);
	      /* wait until no more pending connection is present */
	      if(queued_item_nb>0){
		VERBOSE("bridged: waiting for queue emptiness");
		xqueue_wait_4_emptiness(&socket_queue);
	      }
	    }
	    eof_worker_flag=1;

	    /* cancel worker threads */
	    VERBOSE("bridged: stopping workers");
	    for(i=0;i<launched_worker_nb;i++){
	      pthread_cancel(worker_args[i].thread);
	    }
	    /* join exited worker threads */
	    for(i=0;i<launched_worker_nb;i++){
	      pthread_join(worker_args[i].thread,NULL);
	    }

	    VERBOSE("bridged: %s",(reload_flag)?"reloading":"exiting");
	  
	    free(worker_args);
	  }
	  /*_*/ /* worker args array init */

	}
	/*_*/ /* thread attributes set up */
    
	/* destroy worker threads attributes */
	pthread_attr_destroy(&worker_attr);
      }

      /* destroy socket queue */  
      xqueue_free_contents(&socket_queue);
    }


    bridge_rus_mgr_free_contents(&rus_mgr);
  }
  
  return fstatus;
}



int main(int argc,char** argv){

  int fstatus=-1;

  int i;
  int background_flag=0;
  int foreground_flag=0;

  int debug_level=0;
  int verbose_level=0;
  char* conf_file_string=NULL;
  char* working_directory;

  /* options processing variables */
  char* progname;
  char* optstring="dvhf:F";
  char* short_options_desc="\nUsage : %s [-h] [-dv] [-F] [-f conffile]\n\n";
  char* addon_options_desc="\
\t-h\t\tshow this message\n\
\t-d\t\tincrease debug level\n\
\t-v\t\tincrease verbose level\n\
\t-F\t\trun in foreground\n \
\t-f conffile\tConfiguration file\n\n";
  int   option;
  
  /* signal handling variables */
  struct sigaction saction;

  /* bridged engine */
  bridged_engine_t engine;

  /* logging */
  FILE* logfile=NULL;
  FILE* debugfile=NULL;

  /* get current program name */
  progname=rindex(argv[0],'/');
  if(progname==NULL)
    progname=argv[0];
  else
    progname++;
  
  /* process options */
  while((option = getopt(argc,argv,optstring)) != -1)
    {
      switch(option)
	{
	case 'v' :
	  verbose_level++;
	  break;
	case 'd' :
	  debug_level++;
	  break;
	case 'f' :
	  conf_file_string=strdup(optarg);
	  break;
    case 'F' :
      foreground_flag = 1;
      break;
	case 'h' :
	default :
	  fprintf(stdout,short_options_desc,progname);
	  fprintf(stdout,"%s\n",addon_options_desc);
	  exit(0);
	  break;
	}
    }

  /* set verbosity and debug level */
  xdebug_setmaxlevel(debug_level);
  xverbose_setmaxlevel(verbose_level);
  
  /* set signal handlers (SIGTERM/SIGHUP/SIGCHLD) */
  saction.sa_handler=signal_handler;
  sigemptyset(&(saction.sa_mask));
  saction.sa_flags=0;
  if(sigaction(SIGTERM,&saction,NULL))
    {
      ERROR("SIGTERM handler set up failed");
      exit_flag=1;
    }
  if(sigaction(SIGHUP,&saction,NULL))
    {
      ERROR("SIGHUP handler set up failed");
      exit_flag=1;
    }
  if(sigaction(SIGUSR2,&saction,NULL))
    {
      ERROR("SIGUSR2 handler set up failed");
      exit_flag=1;
    }
  saction.sa_handler=SIG_IGN;
  if(sigaction(SIGCHLD,&saction,NULL))
    {
      ERROR("SIGCHLD handler set up failed");
      exit_flag=1;
    }

  /* work if no problem during sig handler set up */
  while(!exit_flag)
    {
      reload_flag=0;
    
      /* load configuration */
      fstatus=bridged_engine_init_from_config_file(&engine,conf_file_string);
      if(fstatus)
	{  
	  ERROR("bridged: unable to load configuration, exiting");
	  /* error while loading conf, exit */
	  exit_flag=1;
	}
      else
	{

	  /* no display required, jump into background */
	  if(!verbose_level && !debug_level)
	    {
	      /* go to background mode if not already done */
	      if(!background_flag && !foreground_flag)
		{
		  /* fork, father goes away */
		  if(fork() != 0)
		    exit(EXIT_SUCCESS);
		  
		  /* go into working directory */
		  if(strlen(engine.cachedir)>0)
		    {
		      working_directory=engine.cachedir;
		    }
		  else
		    working_directory="/";
		  chdir(working_directory);
		  VERBOSE("working directory is now %s",working_directory);
		  
		  /* change session ID, fork and keep only son */
		  setsid();
		  if(fork() != 0)
		    exit(EXIT_SUCCESS);
		  
		  /* close all open file descriptor */
		  for(i=0;i<FOPEN_MAX;i++)
		    close(i);

		  /* set background flag in order to do all this switch just once */
		  background_flag=1;
		  
		}


        /* in bg mode : (re)open logfile and debug file
         * fg mode : set log level according to conf file */
        if (!foreground_flag) {
	      if(logfile!=NULL){
		    fclose(logfile);
	    	logfile=NULL;
	      }
	      if(debugfile!=NULL){
		    fclose(debugfile);
		    debugfile=NULL;
	      }
	      if((strlen(engine.logfile)>0) && engine.loglevel && (logfile=fopen(engine.logfile,"a+"))){
		    xverbose_setstream(logfile);
		    xerror_setstream(logfile);
		    xverbose_setmaxlevel(engine.loglevel);
		    xerror_setmaxlevel(engine.loglevel);
	      }
	      else{
		    xverbose_setmaxlevel(0);
		    xerror_setmaxlevel(0);
	      }
	      if((strlen(engine.debugfile)>0) && engine.debuglevel && (debugfile=fopen(engine.debugfile,"a+"))){
		    xdebug_setstream(debugfile);
		    xdebug_setmaxlevel(engine.debuglevel);
	      }
	      else{
		    xdebug_setmaxlevel(0);
	      }
	    }
	    else {
        /* vprintf segv with stderr and multithread so use stdout*/
           FILE* stream=stdout;
           xverbose_setstream(stream);
           xerror_setstream(stream);
           xdebug_setstream(stream);
           xverbose_setmaxlevel(engine.loglevel);
           xerror_setmaxlevel(engine.loglevel);
           xdebug_setmaxlevel(engine.debuglevel);
        }
      } 

	  /* start workers */
	  
	  /* launch main function */
	  if ( bridged_main_loop(&engine) != 0 ) {

	    ERROR("bridged: last main loop ends with error, exiting");
	    exit_flag = 1 ;

	  }

	  /*_*/

	  /* stop workers */
	  
	  /* free engine contents */
	  bridged_engine_free_contents(&engine);
	  /*_*/

	}
      /*_*/
      
    }
  /*_*/
  
  /* free config file */
  XFREE(conf_file_string);
  
  return fstatus;
}
