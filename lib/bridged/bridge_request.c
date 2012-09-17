/*****************************************************************************\
 *  lib/bridged/bridge_request.c - 
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

/* for marshalling */
#include <rpc/types.h>
#include <rpc/xdr.h>

/* logging */
#include "xternal/xlogger.h"

#ifndef BRIDGE_REQUEST_LOGHEADER
#define BRIDGE_REQUEST_LOGHEADER "bridge_request: "
#endif

#ifndef BRIDGE_REQUEST_VERBOSE_BASE_LEVEL
#define BRIDGE_REQUEST_VERBOSE_BASE_LEVEL 5
#endif

#ifndef BRIDGE_REQUEST_DEBUG_BASE_LEVEL
#define BRIDGE_REQUEST_DEBUG_BASE_LEVEL   5
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

/* main header */
#include "bridged/bridge_request.h"

int
bridge_get_req_init(bridge_get_req_t* bgr,char* batchid,char* rmid){
  
  int fstatus=0;

  bgr->batchid=NULL;
  bgr->rmid=NULL;

  if(batchid!=NULL){
    bgr->batchid=strdup(batchid);
    if(bgr->batchid==NULL){
      fstatus=-1;
    }
  }

  if(fstatus==0 && rmid!=NULL){
    bgr->rmid=strdup(rmid);
    if(bgr->rmid==NULL){
      fstatus=-2; 
    }
  }

  return fstatus;
}

int
bridge_get_req_free_contents(bridge_get_req_t* bgr){

  int fstatus=0;
  
  if(bgr->batchid!=NULL){
    free(bgr->batchid);
    bgr->batchid=NULL;
  }
  if(bgr->rmid!=NULL){
    free(bgr->rmid);
    bgr->rmid=NULL;
  }
  
  return fstatus;
}

int
bridge_get_req_init_from_message(bridge_get_req_t* bgr,xmessage_t* msg){

  int fstatus=0;

  size_t batchid_length;
  size_t rmid_length;
  XDR xdr;

  bgr->batchid=NULL;
  bgr->rmid=NULL;

  /* msg type validity check */
  if(msg->type!=XGET_REQUEST)
    fstatus=-1;
  else{

    /* msg data validity check */
    if(msg->data==NULL || msg->length==0)
      fstatus=-2;
    else{
      
      xdrmem_create(&xdr,msg->data,msg->length,XDR_DECODE);

      /* ids lengths extraction */
      if(!xdr_u_long(&xdr,(unsigned long*)&(batchid_length))
	 || !xdr_u_long(&xdr,(unsigned long*)&(rmid_length))){
	fstatus=-3;
      }
      else{

	/* ids values extraction */
	fstatus=0;
	if(batchid_length>0){
	  bgr->batchid=(char*)malloc((batchid_length+1)*sizeof(char));
	  if(bgr->batchid==NULL){
	    fstatus=-4;
	  }
	  else{

	    if(!xdr_opaque(&xdr,(char*)bgr->batchid,(u_int)batchid_length)){
	      fstatus=-5;
	    }
	    else{
	      bgr->batchid[batchid_length]='\0';
	      fstatus=0;
	    }

	  }
	}
	if(fstatus==0 && rmid_length>0){
	  bgr->rmid=(char*)malloc((rmid_length+1)*sizeof(char));
	  if(bgr->rmid==NULL){
	    fstatus=-6;
	  }
	  else{

	    if(!xdr_opaque(&xdr,(char*)bgr->rmid,(u_int)rmid_length)){
	      fstatus=-7;
	    }
	    else{
	      bgr->rmid[rmid_length]='\0';
	      fstatus=0;
	    }

	  }
	}
	
      }
      /*_*/ /* ids lengths extraction */

      xdr_destroy(&xdr);

      /* clean structure if a problem occured */
      if(fstatus!=0){
	bridge_get_req_free_contents(bgr);
      }

    }
    /*_*/ /* msg data validity check */

  }
  /*_*/ /* msg type validity check */

  return fstatus;
}



