#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#include <time.h>
#include <stdarg.h>
#include <string.h>

#include <pthread.h>

static pthread_mutex_t error_mutex = PTHREAD_MUTEX_INITIALIZER ;
#define ERROR_LOCK() pthread_mutex_lock(&error_mutex)
#define ERROR_UNLOCK() pthread_mutex_unlock(&error_mutex)

static pthread_mutex_t debug_mutex = PTHREAD_MUTEX_INITIALIZER ;
#define DEBUG_LOCK() pthread_mutex_lock(&debug_mutex)
#define DEBUG_UNLOCK() pthread_mutex_unlock(&debug_mutex)

static pthread_mutex_t verbose_mutex = PTHREAD_MUTEX_INITIALIZER ;
#define VERBOSE_LOCK() pthread_mutex_lock(&verbose_mutex)
#define VERBOSE_UNLOCK() pthread_mutex_unlock(&verbose_mutex)

#include "xlogger.h"

static int xverbose_max_level=0;
static int xdebug_max_level=0;
static int xerror_max_level=1;

static FILE* xerror_stream = NULL;
static FILE* xverbose_stream = NULL;
static FILE* xdebug_stream = NULL;


void xverbose_base(int level,char* format,va_list args);
void xdebug_base(int level,char* format,va_list args);

void xerror_setstream(FILE* stream){
  xerror_stream=stream;
}

void xerror_setmaxlevel(int level){
  xerror_max_level=level;
}

void xerror(char* format,...){

  char time_string[64];
  time_t current_time;

  /* vfprintf crash on stderr and multithread... */
  FILE* default_stream=stdout;
  FILE* stream;

  if(!xerror_max_level)
    return;

  if(xerror_stream==NULL)
    stream=default_stream;
  else
    stream=xerror_stream;

  va_list args;
  va_start(args,format);

  time(&current_time);
  ctime_r(&current_time,time_string);
  time_string[strlen(time_string)-1]='\0';

  ERROR_LOCK();
  fprintf(stream,"%s [ERROR] ",time_string);
  vfprintf(stream,format,args);
  fprintf(stream,"\n");
  fflush(stream);
  ERROR_UNLOCK();

  va_end(args);

}

void xverbose_setstream(FILE* stream){
  xverbose_stream=stream;
}

void xverbose_setmaxlevel(int level){
  xverbose_max_level=level;
}

void xverbose_base(int level,char* format,va_list args){

  char time_string[64];
  time_t current_time;

  FILE* default_stream=stdout;
  FILE* stream;

  if(xverbose_stream==NULL)
    stream=default_stream;
  else
    stream=xverbose_stream;

  if(level<=xverbose_max_level){
    time(&current_time);
    ctime_r(&current_time,time_string);
    time_string[strlen(time_string)-1]='\0';
    VERBOSE_LOCK();
    fprintf(stream,"%s [INFO%d] ",time_string,level);
    vfprintf(stream,format,args);
    fprintf(stream,"\n");
    fflush(stream);
    VERBOSE_UNLOCK();
  }
}

void xverbose(char* format,...){
  int level=XVERBOSE_LEVEL_1;
  va_list args;
  va_start(args,format);
  xverbose_base(level,format,args);
  va_end(args);
}

void xverbose2(char* format,...){
  int level=XVERBOSE_LEVEL_2;
  va_list args;
  va_start(args,format);
  xverbose_base(level,format,args);
  va_end(args);
}

void xverbose3(char* format,...){
  int level=XVERBOSE_LEVEL_3;
  va_list args;
  va_start(args,format);
  xverbose_base(level,format,args);
  va_end(args);
}

void xverboseN(int level,char* format,...){
  if(level>9)
    level=9;
  va_list args;
  va_start(args,format);
  xverbose_base(level,format,args);
  va_end(args);
}


void xdebug_setmaxlevel(int level){
  xdebug_max_level=level;
}

void xdebug_setstream(FILE* stream){
  xdebug_stream=stream;
}

void xdebug_base(int level,char* format,va_list args){

  char time_string[64];
  time_t current_time;

  FILE* default_stream=stdout;
  FILE* stream;

  if(xdebug_stream==NULL)
    stream=default_stream;
  else
    stream=xdebug_stream;
  
  if(level<=xdebug_max_level){
    time(&current_time);
    ctime_r(&current_time,time_string);
    time_string[strlen(time_string)-1]='\0';
    DEBUG_LOCK();
    fprintf(stream,"%s [DBUG%d] ",time_string,level);
    vfprintf(stream,format,args);
    fprintf(stream,"\n");
    fflush(stream);
    DEBUG_UNLOCK();
  }
  
}

void xdebug(char* format,...){
  int level=XDEBUG_LEVEL_1;
  va_list args;
  va_start(args,format);
  xdebug_base(level,format,args);
  va_end(args);
}

void xdebug2(char* format,...){
  int level=XDEBUG_LEVEL_2;
  va_list args;
  va_start(args,format);
  xdebug_base(level,format,args);
  va_end(args);
}

void xdebug3(char* format,...){
  int level=XDEBUG_LEVEL_3;
  va_list args;
  va_start(args,format);
  xdebug_base(level,format,args);
  va_end(args);
}

void xdebugN(int level,char* format,...){
  if(level>9)
    level=9;
  va_list args;
  va_start(args,format);
  xdebug_base(level,format,args);
  va_end(args);
}
