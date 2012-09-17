/*****************************************************************************\
 *  lib/bridge/bridge_manager.c - 
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

#include <dlfcn.h>

#include <time.h>
#include <math.h>

#include <string.h>

#include "confparse/config_parsing.h"

#include "bridge.h"

#include "xternal/xlogger.h"
#define DEBUG_LOGGER xdebug
#define DEBUG2_LOGGER xdebug2
#define DEBUG3_LOGGER xdebug3
#define ERROR_LOGGER xerror

char* bridge_version(){

  return BRIDGE_VERSION ;
    
}

/*!
 * \ingroup BRIDGE_BATCH_SYSTEM
 * \brief Load batch system
 *
 * \param p_manager pointer on a bridge manager structure
 * \param p_batch_system pointer on a batch system structure to initialize
 * \param name name of this batch system
 * \param library URI of the library to use to load this batch system
 *
 * \retval  0 operation successfully done
 * \retval  1 operation failed because of undefined parameter(s)
 * \retval -1 unable to dynamically open library
 * \retval -2 all functions can't be retrieved from library
*/
int bridge_load_batch_system(bridge_manager_t* p_manager,
			     bridge_batch_system_t* p_batch_system,
			     char* name,
			     char* lib){

  int fstatus=1;

  size_t name_length;
  size_t lib_length;

  /* Check that lib name and file are present */
  if(name==NULL || lib==NULL)
    return fstatus;

  /* Set batch system plugin library according to manager configuration */
  if(p_manager->plugins_dir!=NULL){
    lib_length=strlen(p_manager->plugins_dir)+strlen(lib)+2;
    if(lib[0]!='/'){
      p_batch_system->lib=(char*)malloc(lib_length*sizeof(char));
      if(p_batch_system->lib!=NULL)
	snprintf(p_batch_system->lib,lib_length,"%s/%s",
		 p_manager->plugins_dir,lib);
    }
    else{
      p_batch_system->lib=strdup(lib);
    }
  }
  else{
    p_batch_system->lib=strdup(lib);
  }

  /* Initialize batch system wrapper */
  if(p_batch_system->lib!=NULL){

    /* Set batch system name */
    p_batch_system->name=strdup(name);
    if(p_batch_system->name!=NULL){
      /* Open batch system plugin library */
      p_batch_system->lib_handle=dlopen(p_batch_system->lib,RTLD_LAZY | RTLD_GLOBAL);
      if(p_batch_system->lib_handle!=NULL){
	/* load manager get batch id function */
	p_batch_system->get_batch_id=
	  (int (*)(char**)) dlsym(p_batch_system->lib_handle,"get_batch_id");
	if(p_batch_system->get_batch_id==NULL){
	  DEBUG_LOGGER("Unable to load function %s from library %s",
		       "get_batch_id",p_batch_system->lib);
	}
	else
	  DEBUG2_LOGGER("function '%s' is now loaded from library '%s'",
			"get_batch_id",p_batch_system->lib);
	/* load manager init function */
	p_batch_system->init_batch_manager=
	  (int (*)(bridge_batch_manager_t*)) dlsym(p_batch_system->lib_handle,"init_batch_manager");
	if(p_batch_system->init_batch_manager==NULL){
	  DEBUG_LOGGER("Unable to load function %s from library %s",
		       "init_batch_manager",p_batch_system->lib);
	}
	else
	  DEBUG2_LOGGER("function '%s' is now loaded from library '%s'",
			"init_batch_manager",p_batch_system->lib);
	/* load manager clean function */
	p_batch_system->clean_batch_manager=
	  (int (*)(bridge_batch_manager_t*)) dlsym(p_batch_system->lib_handle,
						       "clean_batch_manager");
	if(p_batch_system->clean_batch_manager==NULL){
	  DEBUG_LOGGER("Unable to load function %s from library %s",
		       "clean_batch_manager",p_batch_system->lib);
	}
	else
	  DEBUG2_LOGGER("function '%s' is now loaded from library '%s'",
		       "clean_batch_manager",p_batch_system->lib);
	/* load session init function */
	p_batch_system->init_batch_session=
	  (int (*)(bridge_batch_manager_t*,bridge_batch_session_t*))
	  dlsym(p_batch_system->lib_handle,"init_batch_session");
	if(p_batch_system->init_batch_session==NULL){
	  DEBUG_LOGGER("Unable to load function %s from library %s",
		       "init_batch_session",p_batch_system->lib);
	}
	else
	  DEBUG2_LOGGER("function '%s' is now loaded from library '%s'",
		       "init_batch_session",p_batch_system->lib);
	/* load session clean function */
	p_batch_system->clean_batch_session=
	  (int (*)(bridge_batch_manager_t*,bridge_batch_session_t*))
	  dlsym(p_batch_system->lib_handle,"clean_batch_session");
	if(p_batch_system->clean_batch_session==NULL){
	  DEBUG_LOGGER("Unable to load function %s from library %s",
		       "clean_batch_session",p_batch_system->lib);
	}
	else
	  DEBUG2_LOGGER("function '%s' is now loaded from library '%s'",
		       "clean_batch_session",p_batch_system->lib);
	/* load session get (current) function */
	p_batch_system->get_batch_sessions=
	  (int (*)(bridge_batch_manager_t*,
		   bridge_batch_session_t **,
		   int*,
		   char*,
		   char*,
		   char*,
		   char*,
		   char*,
		   int))
	  dlsym(p_batch_system->lib_handle,"get_batch_sessions");
	if(p_batch_system->get_batch_sessions==NULL){
	  DEBUG_LOGGER("Unable to load function %s from library %s",
		       "get_batch_sessions",p_batch_system->lib);
	}
	else
	  DEBUG2_LOGGER("function '%s' is now loaded from library '%s'",
		       "get_batch_sessions",p_batch_system->lib);
	/* load session get terminated function*/
	p_batch_system->get_terminated_batch_sessions=
	  (int (*)(bridge_batch_manager_t*,
		   bridge_batch_session_t **,
		   int*,
		   char*,
		   char*,
		   char*,
		   char*,
		   char*,
		   int,
		   time_t,time_t))
	  dlsym(p_batch_system->lib_handle,"get_terminated_batch_sessions");
	if(p_batch_system->get_terminated_batch_sessions==NULL){
	  DEBUG_LOGGER("Unable to load function %s from library %s",
		       "get_terminated_batch_sessions",p_batch_system->lib);
	}
	else
	  DEBUG2_LOGGER("function '%s' is now loaded from library '%s'",
		       "get_terminated_batch_sessions",p_batch_system->lib);
	/* load queue init function*/
	p_batch_system->init_batch_queue=
	  (int (*)(bridge_batch_manager_t*,bridge_batch_queue_t*))
	  dlsym(p_batch_system->lib_handle,"init_batch_queue");
	if(p_batch_system->init_batch_queue==NULL){
	  DEBUG_LOGGER("Unable to load function %s from library %s",
		       "init_batch_queue",p_batch_system->lib);
	}
	else
	  DEBUG2_LOGGER("function '%s' is now loaded from library '%s'",
		       "init_batch_queue",p_batch_system->lib);
	/* load queue clean function */
	p_batch_system->clean_batch_queue=
	  (int (*)(bridge_batch_manager_t*,bridge_batch_queue_t*))
	  dlsym(p_batch_system->lib_handle,"clean_batch_queue");
	if(p_batch_system->clean_batch_queue==NULL){
	  DEBUG_LOGGER("Unable to load function %s from library %s",
		       "clean_batch_queue",p_batch_system->lib);
	}
	else
	  DEBUG2_LOGGER("function '%s' is now loaded from library '%s'",
			"clean_batch_queue",p_batch_system->lib);
	/* load queue get function */
	p_batch_system->get_batch_queues=
	  (int (*)(bridge_batch_manager_t*,
		   bridge_batch_queue_t**,
		   int*,
		   char*))
	  dlsym(p_batch_system->lib_handle,"get_batch_queues");
	if(p_batch_system->get_batch_queues==NULL){
	  DEBUG_LOGGER("Unable to load function %s from library %s",
		       "get_batch_queues",p_batch_system->lib);
	}
	else
	  DEBUG2_LOGGER("function '%s' is now loaded from library '%s'",
		       "get_batch_queues",p_batch_system->lib);
	/* load node init function*/
	p_batch_system->init_batch_node=
	  (int (*)(bridge_batch_manager_t*,bridge_batch_node_t*))
	  dlsym(p_batch_system->lib_handle,"init_batch_node");
	if(p_batch_system->init_batch_node==NULL){
	  DEBUG_LOGGER("Unable to load function %s from library %s",
		       "init_batch_node",p_batch_system->lib);
	}
	else
	  DEBUG2_LOGGER("function '%s' is now loaded from library '%s'",
		       "init_batch_node",p_batch_system->lib);
	/* load node clean function */
	p_batch_system->clean_batch_node=
	  (int (*)(bridge_batch_manager_t*,bridge_batch_node_t*))
	  dlsym(p_batch_system->lib_handle,"clean_batch_node");
	if(p_batch_system->clean_batch_node==NULL){
	  DEBUG_LOGGER("Unable to load function %s from library %s",
		       "clean_batch_node",p_batch_system->lib);
	}
	else
	  DEBUG2_LOGGER("function '%s' is now loaded from library '%s'",
		       "clean_batch_node",p_batch_system->lib);
	/* load node get function*/
	p_batch_system->get_batch_nodes=
	  (int (*)(bridge_batch_manager_t*,
		   bridge_batch_node_t**,
		   int*,
		   char*))
	  dlsym(p_batch_system->lib_handle,"get_batch_nodes");
	if(p_batch_system->get_batch_nodes==NULL){
	  DEBUG_LOGGER("Unable to load function %s from library %s",
		       "get_batch_nodes",p_batch_system->lib);
	}
	else
	  DEBUG2_LOGGER("function '%s' is now loaded from library '%s'",
		       "get_batch_nodes",p_batch_system->lib);
	/* Check that all functions were successfully loaded */
	if(p_batch_system->get_batch_id!=NULL
	   && p_batch_system->init_batch_session!=NULL
	   && p_batch_system->clean_batch_session!=NULL
	   && p_batch_system->get_batch_sessions!=NULL
	   && p_batch_system->get_terminated_batch_sessions!=NULL
	   && p_batch_system->init_batch_queue!=NULL
	   && p_batch_system->clean_batch_queue!=NULL
	   && p_batch_system->get_batch_queues!=NULL
	   && p_batch_system->init_batch_node!=NULL
	   && p_batch_system->clean_batch_node!=NULL
	   && p_batch_system->get_batch_nodes!=NULL)
	  fstatus=0;
	else
	  fstatus=-2;
      }
      else{
	DEBUG_LOGGER("Unable to load '%s' batch system using library '%s' : %s",
		     p_batch_system->name,p_batch_system->lib,dlerror());
	fstatus=-1;
      }

    }

  }

  return fstatus;
}

