#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <string.h>

#include "bridged/bridge_engine.h"

#include "xternal/xlogger.h"

#include "confparse/config_parsing.h"
extern char extern_errormsg[1024];

#define xfree(a) if(a!=NULL){free(a);a=NULL;}

#define LOGHEADER "bridge_engine: "

#define BRIDGE_ENGINE_VERBOSE_BASE_LEVEL 1
#define BRIDGE_ENGINE_DEBUG_BASE_LEVEL   1

#define VERBOSE(h,a...) xverboseN(BRIDGE_ENGINE_VERBOSE_BASE_LEVEL,LOGHEADER h,##a)
#define VERBOSE2(h,a...) xverboseN(BRIDGE_ENGINE_VERBOSE_BASE_LEVEL + 1,LOGHEADER h,##a)
#define VERBOSE3(h,a...) xverboseN(BRIDGE_ENGINE_VERBOSE_BASE_LEVEL + 2,LOGHEADER h,##a)

#define DEBUG(h,a...) xdebugN(BRIDGE_ENGINE_DEBUG_BASE_LEVEL,LOGHEADER h,##a)
#define DEBUG2(h,a...) xdebugN(BRIDGE_ENGINE_DEBUG_BASE_LEVEL + 1,LOGHEADER h,##a)
#define DEBUG3(h,a...) xdebugN(BRIDGE_ENGINE_DEBUG_BASE_LEVEL + 2,LOGHEADER h,##a)

#define ERROR VERBOSE

#define INIT_DEBUG_MARK()    DEBUG("%s : entering",function_name)
#define EXIT_DEBUG_MARK(a)   DEBUG("%s : exiting with status %d",function_name,a)

#define INIT_DEBUG2_MARK()   DEBUG2("%s : entering",function_name)
#define EXIT_DEBUG2_MARK(a)  DEBUG2("%s : exiting with status %d",function_name,a)

#define INIT_DEBUG3_MARK()   DEBUG3("%s : entering",function_name)
#define EXIT_DEBUG3_MARK(a)  DEBUG3("%s : exiting with status %d",function_name,a)

char* default_bridge_conf_file = BRIDGEDAPI_CONF ;

int
bridge_engine_free_contents(bridge_engine_t* engine){
  int fstatus=-1;

  xfree(engine->primary_address);
  xfree(engine->primary_port);

  xfree(engine->secondary_address);
  xfree(engine->secondary_port);

  xfree(engine->logfile);
  xfree(engine->debugfile);

  engine->timeout=0;

  engine->loglevel=0;
  engine->debuglevel=0;

  fstatus=0;

  return fstatus;
}

int
bridge_engine_init(bridge_engine_t* engine,
		   char* primary_address,
		   char* primary_port,
		   char* secondary_address,
		   char* secondary_port,
		   time_t timeout,
		   char* logfile,
		   int loglevel,
		   char* debugfile,
		   int debuglevel){
  int fstatus=-1;
  char* function_name="bridge_engine_init";
  
  if(
     primary_address == NULL ||
     primary_port == NULL ||

     secondary_address == NULL ||
     secondary_port == NULL ||

     logfile == NULL ||
     debugfile == NULL
     )
    {
      ERROR("all required input fields are not valid");
      return fstatus;
    }

  /* initialize engine value */
  engine->primary_address=strdup(primary_address);
  engine->primary_port=strdup(primary_port);

  engine->secondary_address=strdup(secondary_address);
  engine->secondary_port=strdup(secondary_port);

  engine->timeout=timeout;

  engine->logfile=strdup(logfile);
  engine->loglevel=loglevel;

  engine->debugfile=strdup(debugfile);
  engine->debuglevel=debuglevel;

  if(
     engine->primary_address == NULL ||
     engine->primary_port == NULL ||

     engine->secondary_address == NULL ||
     engine->secondary_port == NULL ||

     engine->logfile == NULL ||
     engine->debugfile == NULL){

    ERROR("unable to init bridge engine because of memory allocation pb");
    bridge_engine_free_contents(engine);
  }
  else{
    VERBOSE2("engine %s is '%s'","primary daemon address",engine->primary_address);
    VERBOSE2("engine %s is %s","primary daemon port",engine->primary_port);

    VERBOSE2("engine %s is '%s'","secondary daemon address",engine->secondary_address);
    VERBOSE2("engine %s is %s","secondary daemon port",engine->secondary_port);

    VERBOSE2("engine %s is %u","timeout",engine->timeout);

    VERBOSE2("engine %s is %s","logfile",engine->logfile);
    VERBOSE2("engine %s is %d","loglevel",engine->loglevel);
    VERBOSE2("engine %s is %s","debugfile",engine->debugfile);
    VERBOSE2("engine %s is %d","debuglevel",engine->debuglevel);

    fstatus=0;
  }
  
  return fstatus;
}

int
bridge_engine_init_from_config_file(bridge_engine_t* engine,char* conf_file){
  int fstatus=-1;

  char* function_name="bridge_engine_init";

  config_file_t config;
  int block_nb;

  char* loglevel_string;
  char* debuglevel_string;
  char* timeout_string;

  char* worker_string;
  char* queue_string;

  int i;

  /* parse configuration file */
  if ( conf_file==NULL ){
    conf_file = getenv("BRIDGEDAPI_CONF") ;
  }
  if ( conf_file==NULL ){
    conf_file = default_bridge_conf_file ;
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
	  
	if(strncmp("bridgedapi",block_name,10)==0){
	  
	  
	  VERBOSE("initializing engine from 'bridgedapi' block of file %s",conf_file);

	  if((loglevel_string=config_GetKeyValueByName(config,i,"loglevel")) == NULL){
	    loglevel_string="1";
	  }
	  if((debuglevel_string=config_GetKeyValueByName(config,i,"debuglevel")) == NULL){
	    debuglevel_string="1";
	  }
	  if((timeout_string=config_GetKeyValueByName(config,i,"timeout")) == NULL){
	    timeout_string="30";
	  }

	  fstatus=bridge_engine_init(engine,
				     config_GetKeyValueByName(config,i,"primary_address"),
				     config_GetKeyValueByName(config,i,"primary_port"),
				     config_GetKeyValueByName(config,i,"secondary_address"),
				     config_GetKeyValueByName(config,i,"secondary_port"),
				     strtol(timeout_string,NULL,10)*1000,
				     config_GetKeyValueByName(config,i,"logfile"),
				     strtol(loglevel_string,NULL,10),
				     config_GetKeyValueByName(config,i,"debugfile"),
				     strtol(debuglevel_string,NULL,10)
				     );	   

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
	ERROR("unable to get configuration block 'bridgedapi' from config file %s",
	      conf_file);
      }
      
    }
      
    /* free config file */
    config_Free(config);
    
  }

  return fstatus;
}
