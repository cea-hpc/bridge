/*****************************************************************************\
 *  lib/xternal/xqueue.c - 
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

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include <pthread.h>

#include "xerror.h"

/* logging */
#include "xlogger.h"

#ifndef XQUEUE_LOGHEADER
#define XQUEUE_LOGHEADER "xqueue: "
#endif

#ifndef XQUEUE_VERBOSE_BASE_LEVEL
#define XQUEUE_VERBOSE_BASE_LEVEL 7
#endif

#ifndef XQUEUE_DEBUG_BASE_LEVEL
#define XQUEUE_DEBUG_BASE_LEVEL   7
#endif

#define VERBOSE(h,a...) xverboseN(XQUEUE_VERBOSE_BASE_LEVEL,XQUEUE_LOGHEADER h,##a)
#define VERBOSE2(h,a...) xverboseN(XQUEUE_VERBOSE_BASE_LEVEL + 1,XQUEUE_LOGHEADER h,##a)
#define VERBOSE3(h,a...) xverboseN(XQUEUE_VERBOSE_BASE_LEVEL + 2,XQUEUE_LOGHEADER h,##a)

#define DEBUG(h,a...) xdebugN(XQUEUE_DEBUG_BASE_LEVEL,XQUEUE_LOGHEADER h,##a)
#define DEBUG2(h,a...) xdebugN(XQUEUE_DEBUG_BASE_LEVEL + 1,XQUEUE_LOGHEADER h,##a)
#define DEBUG3(h,a...) xdebugN(XQUEUE_DEBUG_BASE_LEVEL + 2,XQUEUE_LOGHEADER h,##a)

#define ERROR VERBOSE

#define INIT_DEBUG_MARK()    DEBUG("%s : entering",function_name)
#define EXIT_DEBUG_MARK(a)   DEBUG("%s : exiting with status %d",function_name,a)

#define INIT_DEBUG2_MARK()   DEBUG2("%s : entering",function_name)
#define EXIT_DEBUG2_MARK(a)  DEBUG2("%s : exiting with status %d",function_name,a)

#define INIT_DEBUG3_MARK()   DEBUG3("%s : entering",function_name)
#define EXIT_DEBUG3_MARK(a)  DEBUG3("%s : exiting with status %d",function_name,a)

/* main header */
#include "xqueue.h"

/* local functions */
int xqueue_get_length(xqueue_t* queue,int* length);

int
xqueue_init(xqueue_t* queue,unsigned int default_length,size_t item_size){
  int fstatus=XERROR;
  char* function_name="xqueue_init";
  INIT_DEBUG2_MARK();

  queue->head=NULL;
  queue->tail=NULL;

  /* mutex initialization */
  fstatus=pthread_mutex_init(&(queue->mutex),NULL);
  if(fstatus){
    fstatus=XERROR_MUTEX_INIT_FAILED;
  }
  else{
    /* condition initialization */
    fstatus=pthread_cond_init(&(queue->condition),NULL);
    if(fstatus){
      fstatus=XERROR_CONDITION_INIT_FAILED;
    }
    else{
      /* freelist initialization */
      fstatus=xfreelist_init(&(queue->freelist),default_length,item_size);
      if(fstatus){
	fstatus=XERROR;
	/* an error occurred - destroy condition */
	pthread_cond_destroy(&(queue->condition));
      }
    }
    /*_*/ /* condition init */

    /* an error occurred - destroy mutex */
    if(fstatus){
      pthread_mutex_destroy(&(queue->mutex));
    }

  }
  /*_*/ /* mutex init */
  
  EXIT_DEBUG2_MARK(fstatus);
  return fstatus;
}

int
xqueue_free_contents(xqueue_t* queue){
  int fstatus;
  char* function_name="xqueue_free_contents";
  INIT_DEBUG2_MARK();

  xfreelist_item_t* item;
  xfreelist_t* freelist;

  /* release queued items */
  freelist=&(queue->freelist);
  if(freelist!=NULL){
    item=queue->head;
    while(item!=NULL){
      xfreelist_release_item(freelist,item);
      item=item->next;
    }
  }

  queue->head=NULL;
  queue->tail=NULL;

  /* condition destruction */
  pthread_cond_destroy(&(queue->condition));

  /* mutex destruction */
  pthread_mutex_destroy(&(queue->mutex));

  /* free freelist contents */
  fstatus=xfreelist_free_contents(&(queue->freelist));
  
  EXIT_DEBUG2_MARK(fstatus);
  return fstatus;
}