/*!
 * \ingroup BRIDGE_BATCH_SYSTEM
 * \brief Unload batch system
 *
 * \param p_manager pointer on a bridge manager structure
 * \param p_batch_system pointer on a batch system structure to clean
 *
 * \retval  0 operation successfully done
 * \retval -1 operation failed
*/
int bridge_unload_batch_system(bridge_manager_t* p_manager,
			       bridge_batch_system_t* p_batch_system){

  int fstatus=-1;
  
  if(p_batch_system->lib_handle!=NULL)
    dlclose(p_batch_system->lib_handle);

  if(p_batch_system->name!=NULL){
    free(p_batch_system->name);
    p_batch_system->name=NULL;
  }
  if(p_batch_system->lib!=NULL){
    free(p_batch_system->lib);
    p_batch_system->lib=NULL;
  }

  fstatus=0;

  return fstatus;
}


/*!
 * \ingroup BRIDGE_RM_SYSTEM
 * \brief Load rm (resource manager) system
 *
 * \param p_manager pointer on a bridge manager structure
 * \param p_rm_system pointer on a rm system structure to initialize
 * \param name name of this rm system
 * \param library URI of the library to use to load this rm system
 *
 * \retval  0 operation successfully done
 * \retval  1 operation failed because of undefined parameter(s)
 * \retval -1 unable to dynamically open library
 * \retval -2 all functions can't be retrieved from library
*/
int bridge_load_rm_system(bridge_manager_t* p_manager,
			  bridge_rm_system_t* p_rm_system,
			  char* name,
			  char* lib){

  int fstatus=1;
  
  size_t name_length;
  size_t lib_length;
  
  /* Check that lib name and file are present */
  if(name==NULL || lib==NULL)
    return fstatus;

  /* Set resource manager plugin library according to manager configuration */
  if(p_manager->plugins_dir!=NULL){
    lib_length=strlen(p_manager->plugins_dir)+strlen(lib)+2;
    if(lib[0]!='/'){
      p_rm_system->lib=(char*)malloc(lib_length*sizeof(char));
      if(p_rm_system->lib!=NULL)
	snprintf(p_rm_system->lib,lib_length,"%s/%s",
		 p_manager->plugins_dir,lib);
    }
    else{
      p_rm_system->lib=strdup(lib);
    }
  }
  else{
    p_rm_system->lib=strdup(lib);
  }

  /* Initialize resource manager wrapper */
  if(p_rm_system->lib!=NULL){

    /* Set batch system name */
    p_rm_system->name=strdup(name);
    if(p_rm_system->name!=NULL){
      /* Open resource manager plugin library */
      p_rm_system->lib_handle=dlopen(p_rm_system->lib,RTLD_LAZY | RTLD_GLOBAL);
      if(p_rm_system->lib_handle!=NULL){
	/* load rm manager get id function */
	p_rm_system->get_rm_id=
	  (int (*)(char**)) dlsym(p_rm_system->lib_handle,"get_rm_id");
	if(p_rm_system->get_rm_id==NULL){
	  DEBUG_LOGGER("Unable to load function %s from library %s",
		       "get_rm_id",p_rm_system->lib);
	}
	else
	  DEBUG2_LOGGER("function '%s' is now loaded from library '%s'",
			"get_rm_id",p_rm_system->lib);
	/* load rm manager init function */
	p_rm_system->init_rm_manager=
	  (int (*)(bridge_rm_manager_t*)) dlsym(p_rm_system->lib_handle,"init_rm_manager");
	if(p_rm_system->init_rm_manager==NULL){
	  DEBUG_LOGGER("Unable to load function %s from library %s",
		       "init_rm_manager",p_rm_system->lib);
	}
	else
	  DEBUG2_LOGGER("function '%s' is now loaded from library '%s'",
			"init_rm_manager",p_rm_system->lib);
	/* load rm manager clean function */
	p_rm_system->clean_rm_manager=
	  (int (*)(bridge_rm_manager_t*)) dlsym(p_rm_system->lib_handle,
						       "clean_rm_manager");
	if(p_rm_system->clean_rm_manager==NULL){
	  DEBUG_LOGGER("Unable to load function %s from library %s",
		       "clean_rm_manager",p_rm_system->lib);
	}
	else
	  DEBUG2_LOGGER("function '%s' is now loaded from library '%s'",
		       "clean_rm_manager",p_rm_system->lib);
	/* load rm allocation init function */
	p_rm_system->init_rm_allocation=
	  (int (*)(bridge_rm_manager_t*,bridge_rm_allocation_t*))
	  dlsym(p_rm_system->lib_handle,"init_rm_allocation");
	if(p_rm_system->init_rm_allocation==NULL){
	  DEBUG_LOGGER("Unable to load function %s from library %s",
		       "init_rm_allocation",p_rm_system->lib);
	}
	else
	  DEBUG2_LOGGER("function '%s' is now loaded from library '%s'",
		       "init_rm_allocation",p_rm_system->lib);
	/* load rm allocation clean function */
	p_rm_system->clean_rm_allocation=
	  (int (*)(bridge_rm_manager_t*,bridge_rm_allocation_t*))
	  dlsym(p_rm_system->lib_handle,"clean_rm_allocation");
	if(p_rm_system->clean_rm_allocation==NULL){
	  DEBUG_LOGGER("Unable to load function %s from library %s",
		       "clean_rm_allocation",p_rm_system->lib);
	}
	else
	  DEBUG2_LOGGER("function '%s' is now loaded from library '%s'",
		       "clean_rm_allocation",p_rm_system->lib);
	/* load rm allocation get (current) function */
	p_rm_system->get_rm_allocations=
	  (int (*)(bridge_rm_manager_t*,
		   bridge_rm_allocation_t **,
		   int*,
		   char*,
		   char*,
		   char*,
		   char*,
		   char*))
	  dlsym(p_rm_system->lib_handle,"get_rm_allocations");
	if(p_rm_system->get_rm_allocations==NULL){
	  DEBUG_LOGGER("Unable to load function %s from library %s",
		       "get_rm_allocations",p_rm_system->lib);
	}
	else
	  DEBUG2_LOGGER("function '%s' is now loaded from library '%s'",
		       "get_rm_allocations",p_rm_system->lib);
	/* load rm allocation get terminated function */
	p_rm_system->get_terminated_rm_allocations=
	  (int (*)(bridge_rm_manager_t*,
		   bridge_rm_allocation_t **,
		   int*,
		   char*,
		   char*,
		   char*,
		   char*,
		   char*,
		   time_t,time_t))
	  dlsym(p_rm_system->lib_handle,"get_terminated_rm_allocations");
	if(p_rm_system->get_terminated_rm_allocations==NULL){
	  DEBUG_LOGGER("Unable to load function %s from library %s",
		       "get_terminated_rm_allocations",p_rm_system->lib);
	}
	else
	  DEBUG2_LOGGER("function '%s' is now loaded from library '%s'",
		       "get_terminated_rm_allocations",p_rm_system->lib);
	/* load rm partition init function */
	p_rm_system->init_rm_partition=
	  (int (*)(bridge_rm_manager_t*,bridge_rm_partition_t*))
	  dlsym(p_rm_system->lib_handle,"init_rm_partition");
	if(p_rm_system->init_rm_partition==NULL){
	  DEBUG_LOGGER("Unable to load function %s from library %s",
		       "init_rm_partition",p_rm_system->lib);
	}
	else
	  DEBUG2_LOGGER("function '%s' is now loaded from library '%s'",
		       "init_rm_partition",p_rm_system->lib);
	/* load rm partition clean function */
	p_rm_system->clean_rm_partition=
	  (int (*)(bridge_rm_manager_t*,bridge_rm_partition_t*))
	  dlsym(p_rm_system->lib_handle,"clean_rm_partition");
	if(p_rm_system->clean_rm_partition==NULL){
	  DEBUG_LOGGER("Unable to load function %s from library %s",
		       "clean_rm_partition",p_rm_system->lib);
	}
	else
	  DEBUG2_LOGGER("function '%s' is now loaded from library '%s'",
			"clean_rm_partition",p_rm_system->lib);
	/* load rm partition get function */
	p_rm_system->get_rm_partitions=
	  (int (*)(bridge_rm_manager_t*,
		   bridge_rm_partition_t**,
		   int*,
		   char*,char*,char*))
	  dlsym(p_rm_system->lib_handle,"get_rm_partitions");
	if(p_rm_system->get_rm_partitions==NULL){
	  DEBUG_LOGGER("Unable to load function %s from library %s",
		       "get_rm_partitions",p_rm_system->lib);
	}
	else
	  DEBUG2_LOGGER("function '%s' is now loaded from library '%s'",
		       "get_rm_partitions",p_rm_system->lib);
	/* load rm node init function */
	p_rm_system->init_rm_node=
	  (int (*)(bridge_rm_manager_t*,bridge_rm_node_t*))
	  dlsym(p_rm_system->lib_handle,"init_rm_node");
	if(p_rm_system->init_rm_node==NULL){
	  DEBUG_LOGGER("Unable to load function %s from library %s",
		       "init_rm_node",p_rm_system->lib);
	}
	else
	  DEBUG2_LOGGER("function '%s' is now loaded from library '%s'",
		       "init_rm_node",p_rm_system->lib);
	/* load rm node clean function */
	p_rm_system->clean_rm_node=
	  (int (*)(bridge_rm_manager_t*,bridge_rm_node_t*))
	  dlsym(p_rm_system->lib_handle,"clean_rm_node");
	if(p_rm_system->clean_rm_node==NULL){
	  DEBUG_LOGGER("Unable to load function %s from library %s",
		       "clean_rm_node",p_rm_system->lib);
	}
	else
	  DEBUG2_LOGGER("function '%s' is now loaded from library '%s'",
		       "clean_rm_node",p_rm_system->lib);
	/* load rm node get function */
	p_rm_system->get_rm_nodes=
	  (int (*)(bridge_rm_manager_t*,
		   bridge_rm_node_t**,
		   int*,
		   char*))
	  dlsym(p_rm_system->lib_handle,"get_rm_nodes");
	if(p_rm_system->get_rm_nodes==NULL){
	  DEBUG_LOGGER("Unable to load function %s from library %s",
		       "get_rm_nodes",p_rm_system->lib);
	}
	else
	  DEBUG2_LOGGER("function '%s' is now loaded from library '%s'",
		       "get_rm_nodes",p_rm_system->lib);
	/* Check that all functions were successfully loaded */
	if(p_rm_system->get_rm_id!=NULL
	   && p_rm_system->init_rm_allocation!=NULL
	   && p_rm_system->clean_rm_allocation!=NULL
	   && p_rm_system->get_rm_allocations!=NULL
	   && p_rm_system->get_terminated_rm_allocations!=NULL
	   && p_rm_system->init_rm_partition!=NULL
	   && p_rm_system->clean_rm_partition!=NULL
	   && p_rm_system->get_rm_partitions!=NULL
	   && p_rm_system->init_rm_node!=NULL
	   && p_rm_system->clean_rm_node!=NULL
	   && p_rm_system->get_rm_nodes!=NULL)
	  fstatus=0;
	else
	  fstatus=-2;
      }
      else{
	DEBUG_LOGGER("Unable to load '%s' rm system using library '%s' : %s",
		     p_rm_system->name,p_rm_system->lib,dlerror());
	fstatus=-1;
      }

    }

  }

  return fstatus;
}

