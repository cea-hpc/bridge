/*****************************************************************************\
 *  lib/bridged/bridgedapi.c - 
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

/* logging */
#include "xternal/xlogger.h"

#ifndef BRIDGE_REQUEST_LOGHEADER
#define BRIDGE_REQUEST_LOGHEADER "bridge_api: "
#endif

#ifndef BRIDGE_REQUEST_VERBOSE_BASE_LEVEL
#define BRIDGE_REQUEST_VERBOSE_BASE_LEVEL 1
#endif

#ifndef BRIDGE_REQUEST_DEBUG_BASE_LEVEL
#define BRIDGE_REQUEST_DEBUG_BASE_LEVEL   1
#endif

#define VERBOSE(h,a...) xverboseN(BRIDGE_REQUEST_VERBOSE_BASE_LEVEL,BRIDGE_REQUEST_LOGHEADER h,##a)
#define VERBOSE2(h,a...) xverboseN(BRIDGE_REQUEST_VERBOSE_BASE_LEVEL + 1,BRIDGE_REQUEST_LOGHEADER h,##a)
#define VERBOSE3(h,a...) xverboseN(BRIDGE_REQUEST_VERBOSE_BASE_LEVEL + 2,BRIDGE_REQUEST_LOGHEADER h,##a)

#define DEBUG(h,a...) xdebugN(BRIDGE_REQUEST_DEBUG_BASE_LEVEL,BRIDGE_REQUEST_LOGHEADER h,##a)
#define DEBUG2(h,a...) xdebugN(BRIDGE_REQUEST_DEBUG_BASE_LEVEL + 1,BRIDGE_REQUEST_LOGHEADER h,##a)
#define DEBUG3(h,a...) xdebugN(BRIDGE_REQUEST_DEBUG_BASE_LEVEL + 2,BRIDGE_REQUEST_LOGHEADER h,##a)

#define ERROR VERBOSE

#define INIT_DEBUG_MARK()    DEBUG("%s : entering",function_name)
#define EXIT_DEBUG_MARK(a)   DEBUG("%s : exiting with status %d",function_name,a)

#define INIT_DEBUG2_MARK()   DEBUG2("%s : entering",function_name)
#define EXIT_DEBUG2_MARK(a)  DEBUG2("%s : exiting with status %d",function_name,a)

#define INIT_DEBUG3_MARK()   DEBUG3("%s : entering",function_name)
#define EXIT_DEBUG3_MARK(a)  DEBUG3("%s : exiting with status %d",function_name,a)

#include "xternal/xstream.h"
#include "xternal/xmessage.h"

#include "bridged/bridgedapi.h"
#include "bridged/bridge_request.h"
#include "bridged/bridge_engine.h"

int
bridge_api_process_request(char* conf_file,xmessage_t* req,xmessage_t* rep){
  
  int fstatus=XERROR;
  
  int i;
  
  bridge_engine_t engine;

  /* bridged server options */
  char* server;
  char* port;
  int retry;
  int max_retries=3;
  time_t timeout=15;
  
  /* stream */
  int stream;

  /* request data */
  char* snd_buffer;
  size_t snd_length;
  char* rcv_buffer;
  size_t rcv_length;

  int rstatus = BRIDGE_ERROR;

  xmessage_t ack;
  char* ack_buffer;
  size_t ack_length;

  /* init engine from configuration file */
  fstatus=bridge_engine_init_from_config_file(&engine,conf_file);
  if(fstatus){
    ERROR("unable to init engine");
    fstatus=BRIDGE_ERROR_ENGINE_INIT_FAILED;
  }
  else{
    VERBOSE("engine successfully initialized");
  
    /* Marshall request */
    fstatus=xmessage_marshall(req,&snd_buffer,&snd_length);
    if(fstatus){
      ERROR("unable to marshall request");
    }
    else{

      retry=1;
    
      /* loop while retries are authorized */
      while(retry<=max_retries){

	VERBOSE("starting retry %d of %d",retry,max_retries);

	/* loop on primary and secondary */
	for(i=1;i<=2;i++){

	  /* set connection options */
	  if(i%2==1){
	    server=engine.primary_address;
	    port=engine.primary_port;
	    timeout=engine.timeout;
	  }
	  else{
	    server=engine.secondary_address;
	    port=engine.secondary_port;
	    timeout=engine.timeout;
	  }
	
	  /* try to connect primary server */
	  fstatus=BRIDGE_ERROR;
	  stream=xstream_connect(server,port,timeout);
	  if(stream<0){
	    ERROR("unable to connect to server %s:%s",server,port);
	    fstatus=BRIDGE_ERROR_STREAM_CONNECT_FAILED;
	  }
	  else{
	    VERBOSE("successfully connected to server %s:%s",server,port);

	    /* marshalled request transmission */
	    rstatus=xstream_send_msg_timeout(stream,snd_buffer,snd_length,engine.timeout);
	    if(rstatus){
	      ERROR("unable to send marshalled request");
	      fstatus=BRIDGE_ERROR_STREAM_TRANS_FAILED;
	    }
	    else{
	      VERBOSE("marshalled request successfully send");

	      /* reply reception */
	      rstatus=xstream_receive_msg_timeout(stream,&rcv_buffer,&rcv_length,engine.timeout);
	      if(rstatus){
		ERROR("unable to receive marshalled reply");
		fstatus=BRIDGE_ERROR_STREAM_TRANS_FAILED;
	      }
	      else{
		VERBOSE("marshalled reply successfully received");
	    
		rstatus=xmessage_unmarshall(rep,rcv_buffer,rcv_length);
		if(rstatus){
		  ERROR("unable to unmarshall reply");
		}
		else{
		  VERBOSE("reply successfully unmarshalled");
		  fstatus=BRIDGE_SUCCESS;
		}
	    
		/* free marshalled reply */
		free(rcv_buffer);
	      }
	      /*_*/ /* reply reception */
	  
	    }
	    /*_*/ /* marshalled request transmission */

	    /* init ack message */
	    rstatus=xmessage_init(&ack,XEND_REQUEST,NULL,0);
	    if(rstatus){
	      ERROR("unable to initialize end request message");
	    }
	    else{
	      VERBOSE("end request message successfully initialized");
	      
	      /* marshall ack message */
	      rstatus=xmessage_marshall(&ack,&ack_buffer,&ack_length);
	      if(rstatus){
		ERROR("unable to marshall end request");
	      }
	      else{
		VERBOSE("end request successfully marshalled");
		
		/* send ack message */
		rstatus=xstream_send_msg_timeout(stream,ack_buffer,ack_length,engine.timeout);
		if(rstatus){
		  ERROR("unable to send marshalled end request");
		}
		else{
		  VERBOSE("marshalled end request successfully send");
		}
		/*_*/ /* send ack message */
		
		free(ack_buffer);
	      }
	      /*_*/ /* marshall ack message */
	      
	      xmessage_free_contents(&ack);
	    }
	    /*_*/ /* init ack message */
	    
	    /* close stream */
	    xstream_close(stream);
	
	    /* break the loop */
	    break;

	  }
	  /*_*/ /* connect */
      
	}
	/*_*/ /* for */
    
	if(fstatus==0)
	  break;
    
	/* incremente retry */
	retry++;
    
      }
      /*_*/ /* while */
  
      free(snd_buffer);
    }
    /*_*/ /* request marshalling */

    bridge_engine_free_contents(&engine);
  }
  /*_*/ /* engine initialization */
  
  /* if fstatus equals 0, return request status (rstatus) */
  if(fstatus==0)
    return rstatus;
  else
    return fstatus;
}

