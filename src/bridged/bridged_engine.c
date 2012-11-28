/*****************************************************************************\
 *  src/bridged/bridged_engine.c - 
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

#include "xternal/xlogger.h"
#include "bridged_engine.h"

#include "confparse/config_parsing.h"
extern char extern_errormsg[1024];

#define xfree(a) if(a!=NULL){free(a);a=NULL;}

#define LOGHEADER "bridged_engine: "

#define BRIDGED_ENGINE_VERBOSE_BASE_LEVEL 1
#define BRIDGED_ENGINE_DEBUG_BASE_LEVEL   1

#define VERBOSE(h,a...) xverboseN(BRIDGED_ENGINE_VERBOSE_BASE_LEVEL,LOGHEADER h,##a)
#define VERBOSE2(h,a...) xverboseN(BRIDGED_ENGINE_VERBOSE_BASE_LEVEL + 1,LOGHEADER h,##a)
#define VERBOSE3(h,a...) xverboseN(BRIDGED_ENGINE_VERBOSE_BASE_LEVEL + 2,LOGHEADER h,##a)

#define DEBUG(h,a...) xdebugN(BRIDGED_ENGINE_DEBUG_BASE_LEVEL,LOGHEADER h,##a)
#define DEBUG2(h,a...) xdebugN(BRIDGED_ENGINE_DEBUG_BASE_LEVEL + 1,LOGHEADER h,##a)
#define DEBUG3(h,a...) xdebugN(BRIDGED_ENGINE_DEBUG_BASE_LEVEL + 2,LOGHEADER h,##a)

#define ERROR VERBOSE

#define INIT_DEBUG_MARK()    DEBUG("%s : entering",function_name)
#define EXIT_DEBUG_MARK(a)   DEBUG("%s : exiting with status %d",function_name,a)

#define INIT_DEBUG2_MARK()   DEBUG2("%s : entering",function_name)
#define EXIT_DEBUG2_MARK(a)  DEBUG2("%s : exiting with status %d",function_name,a)

#define INIT_DEBUG3_MARK()   DEBUG3("%s : entering",function_name)
#define EXIT_DEBUG3_MARK(a)  DEBUG3("%s : exiting with status %d",function_name,a)

char* default_bridged_conf_file = BRIDGED_CONF ;

int
bridged_engine_free_contents(bridged_engine_t* engine){
  int fstatus=-1;

  xfree(engine->config_file);

  xfree(engine->address);
  xfree(engine->port);

  xfree(engine->cachedir);
  xfree(engine->logfile);
  xfree(engine->debugfile);

  engine->timeout=0;

  engine->loglevel=0;
  engine->debuglevel=0;

  engine->worker_nb=10;
  engine->queue_size=100;

  fstatus=0;

  return fstatus;
}

int
bridged_engine_init(bridged_engine_t* engine,
		    char* conf_file,
		    char* address,
		    char* port,
		    char* cachedir,
		    time_t timeout,
		    char* logfile,
		    int loglevel,
		    char* debugfile,
		    int debuglevel,
		    int worker_nb,
		    int queue_size){
  int fstatus=-1;
  
  /* check input parameters */
  if(
     port == NULL ||
     
     cachedir == NULL ||
     logfile == NULL ||
     debugfile == NULL
     )
    {
      ERROR("all required input fields are not present");
      return fstatus;
    }

  if ( conf_file==NULL ) {
    engine->config_file=NULL;
  }
  else {
    engine->config_file=strdup(conf_file);
    if ( engine->config_file==NULL ) {
      fstatus=-2;
      ERROR("unable to init engine config file");
      goto exit;
    }
  }

  /* set parameters into engine structure */
  if ( address == NULL )
    engine->address=NULL;
  else
    engine->address=strdup(address);

  engine->port=strdup(port);

  engine->cachedir=strdup(cachedir);
  engine->timeout=timeout;

  engine->logfile=strdup(logfile);
  engine->loglevel=loglevel;

  engine->debugfile=strdup(debugfile);
  engine->debuglevel=debuglevel;

  engine->worker_nb=worker_nb;
  engine->queue_size=queue_size;

  /* check engine structure parameters validity */
  if(
     engine->address == NULL ||
     engine->port == NULL ||
     
     engine->cachedir == NULL ||
     engine->logfile == NULL ||
     engine->debugfile == NULL){

    ERROR("unable to init bridged engine because of memory allocation pb");
    bridged_engine_free_contents(engine);
  }
  else{
    VERBOSE2("engine %s is '%s'","configuration file",engine->config_file);
    VERBOSE2("engine %s is '%s'","daemon address",engine->address);
    VERBOSE2("engine %s is %s","daemon port",engine->port);

    VERBOSE2("engine %s is %s","cachedir",engine->cachedir);
    VERBOSE2("engine %s is %u","timeout",engine->timeout);
    VERBOSE2("engine %s is %s","logfile",engine->logfile);
    VERBOSE2("engine %s is %d","loglevel",engine->loglevel);
    VERBOSE2("engine %s is %s","debugfile",engine->debugfile);
    VERBOSE2("engine %s is %d","debuglevel",engine->debuglevel);

    VERBOSE2("engine has %d %s",engine->worker_nb,"worker(s)");
    VERBOSE2("engine %s is %d","queue size",engine->queue_size);

    fstatus=0;
  }
  
  exit:

  return fstatus;
}