/*!
 * \ingroup BRIDGE_RM_SYSTEM
 * \brief Unload rm system
 *
 * \param p_manager pointer on a bridge manager structure
 * \param p_rm_system pointer on a rm system structure to clean
 *
 * \retval  0 operation successfully done
 * \retval -1 operation failed
*/
int bridge_unload_rm_system(bridge_manager_t* p_manager,
			    bridge_rm_system_t* p_rm_system){

  int fstatus=-1;

  if(p_rm_system->lib_handle!=NULL)
    dlclose(p_rm_system->lib_handle);

  if(p_rm_system->name!=NULL){
    free(p_rm_system->name);
    p_rm_system->name=NULL;
  }
  if(p_rm_system->lib!=NULL){
    free(p_rm_system->lib);
    p_rm_system->lib=NULL;
  }

  fstatus=0;

  return fstatus;
}

/*!
 * \ingroup BRIDGE
 * \brief Load Bridge manager from configuration file
 *
 * \param p_manager pointer on a bridge manager structure
 * \param conffile configuration file to use
 *
 * \retval  0 operation successfully done
 * \retval -1 operation failed
*/
int bridge_manager_load_conf(bridge_manager_t* p_manager,
			     char* conffile){
  int fstatus=-1;
  config_file_t config;
  int block_nb;
  int i;
  char* block_name;
  
  int value;

  char* pointer;

  /* Parse configuration file */
  config = config_ParseFile(conffile);
  if(!config){
    ERROR_LOGGER("unable to parse configuration file %s : %s",conffile,config_GetErrorMsg());
  }
  else{
    /* Read all conf blocks */
    block_nb=config_GetNbBlocks(config);
    for (i=0;i<block_nb;i++){
      block_name = config_GetBlockName(config,i);
      /* If batch system block, get its conf */
      if(strcmp("batch_system",block_name)==0){
	/* name */
	pointer=config_GetKeyValueByName(config,i,"name");
	if(pointer!=NULL){
	  p_manager->batch_system_name=strdup(pointer);
	}
	/* library */
	pointer=config_GetKeyValueByName(config,i,"plugin");
	if(pointer!=NULL){
	  p_manager->batch_system_lib=strdup(pointer);
	}
      }
      /* If resource manager block, get its conf */
      else if (strcmp("rm_system",block_name)==0){
	/* name */ 
	pointer=config_GetKeyValueByName(config,i,"name");
	if(pointer!=NULL){
	  p_manager->rm_system_name=strdup(pointer);
	}
	/* library */
	pointer=config_GetKeyValueByName(config,i,"plugin");
	if(pointer!=NULL){
	  p_manager->rm_system_lib=strdup(pointer);
	}
      }
      /* If global block, get global conf */
      else if (strcmp("global",block_name)==0){
	/* log file */
	pointer=config_GetKeyValueByName(config,i,"logfile");
	if(pointer!=NULL){
	  p_manager->logfile=fopen(pointer,"a");
	  if(p_manager->logfile!=NULL)
	    xverbose_setstream(p_manager->logfile);
	}
	/* log level */
	pointer=config_GetKeyValueByName(config,i,"loglevel");
	if(pointer!=NULL){
	  p_manager->loglevel=atoi(pointer);
	}
	/* debug file */
	pointer=config_GetKeyValueByName(config,i,"debugfile");
	if(pointer!=NULL){
	  p_manager->debugfile=fopen(pointer,"a");
	  if(p_manager->debugfile!=NULL)
	    xdebug_setstream(p_manager->debugfile);
	}
	/* debug level */
	pointer=config_GetKeyValueByName(config,i,"debuglevel");
	if(pointer!=NULL){
	  p_manager->debuglevel=atoi(pointer);
	}
	/* plugins directory */
	pointer=config_GetKeyValueByName(config,i,"plugins_dir");
	if(pointer!=NULL){
	  p_manager->plugins_dir=strdup(pointer);
	}
	/* batch system <-> rm system binding */
	pointer=config_GetKeyValueByName(config,i,"bs_rm_binding");
	if(pointer!=NULL){
	  if(strcmp("batchid",pointer)==0){
	    p_manager->bs_rm_binding=BRIDGE_BS_RM_BINDING_BATCHID;
	  }
	  else if(strcmp("exechost_sid",pointer)==0){
	    p_manager->bs_rm_binding=BRIDGE_BS_RM_BINDING_EXECHOST_SID;
	  }
	  else if(strcmp("rmid",pointer)==0){
	    p_manager->bs_rm_binding=BRIDGE_BS_RM_BINDING_RMID;
	  }
	  else
	    p_manager->bs_rm_binding=BRIDGE_BS_RM_BINDING_NONE;
	}
      }
      else
	continue;
    }
    fstatus=0;
    config_Free(config);
  }

  return fstatus;
}


