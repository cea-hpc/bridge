/*****************************************************************************\
 *  src/bridge/rmmstat.c - 
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

#include "bridge/bridge.h"

#define PROG_VERSION  "1.0.1"

/*!
 * \brief display rm manager informations on file stream in an extended form 
 * \internal
 *
 * \param stream FILE* on which to write output
 * \param rm pointer on a rm manager structure to display
 *
 * \retval 0 on success
 * \retval -1 on failure
 */
int display_bridge_rm_manager_on_file_stream(FILE* stream,bridge_rm_manager_t* rm);

/*!
 * \brief display rm manager informations on file stream in an classic form 
 * \internal
 *
 * \param stream FILE* on which to write output
 * \param rm pointer on a rm manager structure to display
 *
 * \retval 0 on success
 * \retval -1 on failure
 */
int display_classic_bridge_rm_manager_on_file_stream(FILE * stream,bridge_rm_manager_t* rm);

/*!
 * \brief display required informations about rm manager on file stream 
 * \internal
 *
 * \param stream FILE* on which to write output
 * \param rm pointer on a rm manager structure to display
 * \param output_fields comma separated list of informations to display
 * \param separator string to write on stream between each required information
 *
 * \retval 0 on success
 * \retval -1 on failure
 */
int display_by_fields_bridge_rm_manager_on_file_stream(FILE* stream,bridge_rm_manager_t* rm,char* output_fields,char* separator);

int main(int argc,char **argv){

  int fstatus=-1;
  int status;

  char* progname;
  char* cb_version;

  bridge_manager_t manager;

  int i,j;
  int verbosity=0;

  FILE* output_stream=stdout;

  char* node_name=NULL;

  char* options_desc=
    "\t-h\t\t\tPrint this message\n"
    "\t-c\t\t\tDisplay results in classic format\n"
    "\t-o fields\t\tDisplay informations using fields format (use -o list for available fields, -o all to get infos for all fields)\n"
    "\t-s separator\t\tWhen used with -o options, enables to change results fields separator\n\t\t\t\t(default is a single spaced string)\n"
    "\t-v\t\t\tWhen used with -o options, the first output line displays list of selected fields\n"
    "\t-V\t\t\tPrint bridge and app versions and exit\n";

  char* fields_list="\n"
    "\tcluster       : `rm cluster` name\n"
    "\tdesc          : `rm cluster` description\n"
    "\ttype          : `rm cluster` type\n"
    "\tversion       : `rm cluster` version\n"
    "\tmaster        : `rm cluster` current master host\n";

  char* output_fields=NULL;
  char* separator=NULL;

#define EXTENDED_DISPLAY 0
#define COMPOSITE_DISPLAY 1
#define CLASSIC_DISPLAY 2
  int display_mode=EXTENDED_DISPLAY;

  char * optstring="ho:s:cvV";
  char option;

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
      display_mode=CLASSIC_DISPLAY;
      break;
    case 'v' :
      verbosity++;
      break;
    case 'o' :
      if(strcmp(optarg,"list")==0){
	fprintf(stdout,"\nAvailable fields are :\n%s\n",fields_list);
	goto exit;
      }
      else if(strcmp(optarg,"all")==0){
	output_fields=strdup("cluster,desc,type,version,master");
      }
      else
	output_fields=strdup(optarg);
      display_mode=COMPOSITE_DISPLAY;
      break;
    case 's' :
      separator=strdup(optarg);
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
      fprintf(stdout,"\nUsage : %s [-h] [-c] [-o fields [-s separator]] \n\n",argv[0]);
      fprintf(stdout,"%s\n",options_desc);
      exit(1);
      break;
    }
  }

  if(bridge_init_manager(&manager)==0){
    if(manager.rm_manager_flag){
      switch(display_mode){

      case EXTENDED_DISPLAY :
	display_bridge_rm_manager_on_file_stream(stdout,&(manager.rm_manager));
	break;
	
      case CLASSIC_DISPLAY :
	display_classic_bridge_rm_manager_on_file_stream(stdout,NULL);
	display_classic_bridge_rm_manager_on_file_stream(stdout,&(manager.rm_manager));
	break;
	
      case COMPOSITE_DISPLAY :
	if(verbosity)
	  display_by_fields_bridge_rm_manager_on_file_stream(stdout,NULL,output_fields,separator);
	display_by_fields_bridge_rm_manager_on_file_stream(stdout,&(manager.rm_manager),
							       output_fields,separator);
	break;
	
      }
      fstatus=0;
    }
    bridge_clean_manager(&manager);
  }
  else{
    fstatus=1;
  }
  
  exit :
    
    if(output_fields!=NULL)
      free(output_fields);
  if(separator!=NULL)
    free(separator);
  
  return fstatus;

}



