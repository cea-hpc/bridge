#include "config.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <time.h>
#include <math.h>

#include <string.h>

#include "bridge/bridge.h"

#define DEBUG_LOGGER xdebug
#define DEBUG2_LOGGER xdebug2
#define DEBUG3_LOGGER xdebug3
#define ERROR_LOGGER xerror

#include <lsf/lsbatch.h>

int init_batch_manager(bridge_batch_manager_t* p_batch_manager){
  int fstatus=-1;
  
  char* cluster_name;
  char* master_name;

  p_batch_manager->cluster=NULL;
  p_batch_manager->master=NULL;
  p_batch_manager->masters_list=NULL;
  p_batch_manager->description=NULL;
  p_batch_manager->type=NULL;
  p_batch_manager->version=NULL;

  cluster_name=ls_getclustername();
  if(cluster_name){
    master_name=ls_getmastername();
    if(master_name){
      p_batch_manager->cluster=strdup(cluster_name);
      p_batch_manager->master=strdup(master_name);
      p_batch_manager->masters_list=NULL;
      p_batch_manager->type=strdup("LSF");
      p_batch_manager->description=strdup("Platform LSF");
      p_batch_manager->version=strdup(LSF_CURRENT_VERSION);
    }
    fstatus=1;
  }
  else{
    fstatus=1;
  }
  
  if(check_batch_manager_validity(p_batch_manager)==0){
    lsb_init(NULL);
    fstatus=0;
  }
  else
    clean_batch_manager(p_batch_manager);
    
  return fstatus;
}


int
check_batch_manager_validity(bridge_batch_manager_t* p_batch_manager){
  int fstatus=-1;

  if(
     p_batch_manager->cluster &&
     p_batch_manager->master &&
     p_batch_manager->description &&
     p_batch_manager->type &&
     p_batch_manager->version
     )
    fstatus=0;

  return fstatus;
}


int clean_batch_manager(bridge_batch_manager_t* p_batch_manager){
  int fstatus=-1;

  xfree(p_batch_manager->cluster);
  xfree(p_batch_manager->master);
  xfree(p_batch_manager->description);
  xfree(p_batch_manager->type);
  xfree(p_batch_manager->version);
  xfree(p_batch_manager->masters_list);

  fstatus=0;

  return fstatus;
}

int get_batch_id(char** id) {
  int fstatus=-1;

  char* bs_id = getenv("LSB_JOBID");
  
  if ( bs_id != NULL ) {
    *id = strdup(bs_id);

    if ( *id != NULL )
      fstatus=0;
  }
  
  return fstatus;
}
