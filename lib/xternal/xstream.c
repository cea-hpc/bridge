/*****************************************************************************\
 *  lib/xternal/xstream.c - 
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

#include "config.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include <string.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>

#include <fcntl.h>
#include <sys/poll.h>

#include <errno.h>
extern int errno;

#include "xlogger.h"

#include "xstream.h"

#define DUMP_ERROR(e,s,S) { char* rc=strerror_r((int)e,(char*)s,(size_t)S); \
  if(rc==0) \
    { \
      s[0]='-'; \
      s[1]='\0'; \
    } \
}

#ifndef XSTREAM_LOGHEADER
#define XSTREAM_LOGHEADER "xstream: "
#endif

#ifndef XSTREAM_VERBOSE_BASE_LEVEL
#define XSTREAM_VERBOSE_BASE_LEVEL 7
#endif

#ifndef XSTREAM_DEBUG_BASE_LEVEL
#define XSTREAM_DEBUG_BASE_LEVEL   7
#endif

#define VERBOSE(h,a...) xverboseN(XSTREAM_VERBOSE_BASE_LEVEL,XSTREAM_LOGHEADER h,##a)
#define VERBOSE2(h,a...) xverboseN(XSTREAM_VERBOSE_BASE_LEVEL + 1,XSTREAM_LOGHEADER h,##a)
#define VERBOSE3(h,a...) xverboseN(XSTREAM_VERBOSE_BASE_LEVEL + 2,XSTREAM_LOGHEADER h,##a)

#define DEBUG(h,a...) xdebugN(XSTREAM_DEBUG_BASE_LEVEL,XSTREAM_LOGHEADER h,##a)
#define DEBUG2(h,a...) xdebugN(XSTREAM_DEBUG_BASE_LEVEL + 1,XSTREAM_LOGHEADER h,##a)
#define DEBUG3(h,a...) xdebugN(XSTREAM_DEBUG_BASE_LEVEL + 2,XSTREAM_LOGHEADER h,##a)

#define ERROR VERBOSE

#define INIT_DEBUG_MARK()    DEBUG("%s : entering",function_name)
#define EXIT_DEBUG_MARK(a)   DEBUG("%s : exiting with status %d",function_name,a)

#define INIT_DEBUG2_MARK()   DEBUG2("%s : entering",function_name)
#define EXIT_DEBUG2_MARK(a)  DEBUG2("%s : exiting with status %d",function_name,a)

#define INIT_DEBUG3_MARK()   DEBUG3("%s : entering",function_name)
#define EXIT_DEBUG3_MARK(a)  DEBUG3("%s : exiting with status %d",function_name,a)

int
xstream_create(const char* hostname,
	       const char* servicename)
{
  char* function_name="xstream_create";
  INIT_DEBUG2_MARK();

  int sock;
  int authorization;

  struct addrinfo* ai;
  struct addrinfo* aitop;
  struct sockaddr_in addr;
  struct sockaddr_in addresse;

  struct addrinfo hints;

  int fstatus=XERROR;
  int status=-1;

  /* create an AF_INET socket */
  if ( ( sock = socket(AF_INET, SOCK_STREAM, 0) ) < 0 ){
    ERROR("socket creation failed : %s",strerror(errno));
    return XERROR_STREAM_SOCKET_FAILED;
  }
  VERBOSE("socket creation succeed");
  
  /* set reuse flag, restart will not crash due to an already bound TCP port */
  authorization=1;
  if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &authorization, sizeof(int))){
    ERROR("socket option set up failed : %s",strerror(errno));
    close(sock);
    return XERROR_STREAM_SETSOCKOPT_FAILED;
  }
  VERBOSE("socket REUSEADDR option is now set");
  
  /* 
   * Set hint flag in order to listen on any address 
   * if hostname is not specified
   */
  memset(&hints,0,sizeof(hints));
  hints.ai_flags=AI_PASSIVE;
  hints.ai_family=AF_INET;
  
  /*
   * get 'hostname' network informations
   */
  status=getaddrinfo(strnlen(hostname,1)?hostname:NULL,servicename,&hints,&aitop);
  if(status){
    ERROR("getaddrinfo (%s:%s) failed : %s",hostname,servicename,gai_strerror(status));
    close(sock);
    return XERROR_STREAM_GETADDRINFO_FAILED;
  }
  else{
    VERBOSE("getaddrinfo (%s:%s) succeed",hostname,servicename);

    /*
     * For all returned addresses, try to bind socket on it
     * exits when it succeeds or fail after all tries
     */
    for(ai=aitop; ai; ai=ai->ai_next){
      memcpy(&addr,ai->ai_addr,ai->ai_addrlen);
      
      if(addr.sin_family==AF_INET){
	memset(& addresse, 0, sizeof(struct sockaddr_in));
	addresse.sin_family = AF_INET;
	addresse.sin_port = addr.sin_port;
	addresse.sin_addr.s_addr = addr.sin_addr.s_addr;
	
	if(bind(sock, (struct sockaddr*) &addresse, sizeof(struct sockaddr_in))<0){
	  ERROR("bind(%s:%d) failed : %s",inet_ntoa(addr.sin_addr),ntohs(addr.sin_port),strerror(errno));
	  fstatus=XERROR_STREAM_BIND_FAILED;
	  continue;
	}
	else{
	  VERBOSE("bind(%s:%d) succeed",inet_ntoa(addr.sin_addr),ntohs(addr.sin_port));
	  fstatus=XSUCCESS;
	  break;
	} /* bind */

      } /* AF_INET check */

    } /* for(ai=...) */

    /* free addrinfo structures */
    freeaddrinfo(aitop);
    
  } /* getaddrinfo */
  

  /*
   * Return the socket file descriptor if success, -1 otherwise
   */
  if(fstatus==XSUCCESS)
    fstatus=sock;
  else{
    close(sock);  
  }
  
  EXIT_DEBUG2_MARK(fstatus);

  return fstatus;
}


