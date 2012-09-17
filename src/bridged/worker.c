/*****************************************************************************\
 *  src/bridged/worker.c - 
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

#include "bridged/bridge_request.h"

#include "bridged_engine.h"
#include "worker.h"

#define VERBOSE xverbose
#define VERBOSE2 xverbose2
#define VERBOSE3 xverbose3
#define ERROR VERBOSE
#define ERROR2 VERBOSE
#define ERROR3 VERBOSE

#define XFREE(a) if( a != NULL) { free(a); a=NULL;};


int worker_send_error_reply(void* p_args,int stream,char* error_message){
  int fstatus=-1;

  bridged_worker_args_t* wargs;
  xmessage_t rep;

  char* buffer;
  size_t length;

  wargs=(bridged_worker_args_t*)p_args;
  if(wargs==NULL){
    return -1;
  }

  /* initialize ping reply message */
  fstatus=xmessage_init(&rep,XERROR_REPLY,error_message,strlen(error_message));
  if(fstatus){
    ERROR3("worker[%d] : unable to initialize reply reply message",wargs->id);
  }
  else{
    VERBOSE3("worker[%d] : error reply message successfully initialized",wargs->id);

    /* marshall ping reply message */
    fstatus=xmessage_marshall(&rep,&buffer,&length);
    if(fstatus){
      ERROR3("worker[%d] : unable to marshall error reply message",wargs->id);
    }
    else{
      VERBOSE3("worker[%d] : error reply message successfully marshalled",wargs->id);
      
      /* send marshalled ping reply message */
      fstatus=xstream_send_msg_timeout(stream,buffer,length,wargs->engine->timeout);
      if(fstatus){
	ERROR3("worker[%d] : unable to send error reply marshalled message",wargs->id);
      }
      else{
	VERBOSE3("worker[%d] : error reply marshalled message successfully sended",wargs->id);
	fstatus=0;
      }
      
      /* free marshalled reply message */
      free(buffer);
    }
    
    /* free reply bridge message */
    xmessage_free_contents(&rep);
  } 
  
  return fstatus;
}

int worker_process_ping_request(void* p_args,int stream,xmessage_t* req){
  int fstatus=-1;

  bridged_worker_args_t* wargs;
  bridged_engine_t* engine;

  xmessage_t rep;

  char* buffer;
  size_t length;

  wargs=(bridged_worker_args_t*)p_args;
  if(wargs==NULL){
    return -1;
  }
  
  engine=wargs->engine;

  /* initialize ping reply message */
  fstatus=xmessage_init(&rep,XPING_REPLY,NULL,0);
  if(fstatus){
    ERROR3("worker[%d] : unable to initialize ping reply message",wargs->id);
  }
  else{
    VERBOSE3("worker[%d] : ping reply message successfully initialized",wargs->id);

    /* marshall ping reply message */
    fstatus=xmessage_marshall(&rep,&buffer,&length);
    if(fstatus){
      ERROR3("worker[%d] : unable to marshall ping reply message",wargs->id);
    }
    else{
      VERBOSE3("worker[%d] : ping reply message successfully marshalled",wargs->id);

      /* send marshalled ping reply message */
      fstatus=xstream_send_msg_timeout(stream,buffer,length,wargs->engine->timeout);
      if(fstatus){
	ERROR3("worker[%d] : unable to send ping reply marshalled message",wargs->id);
      }
      else{
	VERBOSE3("worker[%d] : ping reply marshalled message successfully sended",wargs->id);
	fstatus=0;
      }

      /* free marshalled reply message */
      free(buffer);
    }
    
    /* free reply bridge message */
    xmessage_free_contents(&rep);
  }

  /* for logging */
  if(fstatus==0)
    VERBOSE3("worker[%d] : ping request succeeds",wargs->id);
  else
    VERBOSE3("worker[%d] : ping request fails",wargs->id);

  return fstatus;
}