/*!
 * \ingroup BRIDGE
 * \brief Initialize bridge manager
 *
 * \param p_manager pointer on a bridge manager structure
 *
 * \retval   0 operation successfully done
 * \retval  -1 internal error
 * \retval  -2 no configuration file
 * \retval  -3 invalid configuration file
 * \retval -10 batch system required but not loaded
 * \retval -20 rm system required but not loaded
 * \retval -30 batch and rm systems required but not loaded
*/
int bridge_init_manager(bridge_manager_t* p_manager){

  int fstatus=-1;
  int status;

  char* pointer;
  char* conffile;

  int batch_system_required=0;
  int rm_system_required=0;

  p_manager->version=strdup(BRIDGE_VERSION);

  p_manager->batch_system_flag=0;
  p_manager->batch_manager_flag=0;
  p_manager->rm_system_flag=0;
  p_manager->rm_manager_flag=0;

  p_manager->bs_rm_binding=BRIDGE_BS_RM_BINDING_NONE;

  p_manager->plugins_dir=NULL;
  p_manager->errorfile=stderr;

  p_manager->batch_system_name=NULL;
  p_manager->batch_system_lib=NULL;
  p_manager->rm_system_name=NULL;
  p_manager->rm_system_lib=NULL;

  p_manager->logfile=stdout;
  p_manager->debugfile=stdout;


  p_manager->loglevel=0;
  p_manager->debuglevel=0;
  p_manager->errorlevel=1;

  /* Check that bridge configuration file is not set in Environment */
  pointer=getenv(BRIDGE_CONF_FILE_ENV_VAR);
  if(pointer==NULL){
    conffile=strdup(BRIDGE_CONF);
  }
  else if(strlen(pointer)>0){
    conffile=strdup(pointer);
  }
  else{
    conffile=strdup(BRIDGE_CONF);
  }

  /* Check that conffile is defined */
  if(conffile==NULL)
    return -2;
  else{
    /* load configuration */
    if(bridge_manager_load_conf(p_manager,conffile)!=0){
      free(conffile);
      return -3;
    }
  }
  
  xverbose_setmaxlevel(p_manager->loglevel);
  xdebug_setmaxlevel(p_manager->debuglevel);
  xerror_setmaxlevel(p_manager->errorlevel);

  if(p_manager->batch_system_name!=NULL &&
     p_manager->batch_system_lib!=NULL){
    if(strncmp("none",p_manager->batch_system_name,5)!=0 &&
       strncmp("none",p_manager->batch_system_lib,5)!=0)
      batch_system_required=1;
  }
  if(batch_system_required==1){
    /* try to load batch system plugin */
    if(bridge_load_batch_system(p_manager,
				&(p_manager->batch_system),
				p_manager->batch_system_name,
				p_manager->batch_system_lib)==0){
      p_manager->batch_system_flag=1;
      /* initialize batch system */
      if(p_manager->batch_system.init_batch_manager(&(p_manager->batch_manager))==0){
	p_manager->batch_manager_flag=1;
      }
      else{
	ERROR_LOGGER("unable to initialize manager for batch system '%s'",p_manager->batch_system_name);
      }
    }
    else{
      ERROR_LOGGER("unable to load batch system '%s'",p_manager->batch_system_name);
    }
  }
  else{
    DEBUG_LOGGER("batch system is not defined");
  }

  if(p_manager->rm_system_name!=NULL &&
     p_manager->rm_system_lib!=NULL){
    if(strncmp("none",p_manager->rm_system_name,5)!=0 &&
       strncmp("none",p_manager->rm_system_lib,5)!=0)
      rm_system_required=1;
  }
  if(rm_system_required==1){
    /* try to load resource managemer plugin */
    if(bridge_load_rm_system(p_manager,
			     &(p_manager->rm_system),
			     p_manager->rm_system_name,
			     p_manager->rm_system_lib)==0){
      p_manager->rm_system_flag=1;
      if(p_manager->rm_system.init_rm_manager(&(p_manager->rm_manager))==0){
	p_manager->rm_manager_flag=1;
      }
      else{
	ERROR_LOGGER("unable to initialize manager for rm system '%s'",p_manager->rm_system_name);
      }
    }
    else{
      ERROR_LOGGER("unable to load rm system '%s'",p_manager->rm_system_name);
    }
  }
  else{
    DEBUG_LOGGER("resource management system is not defined");
  }

  /* if a binding is defined batch and rm systems are required */
  if(p_manager->bs_rm_binding!=BRIDGE_BS_RM_BINDING_NONE){
    DEBUG_LOGGER("batch and resource management systems are required due to specified binding");
    batch_system_required=1;
    rm_system_required=1;
  }
  
  fstatus=0;  
  if(batch_system_required && !p_manager->batch_manager_flag){
    fstatus+=-10;
  }
  if(rm_system_required && !p_manager->rm_manager_flag){
    fstatus=-20;
  }

  return fstatus;
}

