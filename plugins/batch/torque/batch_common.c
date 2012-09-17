/*****************************************************************************\
 *  plugins/batch/torque/batch_common.c - 
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

#include <stdint.h>

#include "batch_common.h"

#define DEBUG_LOGGER xdebug
#define DEBUG2_LOGGER xdebug2
#define DEBUG3_LOGGER xdebug3
#define ERROR_LOGGER xerror

int extract_field(char* statusline,char* header,char** field)
{
	char* p;
	char* e;
	
	size_t s;
	
	p = strstr(statusline,header);
	
	if ( p != NULL ) {
		p+= strlen(header);
		e = strchr(p,',');
		if ( e == NULL )
			*field = strdup(p);
		else {
			s = (size_t) (e-p);
			*field = (char*) malloc ( (s+1) * sizeof(char) );
			memcpy(*field,p,s);
			(*field)[s]='\0';
		}
		return 0;
	}

	return 1;
}

int extract_mem(char* statusline,char* header,uint32_t* i)
{
	int fstatus;
	char* field;

	if ( extract_field(statusline,header,&field) == 0 ) {
		fstatus = convert_mem(field,i);
		free(field);
		return fstatus;
	}

	return 1;
}

int extract_float(char* statusline,char* header,float* load)
{
	char* p;
	char* e;
	char* field;

	if ( extract_field(statusline,header,&field) == 0 ) {
		sscanf(field,"%f",load);
		free(field);
		return 0;
	}

	return 1;

}

int extract_cores(char* line,uint32_t* i)
{

	char* boe;
	char* noe;
	char* moe;
	char* eoe;
	char c,d;

	uint32_t core;
	uint32_t node;
	uint32_t total;

	total=0;

	boe=line;

	/* a line is elt[,elt]* */
	/* elt is node_nb[:...]*  ppn=core_nb */
	while ( boe != NULL ) {
		
		node=0;
		core=1;

		/* get nodes nb */
		eoe=boe;
		while ( isdigit(*eoe) )
			eoe++;
		c=*eoe;
		*eoe='\0';
		sscanf(boe,"%d",&node);
		*eoe=c;
		
		/* search the end of the element */
		moe = index(boe,'+');
		if ( moe != NULL ) {
			d=*moe;
			*moe='\0';
		}
		
		/* get cores nb for each of those nodes */
		noe = strstr(boe,"ppn=");
		if ( noe != NULL ) {
			noe+=4;
			eoe=noe;
			while ( isdigit(*eoe) )
				eoe++;
			c=*eoe;
			*eoe='\0';
			sscanf(noe,"%d",&core);
			*eoe=c;
		}
		total += node * core ;
		
		if ( moe != NULL )
			*moe=d;
		
		/* search next element */
		boe = index(boe,'+');
		if ( boe != NULL )
			boe++;
		
	}

	*i=total;

	return 0;
}

int extract_uint32(char* statusline,char* header,uint32_t* i)
{
	char* field;

	if ( extract_field(statusline,header,&field) == 0 ) {
		sscanf(field,"%u",i);
		free(field);
		return 0;
	}

	return 1;
}

int convert_mem(char* field,uint32_t* i)
{
	char unit[3];
	uint32_t mem;
	
	
	if ( sscanf(field,"%u%2s",&mem,unit) == 2 ) {
		unit[2]='\0';
		if ( strcmp(unit,"kb") == 0 )
			mem = mem / 1024 ;
		else if ( strcmp(unit,"gb") == 0 )
			mem = mem * 1024 ;
		else
			return 1;
		*i = mem;
		return 0;
	}

	return 1;
}

int convert_time(char* field,time_t* t)
{
	char unit[3];
	time_t time,hour,min,sec;
	
	
	
	if ( sscanf(field,"%u:%u:%u",&hour,&min,&sec) == 3 ) {
		*t = hour*3600 + min*60 + sec ;
		return 0;
	}

	return 1;
}
