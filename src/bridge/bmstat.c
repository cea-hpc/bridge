#include "config.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "bridge/bridge.h"

#define PROG_VERSION  "1.0.1"

/*!
 * \brief display batch manager informations on file stream in an extended form 
 * \internal
 *
 * \param stream FILE* on which to write output
 * \param bm pointer on a batch manager structure to display
 *
 * \retval 0 on success
 * \retval -1 on failure
 */
int display_bridge_batch_manager_on_file_stream(FILE* stream,bridge_batch_manager_t* bm);

/*!
 * \brief display batch manager informations on file stream in an classic form 
 * \internal
 *
 * \param stream FILE* on which to write output
 * \param bm pointer on a batch manager structure to display
 *
 * \retval 0 on success
 * \retval -1 on failure
 */
int display_classic_bridge_batch_manager_on_file_stream(FILE * stream,bridge_batch_manager_t* bm);

/*!
 * \brief display required informations about batch manager on file stream 
 * \internal
 *
 * \param stream FILE* on which to write output
 * \param bm pointer on a batch manager structure to display
 * \param output_fields comma separated list of informations to display
 * \param separator string to write on stream between each required information
 *
 * \retval 0 on success
 * \retval -1 on failure
 */
int display_by_fields_bridge_batch_manager_on_file_stream(FILE* stream,bridge_batch_manager_t* bm,char* output_fields,char* separator);

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
    "\tcluster       : `batch cluster` name\n"
    "\tdesc          : `batch cluster` description\n"
    "\ttype          : `batch cluster` type\n"
    "\tversion       : `batch cluster` version\n"
    "\tmaster        : `batch cluster` current master host\n";

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

    switch(display_mode){

    case EXTENDED_DISPLAY :
      display_bridge_batch_manager_on_file_stream(stdout,&(manager.batch_manager));
      break;

    case CLASSIC_DISPLAY :
      display_classic_bridge_batch_manager_on_file_stream(stdout,NULL);
      display_classic_bridge_batch_manager_on_file_stream(stdout,&(manager.batch_manager));
      break;

    case COMPOSITE_DISPLAY :
      if(verbosity)
	display_by_fields_bridge_batch_manager_on_file_stream(stdout,NULL,output_fields,separator);
      display_by_fields_bridge_batch_manager_on_file_stream(stdout,&(manager.batch_manager),
							    output_fields,separator);
      break;

    }
    fstatus=0;
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



int display_bridge_batch_manager_on_file_stream(FILE* stream,bridge_batch_manager_t* bm){

  int i;
  
  fprintf(stream,
	  "-------------------------------------------------------\n");
  if(bm->cluster!=NULL)
    fprintf(stream,"Cluster \t\t: %s\n",bm->cluster);
  else
    fprintf(stream,"Cluster \t\t: -\n");

  if(bm->description!=NULL)
    fprintf(stream,"Desc \t\t\t: %s\n",bm->description);
  else
    fprintf(stream,"Desc \t\t\t: -\n");

  if(bm->type!=NULL)
    fprintf(stream,"Type \t\t\t: %s\n",bm->type);
  else
    fprintf(stream,"Type \t\t\t: -\n");

  if(bm->version!=NULL)
    fprintf(stream,"Version \t\t: %s\n",bm->version);
  else
    fprintf(stream,"Version \t\t: -\n");

  if(bm->master!=NULL)
    fprintf(stream,"Master \t\t\t: %s\n",bm->master);
  else
    fprintf(stream,"Master \t\t\t: -\n");

  fprintf(stream,
	  "-------------------------------------------------------\n");
  return 0;
}

int display_classic_bridge_batch_manager_on_file_stream(FILE * stream,bridge_batch_manager_t* bm){
  int fstatus=0;
  
  if(!bm){
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
	    (bm->cluster==NULL)?"-":bm->cluster,
	    (bm->type==NULL)?"-":bm->type,
	    (bm->version==NULL)?"-":bm->version,
	    (bm->master==NULL)?"-":bm->master,
	    (bm->description==NULL)?"-":bm->description);
    fprintf(stream,"%-16s %-16s %-16s %-16s %-16s\n",
	    "----------------",
	    "----------------",
	    "----------------",
	    "----------------",
	    "----------------");
  }

  return fstatus;
}

int display_by_fields_bridge_batch_manager_on_file_stream(FILE* stream,bridge_batch_manager_t* bm,char* output_fields,char* separator){

  int status=0;

  int i;
  int display_header=0;

  char* token=NULL;

  int token_nb;

  if(separator==NULL)
    separator=" ";

  if(bm==NULL)
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
	  if(bm->cluster!=NULL)
	    fprintf(stream,"%s",bm->cluster);
	  else
	    fprintf(stream,"-");
	}
	/* Manager description */
	else if(strcmp(token,"desc")==0){
	  if(bm->description!=NULL)
	    fprintf(stream,"%s",bm->description);
	  else
	    fprintf(stream,"-");
	}
	/* Manager type */
	else if(strcmp(token,"type")==0){
	  if(bm->type!=NULL)
	    fprintf(stream,"%s",bm->type);
	  else
	    fprintf(stream,"-");
	}
	/* Manager version */
	else if(strcmp(token,"version")==0){
	  if(bm->version!=NULL)
	    fprintf(stream,"%s",bm->version);
	  else
	    fprintf(stream,"-");
	}
	/* Manager master name */
	else if(strcmp(token,"master")==0){
	  if(bm->master!=NULL)
	    fprintf(stream,"%s",bm->master);
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