/*!
 * \ingroup BRIDGE
 * \brief Destroy bridge manager
 *
 * \param p_manager pointer on a bridge manager structure
 *
 * \retval  0 operation successfully done
 * \retval -1 internal error
*/
int bridge_clean_manager(bridge_manager_t* p_manager){

  int fstatus=-1;
  // rm
  if(p_manager->rm_manager_flag){
    p_manager->rm_system.clean_rm_manager(&(p_manager->rm_manager));
  }
  if(p_manager->rm_system_flag){
    bridge_unload_rm_system(p_manager,&(p_manager->rm_system));
  }
  // batch
  if(p_manager->batch_manager_flag){
    p_manager->batch_system.clean_batch_manager(&(p_manager->batch_manager));
  }
  if(p_manager->batch_system_flag){
    bridge_unload_batch_system(p_manager,&(p_manager->batch_system));
  }

  if(p_manager->plugins_dir!=NULL){
    free(p_manager->plugins_dir);
    p_manager->plugins_dir=NULL;
  }
  
  if(p_manager->batch_system_name!=NULL){
    free(p_manager->batch_system_name);
    p_manager->batch_system_name=NULL;
  }
  if(p_manager->batch_system_lib!=NULL){
    free(p_manager->batch_system_lib);
    p_manager->batch_system_lib=NULL;
  }

  if(p_manager->rm_system_name!=NULL){
    free(p_manager->rm_system_name);
    p_manager->rm_system_name=NULL;
  }
  if(p_manager->rm_system_lib!=NULL){
    free(p_manager->rm_system_lib);
    p_manager->rm_system_lib=NULL;
  }

  if(p_manager->logfile!=NULL)
    fclose(p_manager->logfile);
  if(p_manager->debugfile!=NULL)
    fclose(p_manager->debugfile);

  if(p_manager->version!=NULL){
    free(p_manager->version);
    p_manager->version=NULL;
  }

  fstatus=0;

  return fstatus;
}