int
xstream_connect(const char* hostname,
		const char* servicename,
		time_t timeout)
{
  char* function_name="xstream_connect";
  INIT_DEBUG2_MARK();

  int sock;
  int sock_flags = 0;

  struct addrinfo* ai;
  struct addrinfo* aitop;
  struct sockaddr_in addr;
  struct sockaddr_in addresse;
  
  socklen_t optlen;
  struct addrinfo hints;

  int rc;
  struct pollfd ufds;
  int sockopt;

  int fstatus=XERROR;
  int status=-1;

  /* set hint flag that indicate to get TCP/IP information only */
  memset(&hints,0,sizeof(hints));
  hints.ai_family=AF_INET;
  hints.ai_socktype=SOCK_STREAM;

  /*
   * get 'hostname' network informations
   */
  status=getaddrinfo(hostname,servicename,&hints,&aitop);
  if(status){
    ERROR("getaddrinfo (%s:%s) failed : %s",hostname,servicename,gai_strerror(status));
    return XERROR_STREAM_GETADDRINFO_FAILED;
  }
  else{

    /*
     * for all returned addresses try to connect the socket to
     */
    for(ai=aitop; ai; ai=ai->ai_next){
      memset(&addresse, 0, sizeof(struct sockaddr_in));
      memcpy(&addr,ai->ai_addr,ai->ai_addrlen);

	addresse.sin_family = AF_INET;
	addresse.sin_port = addr.sin_port;
	addresse.sin_addr.s_addr = addr.sin_addr.s_addr;

	/* create an AF_INET socket */
	if (( sock = socket(AF_INET, SOCK_STREAM, 0) ) < 0 ){
	  ERROR("socket creation failed : %s",strerror(errno));
	  fstatus=XERROR_STREAM_SOCKET_FAILED;
	  continue;
	}
	VERBOSE("socket creation succeed");
	
	/* if timeout is not zero, set non blocking mode */
	if(timeout!=0){
	  sock_flags=fcntl(sock,F_GETFL);
	  if(fcntl(sock,F_SETFL, sock_flags | O_NONBLOCK)){
	    ERROR("unable to set socket non-blocking flag : %s",strerror(errno));
	    close(sock);
	    fstatus=XERROR_STREAM_SETSOCKOPT_FAILED;
	    continue;
	  }
	  VERBOSE("socket non-blocking flag is now set");
	}

	rc=connect(sock, (struct sockaddr*) &addresse, sizeof(struct sockaddr_in));
	/* connection failed */
	if(rc<0 && errno != EINPROGRESS && errno != EALREADY){
	  ERROR("connect (%s:%d) failed : %s (%d)",inet_ntoa(addresse.sin_addr),
		ntohs(addresse.sin_port),strerror(errno),errno);
	  fstatus=XERROR_STREAM_CONNECT_FAILED;
	  close(sock);
	  continue;
	}
	/* connection in progress */
	else if(rc<0){
	  ufds.fd=sock;
	  ufds.events= POLLIN | POLLOUT ;
	  ufds.revents=0;
	  /* poll socket */
	  do{
	    rc=poll(&ufds,1,timeout);
	  }
	  while(rc==-1 && (errno==EINTR || errno==EALREADY));

	  if(rc==-1){
	    ERROR("poll (%s:%d) failed : %s",inet_ntoa(addresse.sin_addr),ntohs(addresse.sin_port),
		  strerror(errno));
	  }
	  else if(rc==0){
	    ERROR("poll (%s:%d) times out",inet_ntoa(addresse.sin_addr),ntohs(addresse.sin_port));
	  }
	  else{
	    /* we have to verify that this is not an error that trigger poll success */
	    optlen=sizeof(sockopt);
	    rc=getsockopt(sock,SOL_SOCKET,SO_ERROR,&sockopt,&optlen);
	    if(rc<0){
	      ERROR("unable to get socket SO_ERROR value despite of %s:%d polling success : %s",
		    inet_ntoa(addresse.sin_addr),ntohs(addresse.sin_port),strerror(errno));
	    }
	    else{
	      if(sockopt){
		ERROR("connect (%s:%d) failed while polling : %s",inet_ntoa(addresse.sin_addr),
		      ntohs(addresse.sin_port),strerror(sockopt));
	      }
	      else{
		VERBOSE("connect (%s:%d) succeed while polling",inet_ntoa(addresse.sin_addr),
			ntohs(addresse.sin_port));
		fstatus=XSUCCESS;
		break;
	      }
	    }
	  }
	  close(sock);
	  fstatus=XERROR_STREAM_POLL_ERROR;
	}
	/* connection succeed immediately */
	else{
	  VERBOSE("connect (%s:%d) immediately succeed",inet_ntoa(addresse.sin_addr),
		  ntohs(addresse.sin_port));
	  fstatus=XSUCCESS;
	  break;
	}
	
    } /* for (ai=...) */

    /* free addrinfo structures */
    freeaddrinfo(aitop);

  }
  
  /* reverse socket flags */
  if(timeout!=0){
    fcntl(sock,F_SETFL,sock_flags);
  }

  /*
   * Return the socket file descriptor if success, -1 otherwise
   */
  if(fstatus==XSUCCESS)
    fstatus=sock;
  else{
    close(sock);    
    fstatus=-1;
  }

  EXIT_DEBUG2_MARK(fstatus);

  return fstatus;
}