int display_bridge_rm_manager_on_file_stream(FILE* stream,bridge_rm_manager_t* rm){

  int i;
  
  fprintf(stream,
	  "-------------------------------------------------------\n");
  if(rm->cluster!=NULL)
    fprintf(stream,"Cluster \t\t: %s\n",rm->cluster);
  else
    fprintf(stream,"Cluster \t\t: -\n");

  if(rm->description!=NULL)
    fprintf(stream,"Desc \t\t\t: %s\n",rm->description);
  else
    fprintf(stream,"Desc \t\t\t: -\n");

  if(rm->type!=NULL)
    fprintf(stream,"Type \t\t\t: %s\n",rm->type);
  else
    fprintf(stream,"Type \t\t\t: -\n");

  if(rm->version!=NULL)
    fprintf(stream,"Version \t\t: %s\n",rm->version);
  else
    fprintf(stream,"Version \t\t: -\n");

  if(rm->master!=NULL)
    fprintf(stream,"Master \t\t\t: %s\n",rm->master);
  else
    fprintf(stream,"Master \t\t\t: -\n");

  fprintf(stream,
	  "-------------------------------------------------------\n");
  return 0;
}

int display_classic_bridge_rm_manager_on_file_stream(FILE * stream,bridge_rm_manager_t* rm){
  int fstatus=0;
  
  if(!rm){
    fprintf(stream,"%-16s %-16s %-16s %-16s %-16s\n",
	    "----------------",
	    "----------------",
	    "----------------",
	    "----------------",
	    "----------------");
    fprintf(stream,"%-16s %-16s %-16s %-16s %-16s\n","Cluster Name","Type","Version","Master Host","Description");
    fprintf(stream,"%-16s  %-16s%-16s %-16s %-16s\n",
	    "----------------",
	    "----------------",
	    "----------------",
	    "----------------",
	    "----------------");
  }
  else{
    fprintf(stream,"%-16s %-16s %-16s %-16s %-16s\n",
	    rm->cluster != NULL ? rm->cluster : "unknown",
	    rm->type != NULL ? rm->type : "unknown",
	    rm->version != NULL ? rm->version : "unknown",
	    rm->master != NULL ? rm->master : "unknown",
	    rm->description != NULL ? rm->description : "unknown");
    fprintf(stream,"%-16s %-16s %-16s %-16s %-16s\n",
	    "----------------",
	    "----------------",
	    "----------------",
	    "----------------",
	    "----------------");
  }

  return fstatus;
}

int display_by_fields_bridge_rm_manager_on_file_stream(FILE* stream,bridge_rm_manager_t* rm,char* output_fields,char* separator){

  int status=0;

  int i;
  int display_header=0;

  char* token=NULL;

  int token_nb;

  if(separator==NULL)
    separator=" ";

  if(rm==NULL)
    display_header=1;

  status=bridge_common_string_get_tokens_quantity(output_fields,",",&token_nb);
  if(status==0){
    for(i=1;i<=token_nb;i++){
      if(bridge_common_string_get_token(output_fields,",",i,&token)<0){
	continue;
      }
      else{
	if(display_header==1){
	  fprintf(stream,"%s",token);
	}

	/* Manager cluster name */
	else if(strcmp(token,"cluster")==0){
	  if(rm->cluster!=NULL)
	    fprintf(stream,"%s",rm->cluster);
	  else
	    fprintf(stream,"-");
	}
	/* Manager description */
	else if(strcmp(token,"desc")==0){
	  if(rm->description!=NULL)
	    fprintf(stream,"%s",rm->description);
	  else
	    fprintf(stream,"-");
	}
	/* Manager type */
	else if(strcmp(token,"type")==0){
	  if(rm->type!=NULL)
	    fprintf(stream,"%s",rm->type);
	  else
	    fprintf(stream,"-");
	}
	/* Manager version */
	else if(strcmp(token,"version")==0){
	  if(rm->version!=NULL)
	    fprintf(stream,"%s",rm->version);
	  else
	    fprintf(stream,"-");
	}
	/* Manager master name */
	else if(strcmp(token,"master")==0){
	  if(rm->master!=NULL)
	    fprintf(stream,"%s",rm->master);
	  else
	    fprintf(stream,"-");
	}
	/* error */
	else{
	  fprintf(stream,"?");
	}

	free(token);
	token=NULL;
	if(i!=token_nb)
	  fprintf(stream,"%s",separator);
      }
    }
  }

  fprintf(stream,"\n");
  
  return 0;
}
