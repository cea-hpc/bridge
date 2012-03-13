/**
 * \file bridge_rus.c
 * \author M. Hautreux
 * \date 18/06/2008
 */
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <pthread.h>

#include <string.h>

#include "bridged/bridge_rus.h"

#include "xternal/xlogger.h"

#include "confparse/config_parsing.h"
extern char extern_errormsg[1024];

#define xfree(a) if(a!=NULL){free(a);a=NULL;}

#define LOGHEADER "bridge_rus: "

#ifndef BRIDGE_RUS_VERBOSE_BASE_LEVEL
#define BRIDGE_RUS_VERBOSE_BASE_LEVEL 5
#endif

#ifndef BRIDGE_RUS_DEBUG_BASE_LEVE
#define BRIDGE_RUS_DEBUG_BASE_LEVEL   5
#endif

#define VERBOSE(h,a...) xverboseN(BRIDGE_RUS_VERBOSE_BASE_LEVEL,LOGHEADER h,##a)
#define VERBOSE2(h,a...) xverboseN(BRIDGE_RUS_VERBOSE_BASE_LEVEL + 1,LOGHEADER h,##a)
#define VERBOSE3(h,a...) xverboseN(BRIDGE_RUS_VERBOSE_BASE_LEVEL + 2,LOGHEADER h,##a)

#define DEBUG(h,a...) xdebugN(BRIDGE_RUS_DEBUG_BASE_LEVEL,LOGHEADER h,##a)
#define DEBUG2(h,a...) xdebugN(BRIDGE_RUS_DEBUG_BASE_LEVEL + 1,LOGHEADER h,##a)
#define DEBUG3(h,a...) xdebugN(BRIDGE_RUS_DEBUG_BASE_LEVEL + 2,LOGHEADER h,##a)

#define ERROR VERBOSE

#define INIT_DEBUG_MARK()    DEBUG("%s : entering",function_name)
#define EXIT_DEBUG_MARK(a)   DEBUG("%s : exiting with status %d",function_name,a)

#define INIT_DEBUG2_MARK()   DEBUG2("%s : entering",function_name)
#define EXIT_DEBUG2_MARK(a)  DEBUG2("%s : exiting with status %d",function_name,a)

#define INIT_DEBUG3_MARK()   DEBUG3("%s : entering",function_name)
#define EXIT_DEBUG3_MARK(a)  DEBUG3("%s : exiting with status %d",function_name,a)

/* popen doesn't seem to be thread safe due to its internal execl call */
/* this one doesn't use execl and so works better */
FILE* mypopen(char* command,char*mode) {

  FILE* pfile = NULL;
  int fd[2];

  int ntok=0;
  char* p;
  char* wp;
  char* token;
  char* tokens[100];

  p=strdup(command);
  if ( p == NULL ) {
    return NULL;
  }

  token=strtok_r(p," \t",&wp);
  tokens[ntok]=token;
  ntok++;
  while ( token != NULL ) {
    token = strtok_r(NULL," \t",&wp);
    tokens[ntok]=token;
    ntok++;
    if ( ntok == 99 ) {
      VERBOSE("mypopen command has more than 99 parameters, skipping the end");
      tokens[ntok]=NULL;
      break;
    }
  }

  if ( pipe(fd) == 0 ) {

    switch ( fork() ) {

    case -1 :
      close(fd[0]);
      close(fd[1]);
      break;
    
    case 0 :
      close(fd[0]);
      dup2(fd[1],1);
      execv(tokens[0],tokens);
      exit(EXIT_SUCCESS);
      break;

    default :
      close(fd[1]);
      pfile=fdopen(fd[0],mode);

    }

  }

  free(p);
  return pfile;
}