int
xstream_accept(int socket){
  char* function_name="xstream_accept";
  INIT_DEBUG2_MARK();

  int incoming_stream;
  struct sockaddr_in remote_addr;
  socklen_t addrlen;

  int fstatus=XERROR;
  
  addrlen=sizeof(remote_addr);

  incoming_stream=accept(socket,(struct sockaddr *)&remote_addr,&addrlen);
  if(incoming_stream<0 && errno==EINTR){
    ERROR("error while accepting incoming request : interrupted");
    fstatus=XERROR_EINTR;
  }
  else if(incoming_stream<0){
    ERROR("error while accepting incoming request : %s",strerror(errno));
  }
  else{
    fstatus=incoming_stream;
  }
  
  EXIT_DEBUG2_MARK(fstatus);

  return fstatus;
}

int
xstream_close(int socket){
  int fstatus=-1;

  close(socket);

  return fstatus;
}

int
xstream_listen(int socket,int backlog){
  int fstatus=XERROR;
  char* function_name="xstream_listen";
  INIT_DEBUG2_MARK();

  fstatus=listen(socket,backlog);
  if(fstatus!=0){
    ERROR("error while specifying stream listening queue length : %s",strerror(errno));
  }
  
  EXIT_DEBUG2_MARK(fstatus);

  return fstatus;
}


int xstream_send(int socket,char* buffer,size_t length){

  return xstream_send_timeout(socket,buffer,length,0);

}

