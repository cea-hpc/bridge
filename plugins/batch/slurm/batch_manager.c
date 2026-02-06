/*****************************************************************************\
 *  plugins/batch/slurm/batch_manager.c -
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
#include <time.h>
#include <ctype.h>

#include "xternal/xlogger.h"
#include "bridge/bridge.h"

#define DEBUG_LOGGER xdebug
#define DEBUG2_LOGGER xdebug2
#define DEBUG3_LOGGER xdebug3
#define ERROR_LOGGER xerror

#include <slurm/slurm.h>

/* local functions */
int check_batch_manager_validity(bridge_batch_manager_t* p_batch_manager);
int clean_batch_manager(bridge_batch_manager_t* p_batch_manager);


int
init_batch_manager(bridge_batch_manager_t* p_batch_manager)
{
	int fstatus=-1;

	long api_version;
	char version[128];

	/* deprecated fields - kept for ABI compatibility */
	p_batch_manager->cluster=NULL;
	p_batch_manager->master=NULL;
	p_batch_manager->masters_list=NULL;

	p_batch_manager->description=NULL;
	p_batch_manager->type=NULL;
	p_batch_manager->version=NULL;

#if SLURM_VERSION_NUMBER >= SLURM_VERSION_NUM(23,11,1)
        /* add required slurm_init() since slurm 23.11.1 */
        slurm_init(NULL);
#endif
	/* get slurm API version */
	api_version=slurm_api_version();
	snprintf(version,128,"%ld.%ld.%ld",
		 SLURM_VERSION_MAJOR(api_version),
		 SLURM_VERSION_MINOR(api_version),
		 SLURM_VERSION_MICRO(api_version));

	p_batch_manager->version=strdup(version);
	p_batch_manager->type=strdup("Slurm");
	p_batch_manager->description=strdup("no desc");

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

	if(p_batch_manager->description &&
	   p_batch_manager->type &&
	   p_batch_manager->version)
		fstatus=0;

	return fstatus;
}

int
clean_batch_manager(bridge_batch_manager_t* p_batch_manager)
{
	xfree(p_batch_manager->description);
	xfree(p_batch_manager->type);
	xfree(p_batch_manager->version);

#if SLURM_VERSION_NUMBER >= SLURM_VERSION_NUM(23,11,1)
        slurm_fini();
#endif

	return 0;
}

int
get_batch_id(char** id)
{
	int fstatus=-1;

	char* bs_id = getenv("SLURM_JOBID");
	if ( bs_id != NULL ) {
		*id = strdup(bs_id);
		if (*id != NULL)
			fstatus=0;
	}

	return fstatus;
}

int
get_manager_info(bridge_manager_info_t* p_info)
{
	int fstatus=-1;
	slurm_conf_t *pscc;
	int i;

	p_info->cluster = NULL;
	p_info->master = NULL;
	p_info->masters_list = NULL;

	/* get slurm controller configuration */
	if(slurm_load_ctl_conf(0, &pscc) != 0) {
		DEBUG3_LOGGER("unable to get slurmctl configuration");
		return fstatus;
	}

	/* get master name */
	if (pscc->control_machine[0] != NULL)
		p_info->master = strdup(pscc->control_machine[0]);

	/* get cluster name */
	if (pscc->cluster_name != NULL) {
		p_info->cluster = strdup(pscc->cluster_name);
	}
	else if (p_info->master != NULL) {
		/* guess it using master name */
		char* p;
		char* cluster;
		char* separator;
		cluster = strdup(p_info->master);
		if (cluster != NULL) {
			separator = index(cluster, '-');
			if (separator == NULL) {
				p = cluster;
				while (!isdigit(*p) && *p != '\0')
					p++;
				if (*p != '\0') {
					*p = '\0';
					p_info->cluster = strdup(cluster);
				}
			}
			else {
				if (strncmp("mgr", separator+1, 4) == 0) {
					*separator = '\0';
					p_info->cluster = strdup(cluster);
				}
				else if (strncmp("cws", separator-3, 3) == 0) {
					p_info->cluster = strdup(separator+1);
				}
			}
			free(cluster);
		}
	}

	/* build masters list */
	if (pscc->control_cnt >= 2) {
		p_info->masters_list = malloc(sizeof(char *) * (pscc->control_cnt + 1));
		if (p_info->masters_list != NULL) {
			for (i = 0; i < pscc->control_cnt; i++) {
				p_info->masters_list[i] = strdup(pscc->control_machine[i]);
			}
			p_info->masters_list[pscc->control_cnt] = NULL;
		}
	}

	/* free slurm controller configuration data */
	slurm_free_ctl_conf(pscc);

	fstatus = 0;
	return fstatus;
}

int
clean_manager_info(bridge_manager_info_t* p_info)
{
	int i;

	xfree(p_info->cluster);
	xfree(p_info->master);

	if (p_info->masters_list != NULL) {
		for (i = 0; p_info->masters_list[i] != NULL; i++) {
			xfree(p_info->masters_list[i]);
		}
		xfree(p_info->masters_list);
	}

	return 0;
}