int
bridge_rus_record_init(bridge_rus_record_t* record,char* id)
{
  int retval = BRIDGE_RUS_ERROR;
  char* function_name="bridge_rus_record_init";
  INIT_DEBUG2_MARK();

  record->used_time=0;
  record->usable_time=0;
  memset(record->id,0,BRIDGE_RUS_MAX_ID_LENGTH);
  if ( id == NULL ) {
    retval = BRIDGE_RUS_ERROR_BAD_ID ;
  }
  else {
    strncpy(record->id,id,BRIDGE_RUS_MAX_ID_LENGTH);
    retval = 0 ;
  }

 done:
  EXIT_DEBUG2_MARK(retval);
  return retval;
}

int
bridge_rus_record_free_contents(bridge_rus_record_t* record)
{
  int retval = BRIDGE_RUS_ERROR ;
  char* function_name = "bridge_rus_record_free_contents" ;
  INIT_DEBUG2_MARK();

  record->used_time=0;
  record->usable_time=0;
  memset(record->id,0,BRIDGE_RUS_MAX_ID_LENGTH);
  retval = BRIDGE_RUS_SUCCESS ;

  EXIT_DEBUG2_MARK(retval);
  return retval;
}

int
bridge_rus_mgr_init(bridge_rus_mgr_t* rus,char* conf_file)
{
  int retval = BRIDGE_RUS_ERROR ;
  char* function_name="bridge_rus_mgr_init";
  INIT_DEBUG2_MARK();

  config_file_t config;
  int block_nb;
  char* conf_block_name="rus_mgr";

  char* rus_mgr_cmd_str=NULL;
  char* rus_mgr_refresh_str=NULL;
  char* rus_mgr_protection_str=NULL;

  int i;

  rus->refresh_interval=60;
  rus->synchro_command=NULL;
  rus->config_file=NULL;

  /* manager init using conf file */
  if ( conf_file==NULL ) {
    ERROR("unable to init manager : no config file defined");
    retval = BRIDGE_RUS_ERROR_NO_CONF_FILE ;
  }
  else {

    /* parse configuration file */
    config = config_ParseFile(conf_file);
    if(!config){
      ERROR("unable to parse configuration file %s : %s",conf_file,extern_errormsg);
      retval = BRIDGE_RUS_ERROR_BAD_CONF_FILE ;
    }
    else{
      /* get conf blocks quantity */
      block_nb=config_GetNbBlocks(config);
      if(block_nb<=0){
	ERROR("unable to get configuration blocks from config file %s : %s",
	      conf_file,extern_errormsg);
	retval = BRIDGE_RUS_ERROR_BAD_CONF_FILE ;
      }
      else{

	/* look for relevants block and add contents to engine conf */
	for (i=0;i<block_nb;i++){
	  char* block_name;
	  block_name = config_GetBlockName(config,i);

	  /* find rus mgr conf block name */
	  if(strncmp(conf_block_name,block_name,strlen(conf_block_name))==0){
	    VERBOSE2("initializing manager from '%s' block of file %s",conf_block_name,conf_file);
	    retval = BRIDGE_RUS_SUCCESS;

	    rus->config_file=strdup(conf_file);
	    rus_mgr_cmd_str=config_GetKeyValueByName(config,i,"synchro_command");
	    if ( rus_mgr_cmd_str != NULL ) {
	      rus->synchro_command=(void*)strdup(rus_mgr_cmd_str);
	    }
	    rus_mgr_refresh_str=config_GetKeyValueByName(config,i,"refresh_interval");
	    if ( rus_mgr_refresh_str != NULL ) {
	      errno=0;
	      rus->refresh_interval=strtol(rus_mgr_refresh_str,NULL,10);
	      if(errno!=0){
		rus->refresh_interval=60;
		VERBOSE2("invalid refresh rate for manager, using the default one '%d'",
			rus->refresh_interval);
	      }
	    }
	    else {
	      VERBOSE2("invalid refresh rate for manager, using the default one '%d'",
		      rus->refresh_interval);
	    }
	    rus_mgr_protection_str=config_GetKeyValueByName(config,i,"protection_time");
	    if ( rus_mgr_protection_str != NULL ) {
	      errno=0;
	      rus->protection_time=strtol(rus_mgr_protection_str,NULL,10);
	      if(errno!=0){
		rus->protection_time=0;
		VERBOSE2("invalid protection time for manager, using the default one '%d'",
			rus->protection_time);
	      }
	    }
	    else {
	      VERBOSE2("invalid protection time for manager, using the default one '%d'",
		      rus->protection_time);
	    }
	    /* library init */
	    if ( rus->config_file != NULL && 
		 rus->synchro_command != NULL ) {
	      retval=xlibrary_init(&(rus->library),BRIDGE_RUS_MGR_DEFAULT_LIBRARY_SIZE,
				   sizeof(bridge_rus_record_t));
	      if (retval ) {
		ERROR("unable to init library");
		retval = BRIDGE_RUS_ERROR_LIBRARY_INIT ;
	      }
	      else {
		VERBOSE2("library successfully initialized");
		retval = BRIDGE_RUS_SUCCESS ;
	      }
	    }
	    else {
	      retval = BRIDGE_RUS_ERROR_BAD_CONF_FILE ;
	    }
	    /*_*/ /* library init */

	  } /*_*/ /* rus mgr config block */
	
	} /*_*/ /* look for relevant blocks */
      
      }
      /* free config file */
      config_Free(config);
    } /*_*/ /* parse conf file */
      
      
    /* check that configuration parsing succeed */
    if ( retval != BRIDGE_RUS_SUCCESS ) {
      ERROR("unable to init manager using config file '%s'",conf_file);
      retval = BRIDGE_RUS_ERROR_BAD_CONF_FILE ;
    }
    else {
      VERBOSE("initialization succeed",conf_file);
      VERBOSE2("manager synchro cmd is '%s'",(char*)rus->synchro_command);
      VERBOSE2("manager refresh interval is '%d'",rus->refresh_interval);
      VERBOSE2("manager protection time is '%d'",rus->protection_time);
    }

  } /*_*/ /* manager init */

  
  EXIT_DEBUG2_MARK(retval);
  return retval;
}