int
xqueue_enqueue(xqueue_t* queue,void* data,size_t length){
  int fstatus;
  char* function_name="xqueue_enqueue";
  INIT_DEBUG_MARK();

  xfreelist_t* freelist;
  xfreelist_item_t* item;

  fstatus=pthread_mutex_lock(&(queue->mutex));
  if(fstatus){
    return XERROR_MUTEX_LOCK_FAILED;
  }
  pthread_cleanup_push(pthread_mutex_unlock,(void*)(&(queue->mutex)));

  /* extract an item from freelist */
  freelist=&(queue->freelist);
  if(freelist==NULL){
    fstatus=XERROR_QUEUE_FREELIST_IS_NULL;
    goto exit;
  }
  
  /* check input element size */
  if(length>freelist->item_size){
    fstatus=XERROR_FREELIST_IS_EMPTY;
    goto exit;
  }
  

  do{
    
    /* extract an item from freelist */
    fstatus=xfreelist_extract_item(freelist,&item);

    if(fstatus==XERROR_FREELIST_IS_EMPTY){
      VERBOSE("enqueuing: queue's freelist is empty, waiting for dequeuing");
      pthread_cond_wait(&(queue->condition),&(queue->mutex));
    }

  }
  while(fstatus==XERROR_FREELIST_IS_EMPTY);

  if(fstatus){
    fstatus=XERROR_QUEUE_FREELIST_EXTRACT_ITEM;
  }
  else{

    /* copy input element into extracted item */
    memcpy(item->data,data,length);

    /* enqueue item on queue */
    if(queue->head==NULL){
      /* empty queue */
      item->next=NULL;
      queue->head=item;
      queue->tail=item;
    }
    else{
      /* non empty queue */
      item->next=queue->head;
      queue->head->previous=item;
      queue->head=item;
    }

    fstatus=XSUCCESS;
  }

 exit:

  if(fstatus==XSUCCESS)
    pthread_cond_signal(&(queue->condition));

  pthread_cleanup_pop(1); /*   pthread_mutex_unlock(&(queue->mutex)) */

  EXIT_DEBUG_MARK(fstatus);
  return fstatus;
}

int
xqueue_dequeue(xqueue_t* queue,void* data,size_t length){
  int fstatus=XERROR;
  char* function_name="xqueue_dequeue";
  INIT_DEBUG_MARK();

  xfreelist_item_t* item;
  xfreelist_t* freelist;

  fstatus=pthread_mutex_lock(&(queue->mutex));
  if(fstatus){
    ERROR("unable to lock queue for dequeuing");
    return XERROR_MUTEX_LOCK_FAILED;
  }
  pthread_cleanup_push(pthread_mutex_unlock,(void*)(&(queue->mutex)));

  do{
    
    /* check for queue emptylessness */
    if(queue->tail==NULL){
      VERBOSE("dequeuing: queue is empty, waiting for element enqueuing...");
      pthread_cond_wait(&(queue->condition),&(queue->mutex));
    }

  }
  while(queue->tail==NULL);

  if(queue->tail==NULL){
    fstatus=XERROR_QUEUE_IS_EMPTY;
  }
  else{

    /* check freelist validity */
    freelist=&(queue->freelist);
    if(freelist==NULL){
      fstatus=XERROR_QUEUE_FREELIST_IS_NULL;
      goto exit;
    }

    /* check output element size */
    if(length>freelist->item_size){
      fstatus=XERROR_FREELIST_IS_EMPTY;
      goto exit;
    }
    
    /* get last item */
    item=queue->tail;
    
    /* shift queue */
    queue->tail=item->previous;
    if(queue->tail!=NULL)
      queue->tail->next=NULL;
    else
      queue->head=NULL;
    
    /* isolate dequeued item */
    item->next=NULL;
    item->previous=NULL;

    /* copy item data into output element */
    memcpy(data,item->data,length);

    /* TODO : zeroed data */

    /* release item to freelist */
    fstatus=xfreelist_release_item(freelist,item);

  }

 exit:

  if(fstatus==XSUCCESS)
    pthread_cond_signal(&(queue->condition));

  pthread_cleanup_pop(1); /*   pthread_mutex_unlock(&(queue->mutex)) */

  EXIT_DEBUG_MARK(fstatus);
  return fstatus;
}


