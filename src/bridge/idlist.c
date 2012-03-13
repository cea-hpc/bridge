#include "config.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

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
    "\t-c\t\t\tShow compacted ids list\n"
    "\t-e\t\t\tShow extended ids list\n"
    "\t-n\t\t\tShow ids quantity\n"
    "\t-V\t\t\tPrint bridge and app versions and exit\n";

  char* output_fields=NULL;
  char* separator=NULL;

  char * optstring="hcenV";
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
      fprintf(stream,"\nUsage : %s [-h] [-cen] id id idlist ... \n\n",argv[0]);
      fprintf(stream,"%s\n",options_desc);
      goto exit;
    }
  }

  bridge_idlist_t idlist;

  status=bridge_idlist_init(&idlist,&argv[optind],argc-optind);
  if(status){
    fprintf(stderr,"Unable to init ids list\n");
  }
  else{
    char* output_string;

    long int quantity=bridge_idlist_ids_quantity(&idlist);

    if(enum_mode){
      fprintf(stream,"%u\n",quantity);
    }
    else if(!extended_mode){
      if(bridge_idlist_get_compacted_string(&idlist,&output_string)==0){
	fprintf(stream,"%s\n",output_string);
	free(output_string);
      }
    }
    else{
      if(bridge_idlist_get_extended_string(&idlist,&output_string)==0){
	fprintf(stream,"%s\n",output_string);
	free(output_string);
      }
    }

    bridge_idlist_free_contents(&idlist);
  }



  exit :

  return fstatus;

}
