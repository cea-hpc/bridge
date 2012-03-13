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

#include <pbs_error.h>
#include <pbs_ifl.h>
extern char *pbs_server;

static int pbs_fd;

int
init_batch_manager(bridge_batch_manager_t* p_batch_manager)
{
	int fstatus=-1;
  
	struct batch_status * ostat;
	struct attrl *attr;

	char* cluster_name;
	char* master_name;

	p_batch_manager->cluster=NULL;
	p_batch_manager->master=NULL;
	p_batch_manager->masters_list=NULL;
	p_batch_manager->description=NULL;
	p_batch_manager->type=NULL;
	p_batch_manager->version=NULL;

	/* connection to the pbs master */
	pbs_fd = pbs_connect(NULL);
	p_batch_manager->private = (void*) &pbs_fd ;
	if ( pbs_fd < 0 ) {
		DEBUG_LOGGER("unable to connect to Torque server");
		return fstatus ;
	}

	/* set information based on pbs_server */
	p_batch_manager->type=strdup("Torque");
	p_batch_manager->description=strdup("Torque Batch System");
	p_batch_manager->master=strdup(pbs_server);
	p_batch_manager->cluster=strdup(pbs_server);
	if ( p_batch_manager->cluster != NULL ) {
		char* p;
		int i;
		i = strlen(p_batch_manager->cluster);
		p=p_batch_manager->cluster + i - 1 ;
		while ( isdigit(*p) ) {
			*p='\0';
			p--;
			i--;
			if ( i == 0 )
				break;
		}
	}

	/* get master stats */
	ostat = pbs_statserver(pbs_fd,NULL,NULL);
	if ( ostat == NULL ) {
		pbs_disconnect(pbs_fd);
		pbs_fd=-1;
	}

	/* set master PBS version */
	attr = ostat->attribs ;
	while ( attr != NULL ) {
		if (!strcmp(attr->name, ATTR_pbsversion))
		{
			p_batch_manager->version=strdup(attr->value);
		}
		attr = attr->next ;
	}
	
	/* clean master stats */
	pbs_statfree(ostat);

	/* check validity */
	fstatus = 1;
	if(check_batch_manager_validity(p_batch_manager)==0){
		fstatus=0;
	}
	else
		clean_batch_manager(p_batch_manager);


	return fstatus;
}


int
check_batch_manager_validity(bridge_batch_manager_t* p_batch_manager)
{
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


int
clean_batch_manager(bridge_batch_manager_t* p_batch_manager)
{
	int fstatus=-1;
	int con_fd;

	con_fd = *((int*)p_batch_manager->private);

	if ( con_fd >= 0 )
		pbs_disconnect(con_fd);

	xfree(p_batch_manager->cluster);
	xfree(p_batch_manager->master);
	xfree(p_batch_manager->description);
	xfree(p_batch_manager->type);
	xfree(p_batch_manager->version);
	xfree(p_batch_manager->masters_list);

	fstatus=0;

	return fstatus;
}

int
get_batch_id(char** id)
{
	int fstatus=-1;
	char* p;

	char* bs_id = getenv("PBS_JOBID");

	/* PBS_JOBID is of the form %id.%master.local */
	/* we just keep the %id part */
  	if ( bs_id != NULL ) {
		*id = strdup(bs_id);

		if ( *id != NULL ) {
			fstatus=0;

			/* remove the trailing part */
			p = index(*id,'.');
			if ( p != NULL )
				*p='\0';

		}

	}
  
	return fstatus;
}
