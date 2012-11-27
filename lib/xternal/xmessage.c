/*****************************************************************************\
 *  lib/xternal/xmessage.c - 
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

/* for marshalling */
#include <rpc/types.h>
#include <rpc/xdr.h>

#include <string.h>

#include <errno.h>
extern int errno;

#include "xerror.h"

/* logging */
#include "xlogger.h"

#ifndef XMESSAGE_LOGHEADER
#define XMESSAGE_LOGHEADER "xmessage: "
#endif

#ifndef XMESSAGE_VERBOSE_BASE_LEVEL
#define XMESSAGE_VERBOSE_BASE_LEVEL 7
#endif

#ifndef XMESSAGE_DEBUG_BASE_LEVEL
#define XMESSAGE_DEBUG_BASE_LEVEL   7
#endif

#define VERBOSE(h,a...) xverboseN(XMESSAGE_VERBOSE_BASE_LEVEL,XMESSAGE_LOGHEADER h,##a)
#define VERBOSE2(h,a...) xverboseN(XMESSAGE_VERBOSE_BASE_LEVEL + 1,XMESSAGE_LOGHEADER h,##a)
#define VERBOSE3(h,a...) xverboseN(XMESSAGE_VERBOSE_BASE_LEVEL + 2,XMESSAGE_LOGHEADER h,##a)

#define DEBUG(h,a...) xdebugN(XMESSAGE_DEBUG_BASE_LEVEL,XMESSAGE_LOGHEADER h,##a)
#define DEBUG2(h,a...) xdebugN(XMESSAGE_DEBUG_BASE_LEVEL + 1,XMESSAGE_LOGHEADER h,##a)
#define DEBUG3(h,a...) xdebugN(XMESSAGE_DEBUG_BASE_LEVEL + 2,XMESSAGE_LOGHEADER h,##a)

#define ERROR VERBOSE

#define INIT_DEBUG_MARK()    DEBUG("%s : entering",function_name)
#define EXIT_DEBUG_MARK(a)   DEBUG("%s : exiting with status %d",function_name,a)

#define INIT_DEBUG2_MARK()   DEBUG2("%s : entering",function_name)
#define EXIT_DEBUG2_MARK(a)  DEBUG2("%s : exiting with status %d",function_name,a)

#define INIT_DEBUG3_MARK()   DEBUG3("%s : entering",function_name)
#define EXIT_DEBUG3_MARK(a)  DEBUG3("%s : exiting with status %d",function_name,a)

/* main header */
#include "xmessage.h"

int
xmessage_init(xmessage_t* msg,int type,char* buffer,size_t length){
  int fstatus=XERROR;
  char* function_name="xmessage_init";
  INIT_DEBUG2_MARK();

  msg->type=type;
  msg->length=0;

  if(length>0){
    msg->data=(void*)malloc(length*sizeof(char));
    if(msg->data!=NULL){
      msg->length=length;
      memcpy(msg->data,buffer,length);
      fstatus=XSUCCESS;
    }
    else{
      ERROR("unable to allocate memory for message data storage");
      fstatus=XERROR_MEMORY;
    }
  }
  else{
    msg->data=NULL;
    fstatus=XSUCCESS;
  }

  EXIT_DEBUG2_MARK(fstatus);
  return fstatus;
}

int
xmessage_free_contents(xmessage_t* msg){
  int fstatus=XSUCCESS;
  char* function_name="xmessage_free_contents";
  INIT_DEBUG2_MARK();

  msg->type=XPING_REQUEST;

  msg->length=0;

  if(msg->data!=NULL){
    free(msg->data);
    msg->data=NULL;
  }

  EXIT_DEBUG2_MARK(fstatus);
  return fstatus;
}

int
xmessage_marshall(xmessage_t* msg,char** pbuffer,size_t* psize){
  int fstatus=XERROR;
  char* function_name="xmessage_marshall";
  INIT_DEBUG_MARK();

  XDR xdr;
  char* buffer;
  size_t size;

  size=sizeof(int)+sizeof(long);
  size+=msg->length;

  buffer=(char*)malloc(size*sizeof(char));
  if(buffer!=NULL){
    
    xdrmem_create(&xdr,buffer,size,XDR_ENCODE);

    if(!xdr_int(&xdr,(int*)&(msg->type))){
      ERROR("unable to serialize message type");
    }
    else if(!xdr_u_long(&xdr,(unsigned long*)&(msg->length))){
      ERROR("unable to serialize message data length");
    }
    else if(!xdr_opaque(&xdr,(char*)msg->data,(u_int)msg->length)){
      ERROR("unable to serialize message data '%s' (%d)",msg->data,msg->length);
    }
    else{
      VERBOSE("message (type %d) successfully marshalled",msg->type);
      *pbuffer=buffer;
      *psize=size;
      fstatus=XSUCCESS;
    }
    
    xdr_destroy(&xdr);
    
  }
  else{
    ERROR("unable to allocate memory for message marshalling");
  }

  EXIT_DEBUG_MARK(fstatus);
  return fstatus;
}

int
xmessage_unmarshall(xmessage_t* msg,char* buffer,size_t size){
  int fstatus=XERROR;
  char* function_name="xmessage_unmarshall";
  INIT_DEBUG_MARK();

  XDR xdr;

  if(buffer!=NULL){
    
    xdrmem_create(&xdr,buffer,size,XDR_DECODE);

    if(!xdr_int(&xdr,(int*)&(msg->type))){
      ERROR("unable to deserialize message type");
    }
    else if(!xdr_u_long(&xdr,(unsigned long*)&(msg->length))){
      ERROR("unable to deserialize message data length");
    }
    else{

      VERBOSE("message type is %d",msg->type);
      VERBOSE("message length is %u",msg->length);

      if( msg->length > 0 ){
	msg->data=(char*)malloc(msg->length*sizeof(char));
	if(msg->data==NULL){
	  ERROR("unable to allocate memory for message unmarshalling : %s",strerror(errno));	  
	}
	else{

	  if(!xdr_opaque(&xdr,(char*)msg->data,(u_int)msg->length)){
	    ERROR("unable to deserialize message data");
	  }
	  else{
	    VERBOSE("message (type %d) successfully unmarshalled",msg->type);
	    fstatus=XSUCCESS;
	  }

	}
      }
      else{
	msg->data=NULL;
	msg->length=0;
	VERBOSE("message (type %d without data) successfully unmarshalled",msg->type);
	fstatus=XSUCCESS;
      }
      
    }
    
  }
  else
    ERROR("unable to unmarshall message, input buffer is NULL");

  EXIT_DEBUG_MARK(fstatus);
  return fstatus;
}