/*
 * ---------------------------------------------------------------------------------------------------
 * Batch manager related functions                                                                 FIN
 * ---------------------------------------------------------------------------------------------------
 */


/*
 * ---------------------------------------------------------------------------------------------------
 * Batch system related functions                                                              DEBUT
 * ---------------------------------------------------------------------------------------------------
 */
int
bridge_get_batch_id (bridge_manager_t* p_manager,char** id)
{
  int status=-1;

  if(!p_manager->batch_manager_flag)
    return status;

  status=(p_manager->batch_system).get_batch_id(id);

  return status;
}

int bridge_get_batch_sessions(bridge_manager_t* p_manager,
			      bridge_batch_session_t ** p_batch_sessions,
			      int* p_batch_sessions_nb,
			      char* batch_sessions_batch_id,
			      char* name,
			      char* username,
			      char* batch_queue,
			      char* execHost,
			      int rm_infos_flag){
  
  int status=-1;

  /* just for binding */
  bridge_batch_session_t* bsession=NULL;
  bridge_rm_allocation_t* allocations_array=NULL;
  bridge_rm_allocation_t* allocation=NULL;
  int allocations_nb=0;
  int allocation_id;
  int i;
  char* par_nodelist;

  
  if(!p_manager->batch_manager_flag)
    return status;
  /* should never happened because of manager init function */
  if(p_manager->bs_rm_binding!=BRIDGE_BS_RM_BINDING_NONE &&
     !p_manager->rm_manager_flag)
    return status;
    
  switch (p_manager->bs_rm_binding){

  case BRIDGE_BS_RM_BINDING_NONE :

    status=(p_manager->batch_system).get_batch_sessions(&(p_manager->batch_manager),
							p_batch_sessions,
							p_batch_sessions_nb,
							batch_sessions_batch_id,
							name,
							username,
							batch_queue,
							execHost,
							rm_infos_flag);
    break;

  case BRIDGE_BS_RM_BINDING_RMID :
    status=(p_manager->batch_system).get_batch_sessions(&(p_manager->batch_manager),
							p_batch_sessions,
							p_batch_sessions_nb,
							batch_sessions_batch_id,
							name,
							username,
							batch_queue,
							execHost,
							0);
    if(status==0){
      if(p_manager->rm_manager_flag){
	status=bridge_get_rm_allocations(p_manager,
					     &allocations_array,
					     &allocations_nb,
					     NULL,
					     NULL,
					     NULL,
					     NULL,
					     NULL);
	if(status==0){
	  for(i=0;i<*p_batch_sessions_nb;i++){
	    
	    bsession=((*p_batch_sessions)+i);
	    
	    for(allocation_id=0;allocation_id<allocations_nb;allocation_id++){
	      allocation=allocations_array+allocation_id;
	      if(bsession->rm_id!=NULL){
		if(strcmp(bsession->rm_id,allocation->id)==0){
		  /* set used core nb only if not already done */
		  if(bsession->par_cores_nb==0)
		    bsession->par_cores_nb=allocation->total_cores_nb;
		  bsession->par_time_used=allocation->allocated_time/allocation->total_cores_nb;
		  bsession->par_mem_used=allocation->memory_usage;
		  
		  if(bridge_nodelist_get_compacted_string(&(allocation->nodelist),
								&par_nodelist)==0){
		    
		    bridge_nodelist_add_nodes(&(bsession->par_nodelist),
						    par_nodelist);
		    free(par_nodelist);
		  }
		  
		}
	      }
	    }
	    
	  }
	  
	  for(allocation_id=0;allocation_id<allocations_nb;allocation_id++){
	    allocation=allocations_array+allocation_id;
	    bridge_clean_rm_allocation(p_manager,allocation);
	  }
	  free(allocations_array);

	}
	
      }

      status=0;
    }
    break;

  case BRIDGE_BS_RM_BINDING_BATCHID :
  case BRIDGE_BS_RM_BINDING_EXECHOST_SID :
    status=(p_manager->batch_system).get_batch_sessions(&(p_manager->batch_manager),
							p_batch_sessions,
							p_batch_sessions_nb,
							batch_sessions_batch_id,
							name,
							username,
							batch_queue,
							execHost,
							0);
    break;
  }

  return status;
}

