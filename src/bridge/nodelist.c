/*****************************************************************************\
 *  src/bridge/nodelist.c - 
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

#include <string.h>

#include "bridge/bridge.h"

#define PROG_VERSION  "1.0.1"

extern int optind;

int main(int argc,char** argv){

  int fstatus=-1;
  int status;

  FILE* stream=stdout;

  int compacted_mode=1;
  int extended_mode=0;
  int enum_mode=0;

  char* progname;
  char* cb_version;
  char* options_desc=
    "\t-h\t\t\tPrint this message\n"
    "\t-c\t\t\tShow compacted nodes list\n"
    "\t-e\t\t\tShow extended nodes list\n"
    "\t-n\t\t\tShow nodes quantity\n"
    "\t-V\t\t\tPrint bridge and app versions and exit\n";

  char * optstring="hcenV";
  int option;

  progname=strrchr(argv[0],'/');
  if(progname==NULL)
    {
      progname=argv[0];
    }
  else
    {
      progname++;
    }

  while((option = getopt(argc,argv,optstring)) != -1) {
    switch(option){
    case 'c' :
      compacted_mode=1;
      break;
    case 'e' :
      extended_mode=1;
      break;
    case 'n' :
      enum_mode=1;
      break;
    case 'V' :
      cb_version=bridge_version();
      if(cb_version!=NULL)
	fprintf(stdout,"%s version %s (bridge-%s)\n",progname,PROG_VERSION,cb_version);
      else
	fprintf(stdout,"%s version %s (bridge-unknown)\n",progname,PROG_VERSION);
      goto exit;
      break;
    case 'h' :
    default :
      fprintf(stream,"\nUsage : %s [-h] [-cen] nodename nodename nodeslist ... \n\n",argv[0]);
      fprintf(stream,"%s\n",options_desc);
      goto exit;
    }
  }

  bridge_nodelist_t nodelist;

  status=bridge_nodelist_init(&nodelist,&argv[optind],argc-optind);
  if(status){
    fprintf(stderr,"Unable to init nodes list\n");
  }
  else{
    //    fprintf(stream,"Nodes quantity : %u\n",bridge_nodelist_nodes_quantity(&nodelist));

    char* output_string;

    if(enum_mode){
      fprintf(stream,"%ld\n",bridge_nodelist_nodes_quantity(&nodelist));
    }
    else if(!extended_mode){
      if(bridge_nodelist_get_compacted_string(&nodelist,&output_string)==0){
	fprintf(stream,"%s\n",output_string);
	free(output_string);
      }
    }
    else{
      if(bridge_nodelist_get_extended_string(&nodelist,&output_string)==0){
	fprintf(stream,"%s\n",output_string);
	free(output_string);
      }
    }

    bridge_nodelist_free_contents(&nodelist);
  }

  exit :

  return fstatus;

}