int worker_process_get_request(void* p_args,int stream,xmessage_t* req){
  int fstatus=-1;

  bridged_worker_args_t* wargs;
  bridged_engine_t* engine;

  char* error_message;
  char* generic_error_message="generic error";

  bridge_rus_record_t record;
  bridge_rus_mgr_t* rmgr;

  bridge_get_req_t bgreq;
  bridge_get_rep_t bgrep;

  xmessage_t rep;

  char* buffer;
  size_t length;

  int success_flag=0;

  time_t used_time;
  time_t usable_time;
  time_t halt_time;

  error_message=generic_error_message;

  wargs=(bridged_worker_args_t*)p_args;
  if(wargs==NULL){
    return -1;
  }
  
  engine=wargs->engine;
  rmgr=wargs->rus_mgr;

  /* get request initialization */
  fstatus=bridge_get_req_init_from_message(&bgreq,req);
  if(fstatus){
    fstatus=worker_send_error_reply(p_args,stream,error_message);
    VERBOSE3("worker[%d] : get request fails : %s",wargs->id,error_message);
  }
  else{
    VERBOSE3("worker[%d] : get request params are batchid:%s rmid:%s",wargs->id,
	     (bgreq.batchid==NULL)?"-":bgreq.batchid,
	     (bgreq.rmid==NULL)?"-":bgreq.rmid);

    /* bridge rus record init */
    fstatus=bridge_rus_record_init(&record,(bgreq.batchid==NULL)?"-":bgreq.batchid);
    if(fstatus){
      fstatus=worker_send_error_reply(p_args,stream,"rus record init failed");
      VERBOSE3("worker[%d] : rus record init fails",wargs->id);
    }
    else{

      fstatus=bridge_rus_mgr_get_record(rmgr,&record);
      if(fstatus){
	VERBOSE3("worker[%d] : rus record '%s' not found",wargs->id,record.id);
	worker_send_error_reply(p_args,stream,"corresponding rus record not found");
	fstatus=-1;
      }
      else{
	
	used_time=record.used_time;
	usable_time=record.usable_time;
	halt_time=0;

	/* build reply and transmit it */
	fstatus=bridge_get_rep_init(&bgrep,used_time,usable_time,halt_time);
	if(fstatus){
	  ERROR3("worker[%d] : unable to initialize bridge get reply",wargs->id);
	}
	else{
	  VERBOSE3("worker[%d] : bridge get reply successfully initialized",wargs->id);

	  VERBOSE3("worker[%d] : bridge get reply is used=%u usable=%u halt=%u",wargs->id,
		   bgrep.used_time,bgrep.usable_time,bgrep.halt_time);

	  fstatus=bridge_get_rep_create_message(&bgrep,&rep);
	  if(fstatus){
	    VERBOSE3("worker[%d] : unable to create reply message",wargs->id);
	  }
	  else{
	    VERBOSE3("worker[%d] : reply message successfully created",wargs->id);
	
	    /* marshall get reply message */
	    fstatus=xmessage_marshall(&rep,&buffer,&length);
	    if(fstatus){
	      VERBOSE3("worker[%d] : unable to marshall get reply message",wargs->id);
	    }
	    else{
	      VERBOSE3("worker[%d] : get reply message successfully marshalled",wargs->id);
	  
	      /* send marshalled get reply message */
	      fstatus=xstream_send_msg_timeout(stream,buffer,length,wargs->engine->timeout);
	      if(fstatus){
		VERBOSE3("worker[%d] : unable to send get reply marshalled message",wargs->id);
	      }
	      else{
		VERBOSE3("worker[%d] : get reply marshalled message successfully sended",wargs->id);
		fstatus=0;
	      }
	  
	      /* free marshalled reply message */
	      free(buffer);
	    }
	
	    /* free reply bridge message */
	    xmessage_free_contents(&rep);
	  }
      
	  bridge_get_rep_free_contents(&bgrep);
	}

      }

      bridge_rus_record_free_contents(&record);
    }
    /*_*/ /* bridge rus record init */

    if(fstatus)
      VERBOSE2("worker[%d] : get from batchid=%s rmid=%s fails",wargs->id,
	       (bgreq.batchid==NULL)?"-":bgreq.batchid,
	       (bgreq.rmid==NULL)?"-":bgreq.rmid);
    else
      VERBOSE2("worker[%d] : get from batchid=%s rmid=%s succeeds (used=%u usable=%u halt=%u)",wargs->id,
	       (bgreq.batchid==NULL)?"-":bgreq.batchid,
	       (bgreq.rmid==NULL)?"-":bgreq.rmid,
	       used_time,usable_time,halt_time);
    
    bridge_get_req_free_contents(&bgreq);
  }
  /*_*/ /* get request initialization */

  return fstatus;
}


int worker_process_request(void* p_args,int socket){
  int fstatus=-1;

  bridged_worker_args_t* wargs;
  bridged_engine_t* engine;

  char* buffer;
  size_t length;

  xmessage_t req;

  int rstatus=0;
  char ack;
  
  wargs=(bridged_worker_args_t*)p_args;
  if(wargs==NULL){
    return -1;
  }
  
  engine=wargs->engine;

  VERBOSE3("worker[%d] : handling connection on socket %d",wargs->id,socket);

  while(rstatus==0){

    /* receive request */
    if(xstream_receive_msg_timeout(socket,&buffer,&length,engine->timeout)){
      ERROR2("worker[%d] : marshalled request reception failed",wargs->id);
      rstatus=10;
    }
    else{
      VERBOSE3("worker[%d] : marshalled request reception succeed",wargs->id);
      
      /* unmarshall request */
      fstatus=xmessage_unmarshall(&req,buffer,length);
      if(fstatus){
	ERROR2("worker[%d] : unable to unmarshall received marshalled request",wargs->id);
	rstatus=20;
      }
      else{
	
	/* process request */
	switch(req.type){
	  
	case XPING_REQUEST :
	  VERBOSE3("worker[%d] : ping request received",wargs->id);
	  fstatus=worker_process_ping_request(p_args,socket,&req);
	  break;

	case XGET_REQUEST :
	  VERBOSE3("worker[%d] : get request received",wargs->id);
	  fstatus=worker_process_get_request(p_args,socket,&req);
	  break;
	  
	case XEND_REQUEST :
	  VERBOSE3("worker[%d] : end request received",wargs->id);
	  rstatus=1;
	  break;

	default :
	  VERBOSE3("worker[%d] : invalid request received",wargs->id);
	  rstatus=30;
	  break;
	  
	}
	
	/* free bridge request */
	xmessage_free_contents(&req);
      }
      /*_*/ /* unmarshall received msg */
      
      /* free received msg */
      free(buffer);
      buffer=NULL;
    }
    /*_*/ /* receive request */

  }
  
  return fstatus;
}