int xstream_send_timeout(int socket,char* buffer,size_t length,int timeout){
  int fstatus=XERROR;
  int rc;
  size_t written_bytes;

  char test;

  int sock_flags=0;
  int nonblock=0;
  struct pollfd ufds;

  struct timeval start_time;
  struct timeval current_time;
  int timeleft;

  /* set non block mode if required */
  if(timeout!=0){
    sock_flags=fcntl(socket,F_GETFL);
    if(fcntl(socket,F_SETFL, sock_flags | O_NONBLOCK)){
      ERROR("unable to set socket non-blocking flag : %s",strerror(errno));
      return XERROR_STREAM_SETSOCKOPT_FAILED;
    }
    else{
      VERBOSE("socket non-blocking flag is now set");
      nonblock=1;

      ufds.fd=socket;
      ufds.events=POLLOUT;
    }
  }
  
  /* get start time */
  gettimeofday(&start_time,NULL);

  /* send data */
  written_bytes=0;
  while(written_bytes<length){

    /* attempt polling if non block mode is activated */
    if(nonblock){
      VERBOSE3("looking for POLLOUT events on socket %d",socket);

      gettimeofday(&current_time,NULL);
      timeleft=timeout
	-(current_time.tv_sec-start_time.tv_sec)*1000
	-(current_time.tv_sec-start_time.tv_sec)/1000;
      
      if(timeleft<=0){
	ERROR("send at %d/%d bytes transmitted : timeout",
	      written_bytes,length);
	fstatus=XERROR_STREAM_TIMEOUT;
	break;
      }

      if((rc=poll(&ufds,1,timeleft))<=0){
	if(rc<0 && (errno==EINTR || errno==EAGAIN)){
	  continue;
	}
	else if(rc==0){
	  continue;
	}
	else if(rc<0){
	  ERROR("send at %d/%d bytes transmitted : poll error : %s",
		written_bytes,length,strerror(errno));
	  fstatus=XERROR_STREAM_POLL_ERROR;
	  break;
	}
	else{
	  
	  /* we just check that the socket is still here */
	  /* read from a closed nonblocking socket should return 0 */
	  do{
	    rc=read(socket,&test,1);
	  }
	  while(rc<0 && errno==EINTR);
	  if(rc==0){
	    ERROR("send at %d/%d bytes transmitted : socket is gone",
		  written_bytes,length);
	    fstatus=XERROR_STREAM_SOCKET_CLOSED;
	    break;
	  }
	  
	}
      }
      
      /* send data */
      rc=write(socket,buffer+written_bytes,length-written_bytes);
      VERBOSE3("write return code is %d (errno=%d)",rc,errno);

    }
    else {

      /* send data */
      do{
	rc=write(socket,buffer+written_bytes,length-written_bytes);
	VERBOSE3("write return code is %d (errno=%d)",rc,errno);
      }
      while(rc<0 && (errno==EINTR || errno==EAGAIN));

    }

    /* process write return code */
    if(rc>0)
      written_bytes+=rc;
    else if(rc) {
      fstatus=rc;
      break;
    }
    else
      break;
    
  }
  
  /* reverse socket flags */
  if(timeout!=0){
    fcntl(socket,F_SETFL,sock_flags);
  }

  if(written_bytes==length){
    fstatus=XSUCCESS;
  }
    
  return fstatus;
}


int xstream_receive(int socket,char* buffer,size_t length){

  return xstream_receive_timeout(socket,buffer,length,0);

}