int
bridge_get_req_create_message(bridge_get_req_t* bgr,xmessage_t* msg){

  int fstatus=0;

  size_t batchid_length;
  size_t rmid_length;

  char* buffer;
  size_t length;

  XDR xdr;

  /* get batchid length */
  if(bgr->batchid==NULL)
    batchid_length=0;
  else
    batchid_length=strlen(bgr->batchid);

  /* get rmid length */
  if(bgr->rmid==NULL)
    rmid_length=0;
  else
    rmid_length=strlen(bgr->rmid);

  /* allocate buffer */
  length=2*sizeof(long)+batchid_length+rmid_length;
  buffer=(char*)malloc(length*sizeof(char));
  if(buffer==NULL){
    fstatus=-1;
  }
  else{

    /* serialize data */
    xdrmem_create(&xdr,buffer,length,XDR_ENCODE);
    if(!xdr_u_long(&xdr,(unsigned long*)&(batchid_length))){
      ERROR("unable to serialize batchid length");
      fstatus=-2;
    }
    else if(!xdr_u_long(&xdr,(unsigned long*)&(rmid_length))){
      ERROR("unable to serialize rmid length");
      fstatus=-3;
    }
    else{
      fstatus=0;

      if(batchid_length>0){
	if(!xdr_opaque(&xdr,(char*)bgr->batchid,(u_int)batchid_length)){
	  ERROR("unable to serialize batchid '%s'",bgr->batchid);
	  fstatus=-4;
	}
      }

      if(fstatus==0 && rmid_length>0){
	if(!xdr_opaque(&xdr,(char*)bgr->rmid,(u_int)rmid_length)){
	  ERROR("unable to serialize rmid '%s'",bgr->rmid);
	  fstatus=-5;
	}
      }

      /* if ok, build the message */
      if(fstatus==0){
	fstatus=xmessage_init(msg,XGET_REQUEST,buffer,length);
      }

    }

    xdr_destroy(&xdr);
    free(buffer);
  }

  return fstatus;
}



/********************************************************************************\
 * Bridge get reply related functions
\********************************************************************************/
int
bridge_get_rep_init(bridge_get_rep_t* bgr,time_t used_time,time_t usable_time,time_t halt_time){  
  int fstatus=0;

  bgr->used_time=used_time;
  bgr->usable_time=usable_time;
  bgr->halt_time=halt_time;

  return fstatus;
}


int
bridge_get_rep_free_contents(bridge_get_rep_t* bgr){
  int fstatus=0;

  bgr->used_time=0;
  bgr->usable_time=0;
  bgr->halt_time=0;  
  
  return fstatus;
}


int
bridge_get_rep_init_from_message(bridge_get_rep_t* bgr,xmessage_t* msg){
  int fstatus=-1;

  XDR xdr;

  /* msg type validity check */
  if(msg->type!=XGET_REPLY){
    ERROR("bad input message type");
    fstatus=-1;
  }
  else{

    /* msg data validity check */
    if(msg->data==NULL || msg->length==0){
      ERROR("input message doesn't contain data");
      fstatus=-2;
    }
    else{
      
      xdrmem_create(&xdr,msg->data,msg->length,XDR_DECODE);

      if(!xdr_u_long(&xdr,(unsigned long*)&(bgr->used_time))){
	ERROR("unable to unmarshall used time");
	fstatus=-3;
      }
      else if(!xdr_u_long(&xdr,(unsigned long*)&(bgr->usable_time))){
	ERROR("unable to unmarshall usable time");
	fstatus=-4;
      }
      else if(!xdr_u_long(&xdr,(unsigned long*)&(bgr->halt_time))){
	ERROR("unable to unmarshall halt time");
	fstatus=-5;
      }
      else {
	fstatus=0;
      }

      xdr_destroy(&xdr);
    }

  }

  return fstatus;
}


int
bridge_get_rep_create_message(bridge_get_rep_t* bgr,xmessage_t* msg){
  int fstatus=-1;

  char* buffer;
  size_t length;

  XDR xdr;

  /* allocate buffer */
  length=3*sizeof(unsigned long);
  buffer=(char*)malloc(length*sizeof(char));
  if(buffer==NULL){
    ERROR("unable to allocate buffer for get reply marshalling");
    fstatus=-2;
  }
  else{

    VERBOSE("bridge get reply is used=%u usable=%u halt=%u",
	    bgr->used_time,bgr->usable_time,bgr->halt_time);

    xdrmem_create(&xdr,buffer,length,XDR_ENCODE);

    if(!xdr_u_long(&xdr,&(bgr->used_time))){
      ERROR("unable to marshall used time");
      fstatus=-3;
    }
    else if(!xdr_u_long(&xdr,(unsigned long*)&(bgr->usable_time))){
      ERROR("unable to marshall usable time");
      fstatus=-4;
    }
    else if(!xdr_u_long(&xdr,(unsigned long*)&(bgr->halt_time))){
      ERROR("unable to marshall halt time");
      fstatus=-5;
    }
    else {
      fstatus=0;
    }
    
    xdr_destroy(&xdr);

    /* if ok, build the message */
    if(fstatus==0){
      fstatus=xmessage_init(msg,XGET_REPLY,buffer,length);
    }

    free(buffer);
  }

  return fstatus;
}