int bridge_get_terminated_batch_sessions(bridge_manager_t* p_manager,
					 bridge_batch_session_t ** p_batch_sessions,
					 int* p_batch_sessions_nb,
					 char* batch_sessions_batch_id,
					 char* name,
					 char* username,
					 char* batch_queue,
					 char* execHost,
					 int rm_infos_flag,
					 time_t begin_eventTime,time_t end_eventTime){

  int fstatus=-1;
  if(!p_manager->batch_manager_flag)
    return fstatus;
  /* should never happened because of manager init function */
  if(p_manager->bs_rm_binding!=BRIDGE_BS_RM_BINDING_NONE &&
     !p_manager->rm_manager_flag)
    return fstatus;
  
  fstatus=(p_manager->batch_system).get_terminated_batch_sessions(&(p_manager->batch_manager),
								  p_batch_sessions,
								  p_batch_sessions_nb,
								  batch_sessions_batch_id,
								  name,
								  username,
								  batch_queue,
								  execHost,
								  rm_infos_flag,
								  begin_eventTime,end_eventTime);
  return fstatus;
}

int bridge_clean_batch_session(bridge_manager_t* p_manager,
			       bridge_batch_session_t * p_batch_session){
 
  int fstatus=-1;

  if(!p_manager->batch_manager_flag)
    return fstatus;

  fstatus=(p_manager->batch_system).clean_batch_session(&(p_manager->batch_manager),
							p_batch_session);
  return fstatus;
}