int
xqueue_enqueue_non_blocking(xqueue_t* queue,void* data,size_t length){
  int fstatus;
  char* function_name="xqueue_enqueue";
  INIT_DEBUG_MARK();

  xfreelist_t* freelist;
  xfreelist_item_t* item;

  fstatus=pthread_mutex_lock(&(queue->mutex));
  if(fstatus){
    return XERROR_MUTEX_LOCK_FAILED;
  }

  /* extract an item from freelist */
  freelist=&(queue->freelist);
  if(freelist==NULL){
    fstatus=-2;
    goto exit;
  }

  /* check input element size */
  if(length>freelist->item_size){
    fstatus=-3;
    goto exit;
  }

  /* extract an item from freelist */
  fstatus=xfreelist_extract_item(freelist,&item);
  if(fstatus){
    fstatus=-4;
  }
  else{

    VERBOSE("copying %d into %x",*(int*)data,item);

    /* copy input element into extracted item */
    memcpy(item->data,data,length);

    /* enqueue item on queue */
    if(queue->head==NULL){
      /* empty queue */
      item->next=NULL;
      queue->head=item;
      queue->tail=item;
    }
    else{
      /* non empty queue */
      item->next=queue->head;
      queue->head->previous=item;
      queue->head=item;
    }

    fstatus=XSUCCESS;
  }

 exit:

  pthread_mutex_unlock(&(queue->mutex));
  VERBOSE("unlocked");

  EXIT_DEBUG_MARK(fstatus);
  return fstatus;
}

int
xqueue_dequeue_non_blocking(xqueue_t* queue,void* data,size_t length){
  int fstatus=-1;
  char* function_name="xqueue_dequeue";
  INIT_DEBUG_MARK();

  xfreelist_item_t* item;
  xfreelist_t* freelist;

  fstatus=pthread_mutex_lock(&(queue->mutex));
  if(fstatus){
    ERROR("unable to lock queue for dequeuing");
    return -10;
  }

  if(queue->tail==NULL){
    fstatus=-1;
  }
  else{

    /* check freelist validity */
    freelist=&(queue->freelist);
    if(freelist==NULL){
      fstatus=-2;
      goto exit;
    }

    /* check output element size */
    if(length>freelist->item_size){
      fstatus=-3;
      goto exit;
    }
    
    /* get last item */
    item=queue->tail;
    
    /* shift queue */
    queue->tail=item->previous;
    if(queue->tail!=NULL)
      queue->tail->next=NULL;
    else
      queue->head=NULL;
    
    /* isolate dequeued item */
    item->next=NULL;
    item->previous=NULL;

    /* copy item data into output element */
    memcpy(data,item->data,length);

    /* TODO : zeroed data */

    /* release item to freelist */
    fstatus=xfreelist_release_item(freelist,item);

  }

 exit:

  pthread_mutex_unlock(&(queue->mutex));

  EXIT_DEBUG_MARK(fstatus);
  return fstatus;
}


int
xqueue_wait_4_emptiness(xqueue_t* queue){
  int fstatus=-1;
  char* function_name="xqueue_wait_4_emptiness";
  INIT_DEBUG_MARK();

  int empty_flag=0;
  int queue_length;

  do{

    if(xqueue_get_length(queue,&queue_length)){
      ERROR("unable to get queue length");
      fstatus=-1;
    }
    else{

      if(queue_length==0){
	VERBOSE("queue is empty");
	empty_flag=1;
      }
      else{
	VERBOSE("queue is not empty, sleeping...");
	sleep(1);
      }

    }

  }
  while(!empty_flag);

  EXIT_DEBUG_MARK(fstatus);
  return fstatus;
}


int
xqueue_get_length(xqueue_t* queue,int* length){
  int fstatus=-1;
  char* function_name="xqueue_get_length";
  INIT_DEBUG_MARK();

  xfreelist_item_t* item;
  int queue_length=0;

  /* lock queue */
  fstatus=pthread_mutex_lock(&(queue->mutex));
  if(fstatus){
    ERROR("unable to lock queue while attempting to get queue length");
    return -1;
  }
  else{
    
    item=queue->head;
    while(item!=NULL){
      queue_length++;
      item=item->next;
    }

    /* unlock queue */
    pthread_mutex_unlock(&(queue->mutex));

    *length=queue_length;
    fstatus=XSUCCESS;
  }

  EXIT_DEBUG_MARK(fstatus);
  return fstatus;
}
