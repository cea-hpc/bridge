/*****************************************************************************\
 *  src/bridgedc/bridgedc.c - 
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
#include "bridged/bridgedapi.h"

#define PING_REQUEST 1
#define GET_REQUEST  2

#define VERBOSE xverbose
#define ERROR xerror

int main(int argc,char** argv){

  int fstatus=-1;

  /* options processing variables */
  char* progname;
  char  option;
  char* optstring="dvhf:pgb:r:";
  char* short_options_desc="\nUsage : %s [-h] [-dv] [-f conffile] [-b batchid] [-r rmid] [-p|g]\n\n";
  char* addon_options_desc="\
\t-h\t\tshow this message\n\
\t-d\t\tincrease debug level\n\
\t-v\t\tincrease verbose level\n\
\t-f conffile\tConfiguration file\n\
\t-b batchid\tBatch session identifier\n\
\t-r rmid\t\tResource manager job identifier\n\
\t-p\t\tping request (default)\n\
\t-g\t\tget request\n\n";

  /* local variables */
  char* conf_file_string=NULL;
  char* batchid=NULL;
  char* rmid=NULL;

  /* request output values */
  time_t used,usable,halt;

  int debug_level=0;
  int verbose_level=0;

  int action=PING_REQUEST;

  /* get current program name */
  progname=rindex(argv[0],'/');
  if(progname==NULL)
    progname=argv[0];
  else
    progname++;
  
  /* process options */
  while((option = getopt(argc,argv,optstring)) != -1)
    {
      switch(option)
	{
	case 'v' :
	  verbose_level++;
	  break;
	case 'd' :
	  debug_level++;
	  break;
	case 'f' :
	  conf_file_string=strdup(optarg);
	  break;
	case 'b' :
	  batchid=strdup(optarg);
	  if(batchid==NULL){
	    fprintf(stderr,"memory allocation error!");
	    return 1;
	  }
	  break;
	case 'r' :
	  rmid=strdup(optarg);
	  if(rmid==NULL){
	    fprintf(stderr,"memory allocation error!");
	    return 1;
	  }
	  break;
	case 'p' :
	  action=PING_REQUEST;
	  break;
	case 'g' :
	  action=GET_REQUEST;
	  break;
	case 'h' :
	default :
	  fprintf(stdout,short_options_desc,progname);
	  fprintf(stdout,"%s\n",addon_options_desc);
	  exit(0);
	  break;
	}
    }

  /* set verbosity and debug level */
  xdebug_setmaxlevel(debug_level);
  xverbose_setmaxlevel(verbose_level);
  
  switch(action){

  case GET_REQUEST :
    fstatus = bridgedapi_get(conf_file_string,batchid,rmid,
			     &usable,&used,&halt);
    if ( fstatus == 0 ) {
      fprintf(stdout,"used=%u usable=%u halt=%u\n",
	      used,usable,halt);
    }
    else {
      fprintf(stderr,"unable to process get request\n");
    }
    break;

  case PING_REQUEST :
  default :
    fstatus=bridgedapi_ping(conf_file_string);
    if ( fstatus != 0 ) {
      fprintf(stderr,"unable to process ping request\n");
    }
    break;

  }

  if(batchid!=NULL)
    free(batchid);

  if(rmid!=NULL)
    free(rmid);

  return fstatus;

}