int bridge_get_batch_queues(bridge_manager_t* p_manager,
			    bridge_batch_queue_t ** p_batch_queues,
			    int* p_batch_queues_nb,
			    char* batch_queue_name){
  int fstatus=-1;

  if(!p_manager->batch_manager_flag)
    return fstatus;
  
  fstatus=(p_manager->batch_system).get_batch_queues(&(p_manager->batch_manager),
						     p_batch_queues,
						     p_batch_queues_nb,
						     batch_queue_name);
  return fstatus;
}

int bridge_clean_batch_queue(bridge_manager_t* p_manager,
			     bridge_batch_queue_t * p_batch_queue){
  
  int fstatus=-1;

  if(!p_manager->batch_manager_flag)
    return fstatus;
  
  fstatus=(p_manager->batch_system).clean_batch_queue(&(p_manager->batch_manager),
						      p_batch_queue);
  return fstatus;
}

int bridge_get_batch_nodes(bridge_manager_t* p_manager,
			       bridge_batch_node_t ** p_batch_nodes,
			       int* p_batch_nodes_nb,
			       char* batch_node_name){
  int fstatus=-1;

  if(!p_manager->batch_manager_flag)
    return fstatus;
  
  fstatus=(p_manager->batch_system).get_batch_nodes(&(p_manager->batch_manager),
						    p_batch_nodes,
						    p_batch_nodes_nb,
						    batch_node_name);
  return fstatus;
}

int bridge_clean_batch_node(bridge_manager_t* p_manager,
			    bridge_batch_node_t * p_batch_node){
  
  return (p_manager->batch_system).clean_batch_node(&(p_manager->batch_manager),
						    p_batch_node);
}
/*
 * ---------------------------------------------------------------------------------------------------
 * EOF Batch system related functions                                                                   FIN
 * ---------------------------------------------------------------------------------------------------
 */

/*
 * ---------------------------------------------------------------------------------------------------
 * Resource management system related functions                                                    DEBUT
 * ---------------------------------------------------------------------------------------------------
 */
int
bridge_get_rm_id (bridge_manager_t* p_manager,char** id)
{
  int status=-1;

  if(!p_manager->rm_manager_flag)
    return status;

  status=(p_manager->rm_system).get_rm_id(id);

  return status;
}

int bridge_get_rm_allocations(bridge_manager_t * p_manager,
			      bridge_rm_allocation_t ** p_rm_allocations,
			      int* p_rm_allocations_nb,
			      char* rm_allocations_rm_id,
			      char* username,
			      char* rm_partition,
			      char* intersectingNodes,
			      char* includingNodes){
  if(!p_manager->rm_manager_flag)
    return -1;
  int status;
  status=(p_manager->rm_system).get_rm_allocations(&(p_manager->rm_manager),
						   p_rm_allocations,
						   p_rm_allocations_nb,
						   rm_allocations_rm_id,
						   username,
						   rm_partition,
						   intersectingNodes,
						   includingNodes);
  return status;
}

int bridge_get_terminated_rm_allocations(bridge_manager_t * p_manager,
					 bridge_rm_allocation_t ** p_rm_allocations,
					 int* p_rm_allocations_nb,
					 char* rm_allocations_rm_id,
					 char* username,
					 char* rm_partition,
					 char* intersectingNodes,
					 char* includingNodes,
					 time_t begin_eventTime,time_t end_eventTime){
  if(!p_manager->rm_manager_flag)
    return -1;
  int status;
  status=(p_manager->rm_system).get_terminated_rm_allocations(&(p_manager->rm_manager),
							      p_rm_allocations,
							      p_rm_allocations_nb,
							      rm_allocations_rm_id,
							      username,
							      rm_partition,
							      intersectingNodes,
							      includingNodes,
							      begin_eventTime,end_eventTime);
  return status;
}

int bridge_clean_rm_allocation(bridge_manager_t* p_manager,
			       bridge_rm_allocation_t * p_rm_allocation){
  
  if(!p_manager->rm_manager_flag)
    return -1;
  return (p_manager->rm_system).clean_rm_allocation(&(p_manager->rm_manager),
						       p_rm_allocation);
}

int bridge_get_rm_partitions(bridge_manager_t * p_manager,
			     bridge_rm_partition_t ** p_rm_partitions,
			     int* p_rm_partitions_nb,
			     char* rm_partition,
			     char* intersectingNodes,
			     char* includingNodes){
  if(!p_manager->rm_manager_flag)
    return -1;
  int status;
  status=(p_manager->rm_system).get_rm_partitions(&(p_manager->rm_manager),
						  p_rm_partitions,
						  p_rm_partitions_nb,
						  rm_partition,
						  intersectingNodes,
						  includingNodes);
  return status;
}

int bridge_clean_rm_partition(bridge_manager_t* p_manager,
			      bridge_rm_partition_t * p_rm_partition){
  if(!p_manager->rm_manager_flag)
    return 1;
  return (p_manager->rm_system).clean_rm_partition(&(p_manager->rm_manager),
						   p_rm_partition);
}
/*
 * ---------------------------------------------------------------------------------------------------
 * EOF Resource manager related related functions                                                      FIN
 * ---------------------------------------------------------------------------------------------------
 */

