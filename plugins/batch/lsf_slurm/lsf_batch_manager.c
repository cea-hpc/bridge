/*****************************************************************************\
 *  plugins/batch/lsf_slurm/lsf_batch_manager.c - 
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
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <time.h>
#include <math.h>

#include <string.h>

#include "bridge/bridge.h"

#define DEBUG_LOGGER debug
#define DEBUG2_LOGGER debug2
#define DEBUG3_LOGGER debug3
#define ERROR_LOGGER error

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