int
bridged_engine_init_from_config_file(bridged_engine_t* engine,char* conf_file){
  int fstatus=-1;

  config_file_t config;
  int block_nb;

  char* loglevel_string;
  char* debuglevel_string;
  char* timeout_string;

  char* worker_string;
  char* queue_string;

  int i;

  /* parse configuration file */
  if ( conf_file == NULL ){
    conf_file = getenv("BRIDGED_CONF") ;
  }
  if ( conf_file == NULL ){
    conf_file = default_bridged_conf_file ;
  }

  config = config_ParseFile(conf_file);
  if(!config){
    ERROR("unable to parse configuration file %s : %s",conf_file,extern_errormsg);
  }
  else{
    
    /* get conf blocks quantity */
    block_nb=config_GetNbBlocks(config);
    if(block_nb<=0){
      ERROR("unable to get configuration blocks from config file %s : %s",
	    conf_file,extern_errormsg);
    }
    else{

      fstatus=-1;
      /* look for relevants block and add contents to engine conf */
      for (i=0;i<block_nb;i++){
	char* block_name;
	block_name = config_GetBlockName(config,i);
	  
	if(strncmp("bridged",block_name,8)==0){
	  fstatus=1;
	  VERBOSE("initializing engine from 'bridged' block of file %s",conf_file);

	  if((loglevel_string=config_GetKeyValueByName(config,i,"loglevel")) == NULL){
	    loglevel_string="1";
	  }
	  if((debuglevel_string=config_GetKeyValueByName(config,i,"debuglevel")) == NULL){
	    debuglevel_string="1";
	  }
	  if((timeout_string=config_GetKeyValueByName(config,i,"timeout")) == NULL){
	    timeout_string="30";
	  }

	  if((worker_string=config_GetKeyValueByName(config,i,"worker")) == NULL){
	    worker_string="10";
	  }
	  if((queue_string=config_GetKeyValueByName(config,i,"queue")) == NULL){
	    queue_string="100";
	  }

	  fstatus=bridged_engine_init(engine,
				      conf_file,
				      config_GetKeyValueByName(config,i,"address"),
				      config_GetKeyValueByName(config,i,"port"),
				      config_GetKeyValueByName(config,i,"cachedir"),
				      strtol(timeout_string,NULL,10)*1000,
				      config_GetKeyValueByName(config,i,"logfile"),
				      strtol(loglevel_string,NULL,10),
				      config_GetKeyValueByName(config,i,"debugfile"),
				      strtol(debuglevel_string,NULL,10),
				      strtol(worker_string,NULL,10),
				      strtol(queue_string,NULL,10));
	  
	  /* init ok, break */
	  if(fstatus==0){
	    VERBOSE("initialization succeed");
	    break;
	  }
	  else
	    ERROR("initialization failed");
	  
	} /* EOF config block */
	
      } /* EOF for */
      

      if ( fstatus < 0 ) {
	ERROR("unable to get configuration block 'bridged' from config file %s",
	      conf_file);
      }

    }
    
    /* free config file */
    config_Free(config);
    
  }

  return fstatus;
}