int
bridgedapi_ping(char* conf_file){
  int fstatus;
  
  xmessage_t req;
  xmessage_t rep;

  /* initialize ping message */
  fstatus=xmessage_init(&req,XPING_REQUEST,NULL,0);
  if(fstatus){
    ERROR("unable to initialize ping request message");
  }
  else{
    VERBOSE("ping request message successfully initialized");
    fstatus=bridge_api_process_request(conf_file,&req,&rep);
    if(fstatus!=BRIDGE_SUCCESS){
      ERROR("an error occurred while doing ping");
    }
    else{
      fstatus=BRIDGE_ERROR;
      switch(rep.type){
	
      case XPING_REPLY :
	VERBOSE("ping successfully done");
	fstatus=0;
	break;
	
      default :
	VERBOSE("invalid ping reply message : reply type is %d",rep.type);
	break;
	
      }

      xmessage_free_contents(&rep);
    }
    
    xmessage_free_contents(&req);
  }
  
  return fstatus;
}


int
bridgedapi_get(char* conf_file,char* batchid,char* rmid,
	       time_t* usable,time_t* used,time_t* halt){
  int fstatus=BRIDGE_ERROR;

  bridge_get_req_t bgreq;
  bridge_get_rep_t bgrep;

  xmessage_t req;
  xmessage_t rep;

  /* initialize get request */
  fstatus=bridge_get_req_init(&bgreq,batchid,rmid);
  if(fstatus){
    ERROR("unable to initialize get request");
  }
  else{

    /* create a message from get request */
    fstatus=bridge_get_req_create_message(&bgreq,&req);
    if(fstatus){
      ERROR("unable to create get request message");
    }
    else{
      VERBOSE("get request message successfully created");

      fstatus=bridge_api_process_request(conf_file,&req,&rep);
      if(fstatus){
	ERROR("an error occurred while processing request");
      }
      else{
	fstatus=BRIDGE_ERROR;
	switch(rep.type){

	case XERROR_REPLY :
	  VERBOSE("get request fails");
	  fstatus=BRIDGE_ERROR_REQUEST_FAILED;
	  break;

	case XGET_REPLY :
	  fstatus=bridge_get_rep_init_from_message(&bgrep,&rep);
	  if(fstatus){
	    ERROR("unable to init bridge reply based on received data");
	    fstatus=BRIDGE_ERROR_REQUEST_MALFORMED_REPLY;
	  }
	  else{
	    VERBOSE("reply is used=%u usable=%u halt=%u",
		    bgrep.used_time,bgrep.usable_time,bgrep.halt_time);
	    *usable=bgrep.usable_time;
	    *used=bgrep.used_time;
	    *halt=bgrep.halt_time;
	    bridge_get_rep_free_contents(&bgrep);
	    fstatus=BRIDGE_SUCCESS;
	  }
	  break;
	  
	default :
	  VERBOSE("invalid get reply message : reply type is %d",rep.type);
	  break;
	  
	}
	
	xmessage_free_contents(&rep);
      }
      

      xmessage_free_contents(&req);
    }

    bridge_get_req_free_contents(&bgreq);
  }
  
  return fstatus;
}