int
bridge_rus_mgr_free_contents(bridge_rus_mgr_t* rus)
{
  int retval = BRIDGE_RUS_ERROR ;
  char* function_name="bridge_rus_mgr_free_contents";
  INIT_DEBUG2_MARK();

  /* library destruction */
  xlibrary_free_contents(&(rus->library));

  xfree(rus->synchro_command);
  xfree(rus->config_file);

  retval = BRIDGE_RUS_SUCCESS ;

  EXIT_DEBUG2_MARK(retval);
  return retval;
}

int
bridge_rus_mgr_synchronise(bridge_rus_mgr_t* rus,unsigned long * items_nb)
{
  int retval = BRIDGE_RUS_ERROR ;
  char* function_name="bridge_rus_mgr_synchronise";
  INIT_DEBUG_MARK();

  FILE* output;

  char line[128];

  char id[128];
  time_t used_time;
  time_t usable_time;

  int i;
  time_t current_time;
  xlibrary_item_t* pitem;

  char reference[128];

  unsigned long records_nb=0;
  unsigned long updated_records_nb=0;
  unsigned long removed_records_nb=0;

  bridge_rus_record_t record;

  *items_nb=0;

  /* check synchro command validity */
  if ( rus->synchro_command == NULL ) {
    ERROR("no synchronisation command defined");
    retval = BRIDGE_RUS_ERROR_SYNC_CMD_INVALID ;
  }
  else {

    /* open execution pipe */
    output=mypopen(rus->synchro_command,"r");
    if(output==NULL){
      ERROR("unable to synchronise data : popen error");
      retval = BRIDGE_RUS_ERROR_SYNC_CMD_FAILED ;
    }
    else{
      /* read each line */
      while (fgets(line,127,output)!=NULL) {
	/* extract id, used time and usable time */
	used_time=0;
	usable_time=0;
	if(sscanf(line,"%s %u %u\n",id,&used_time,&usable_time)!=3){
	  size_t l=strnlen(line,127);
	  line[l-1]='\0';
	  ERROR("error during synchronisation, skipping invalid line '%s'",
		line);
	}
	else{
	      
	  /* update stats tables */
	  retval=bridge_rus_record_init(&record,id);
	  if ( retval!=0 ) {
	    ERROR("unable to init record usage for reference '%s'",id);
	  }
	  else {
	    record.used_time = used_time + rus->protection_time ;
	    record.usable_time = usable_time;
		
	    retval=xlibrary_add_item(&(rus->library),id,&record,
				     sizeof(bridge_rus_record_t));
	    if(retval){
	      ERROR("unable to add item referenced by '%s' to library",id);
	    }
	    else{
	      VERBOSE2("item referenced by '%s' successfully added to library",
		       id);
	      updated_records_nb++;
	    }
		
	    bridge_rus_record_free_contents(&record);
	  }
	      
	} /*_*/ /* data extraction from line */

      } /*_*/ /* read each line */

      /* close execution pipe */
      fclose(output);
	  
      retval = BRIDGE_RUS_SUCCESS ;
	
    }
    /*_*/ /* open execution pipe */


    /* remove old records from library */
    time(&current_time);
    retval=xlibrary_lock(&(rus->library));
    if(retval){
      ERROR("unable to lock library to remove old records");
    }
    else{
      /* update library linear index */
      retval=xlibrary_update_index(&(rus->library));
      if(retval){
	ERROR("unable to update library index");
      }
      else{
	/* walk through the index to clean oldest items */
	for(i=0;i<rus->library.item_nb;i++){
	  pitem=(xlibrary_item_t*)((xlibrary_item_t**)rus->library.index)[i];
	  if ( (current_time - pitem->timestamp) > 
	       3 * rus->refresh_interval ) {
	    VERBOSE2("item referenced by '%s' is no longer active",
		     pitem->reference);
	    strncpy(reference,pitem->reference,BRIDGE_RUS_MAX_ID_LENGTH);
	    retval=xlibrary_remove_item_nolock(&(rus->library),reference);
	    if(retval){
	      ERROR("unable to remove old item referenced by '%s' from library",
		    reference);
	    }
	    else{
	      VERBOSE2("old item referenced by '%s' successfully removed from library",
		       reference);
	      removed_records_nb++;
	    }
	  }
	  else {
	    VERBOSE3("item referenced by '%s' is still active",
		     pitem->reference);	    
	    records_nb++;
	  }
	} /*_*/ /*walk through the index */

      } /*_*/ /* update library linear index */

      xlibrary_unlock(&(rus->library));
    } /*_*/ /* remove old records */
    
    /* log stats */
    VERBOSE("sync : %u items - %u added - %u removed",
	    records_nb,updated_records_nb,removed_records_nb);

    *items_nb=records_nb;

  }
  /*_*/ /* synchro command validity check */

  EXIT_DEBUG_MARK(retval);
  return retval;
}

int
bridge_rus_mgr_get_record(bridge_rus_mgr_t* rus,bridge_rus_record_t* record)
{
  int retval = BRIDGE_RUS_ERROR ;
  char* function_name="bridge_rus_mgr_get_record";
  INIT_DEBUG_MARK();

  /* try to get the item from library */
  retval=xlibrary_get_item(&(rus->library),record->id,record,
			   sizeof(bridge_rus_record_t));
  if(retval){
    ERROR("unable to get item referenced by '%s' from library",record->id);
    retval = BRIDGE_RUS_ERROR_ITEM_NOT_FOUND ;
  }
  else{
    VERBOSE("item referenced by '%s' successfully got from library",record->id);
    retval = BRIDGE_RUS_SUCCESS ;
  }

  EXIT_DEBUG_MARK(retval);
  return retval;
}