int xstream_receive_timeout(int socket,char* buffer,size_t length,int timeout){
  int fstatus=-1;
  int rc;
  size_t read_bytes;
  
  int sock_flags;
  int nonblock=0;
  struct pollfd ufds;

  struct timeval start_time;
  struct timeval current_time;
  int timeleft;

  /* set non block mode if required */
  if(timeout!=0){
    sock_flags=fcntl(socket,F_GETFL);
    if(fcntl(socket,F_SETFL, sock_flags | O_NONBLOCK)){
      ERROR("unable to set socket non-blocking flag : %s",strerror(errno));
      return XERROR_STREAM_SETSOCKOPT_FAILED;
    }
    else{
      VERBOSE("socket non-blocking flag is now set");
      nonblock=1;

      ufds.fd=socket;
      ufds.events=POLLIN;
    }
  }
  
  /* get start time */
  gettimeofday(&start_time,NULL);

  /* send data */
  read_bytes=0;
  while(read_bytes<length){
    
    /* attempt polling if non block mode is activated */
    if(nonblock){
      VERBOSE3("looking for POLLIN events on socket %d",socket);
      
      gettimeofday(&current_time,NULL);
      timeleft=timeout
	-(current_time.tv_sec-start_time.tv_sec)*1000
	-(current_time.tv_sec-start_time.tv_sec)/1000;
      
      if(timeleft<=0){
	ERROR("receive at %d of %d bytes : timeout",
	      read_bytes,length);
	fstatus=XERROR_STREAM_TIMEOUT;
	break;
      }

      if((rc=poll(&ufds,1,timeleft))<=0){
	if(rc<0 && (errno==EINTR || errno==EAGAIN)){
	  continue;
	}
	else if(rc==0){
	  continue;
	}
	else if(rc<0){
	  ERROR("receive at %d of %d bytes : poll error : %s",
		read_bytes,length,strerror(errno));
	  fstatus=XERROR_STREAM_POLL_ERROR;
	  break;
	}
      }
      
      /* read data from socket */
      rc=read(socket,buffer+read_bytes,length-read_bytes);
      VERBOSE3("read return code is %d (errno=%d)",rc,errno);

    }
    else {

      /* read data from socket */
      do{
	rc=read(socket,buffer+read_bytes,length-read_bytes);
	VERBOSE3("read return code is %d (errno=%d)",rc,errno);
      }
      while(rc<0 && (errno==EINTR || errno==EAGAIN));

    }
    /*_*/ /* attempt polling if required */

    /* process read return code */
    if(rc>0)
      read_bytes+=rc;
    else if (rc==0) {
      ERROR("receive at %d of %d bytes : 0 bytes received during read op",
	    read_bytes,length);
      fstatus=XERROR_STREAM_SOCKET_CLOSED;
      break;
    }
    else {
      ERROR("receive at %d of %d bytes : bad return code on read op : %d",
	    read_bytes,length,rc);
      fstatus=rc;
      break;
    }
    
  }

  if(read_bytes==length){
    fstatus=XSUCCESS;
  }
    
  return fstatus;
}

int xstream_send_msg_timeout(int socket,char* buffer,size_t length,int timeout){

  int fstatus=-1;
  uint32_t nlength;
  
  /* send message length */
  nlength=htonl(length);
  fstatus=xstream_send_timeout(socket,(char*)&nlength,sizeof(uint32_t),timeout);
  if(fstatus!=XSUCCESS){
    ERROR("unable to send message length (%d)",length);
  }
  else{
    VERBOSE("message length (%d) successfully send",length);

    /* send message data */
    fstatus=xstream_send(socket,buffer,length);
    if(fstatus==XSUCCESS){
      VERBOSE("message successfully send");
    }
    else{
      ERROR("unable to send message");
    }
    
  }

  return fstatus;
}


int xstream_receive_msg_timeout(int socket,char** buffer,size_t* length,int timeout){

  int fstatus=XERROR;
  uint32_t nlength;
  
  char* mbuf;
  size_t mlen;
  
  /* receive message length */
  fstatus=xstream_receive_timeout(socket,(char*)&nlength,sizeof(uint32_t),timeout);
  if(fstatus){
    ERROR("unable to receive message length");
  }
  else{
    mlen=ntohl(nlength);
    VERBOSE("message length (%d) successfully received",mlen);

    /* allocate memory for message */
    mbuf=(char*)malloc(mlen*sizeof(char));
    if(mbuf==NULL){
      fstatus=XERROR_MEMORY;
    }
    else{

      /* receive message data */
      fstatus=xstream_receive(socket,mbuf,mlen);
      if(fstatus){
	ERROR("unable to receive message");
	free(mbuf);
      }
      else{
	*buffer=mbuf;
	*length=mlen;
	VERBOSE("message successfully received");
	fstatus=XSUCCESS;
      }

    }
    
  }

  return fstatus;
}


int xstream_send_msg(int socket,char* buffer,size_t length){

  return xstream_send_msg_timeout(socket,buffer,length,0);

}


int xstream_receive_msg(int socket,char** buffer,size_t* length){

  return xstream_receive_msg_timeout(socket,buffer,length,0);

}

