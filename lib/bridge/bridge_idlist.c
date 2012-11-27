/*****************************************************************************\
 *  lib/bridge/bridge_idlist.c - 
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

#include <search.h>

#include <limits.h>

#include "bridge.h"

#include "xternal/xlogger.h"
#define DEBUG_LOGGER xdebug
#define DEBUG2_LOGGER xdebug2
#define DEBUG3_LOGGER xdebug3
#define ERROR_LOGGER xerror

#define MAX_LONG_INT_STRING_SIZE      128

int bridge_idlist_init(bridge_idlist_t* idlist,char** lists, int lists_nb){

  int fstatus=0;

  int i;
  char* list;

  idlist->id_nb=0;
  if(bridge_rangelist_init(&(idlist->rangelist))!=0)
    {
      fstatus=-1;
    }
  else
    {
    for(i=0;i<lists_nb;i++)
      {
	list=lists[i];
	fstatus=bridge_rangelist_add_list(&(idlist->rangelist),list);
	if(fstatus!=0)
	  break;
      }
    if(fstatus!=0)
      {
	bridge_rangelist_free_contents(&(idlist->rangelist));
	fstatus=-1;
      }
    else
      fstatus=0;
    }
  
  return fstatus;
}

int
bridge_idlist_add_ids(bridge_idlist_t* idlist,char* list)
{
  int fstatus;
  fstatus=bridge_rangelist_add_list(&(idlist->rangelist),list);
  return fstatus;
}

int bridge_idlist_free_contents(bridge_idlist_t* idlist)
{
  int fstatus;
  fstatus=bridge_rangelist_free_contents(&(idlist->rangelist));
  idlist->id_nb=0;
  return fstatus;
}

int bridge_idlist_add_range(bridge_idlist_t* idlist,bridge_range_t* r)
{
  int fstatus=-1;
  fstatus=bridge_rangelist_add_range(&(idlist->rangelist),r);
  return fstatus;
}

long int
bridge_idlist_ids_quantity(bridge_idlist_t* idlist)
{
  long int quantity;
  long int i;
  quantity=0;
  for(i=0;i<idlist->rangelist.ranges_nb;i++)
    {
      quantity+=(idlist->rangelist.array[i].to-idlist->rangelist.array[i].from+1);
    }
  return quantity;
}

int
bridge_idlist_get_compacted_string(bridge_idlist_t* idlist,char** p_string)
{
  int fstatus=-1;
  int brackets_flag;

  char* range_string;
  size_t range_string_size;

  char* ranges_string;
  size_t ranges_string_size;

  char* output_string;
  size_t output_string_size=1024;

  long int ids_nb;
  long int i;

  long int from,to;

  ids_nb=bridge_idlist_ids_quantity(idlist);

  if(ids_nb==0)
    return fstatus;
  else if(ids_nb==1)
    brackets_flag=0;
  else
    brackets_flag=1;

  range_string_size=0;
  range_string_size=2*MAX_LONG_INT_STRING_SIZE+2;

  ranges_string_size=1024;
  ranges_string=(char*)malloc(ranges_string_size*sizeof(char));
  if(ranges_string!=NULL)
    {
      ranges_string[0]='\0';
      range_string=(char*)malloc(range_string_size*sizeof(char));
      if(range_string!=NULL)
	{
	  range_string[0]='\0';
	  for(i=0;i<idlist->rangelist.ranges_nb;i++)
	    {
	      from=idlist->rangelist.array[i].from;
	      to=idlist->rangelist.array[i].to;
	      if(from==to)
		{
		  snprintf(range_string,range_string_size,"%ld",from);
		}
	      else
		{
		  snprintf(range_string,range_string_size,"%ld-%ld",from,to);
		}
	      fstatus=bridge_common_string_appends_and_extends(&ranges_string,&ranges_string_size,range_string_size,range_string,",");
	      if(fstatus!=0)
		{
		  fstatus=-1;
		  break;
		}
	    }
	  free(range_string);
	}
    
      if(fstatus==0)
	{
	  output_string_size=3; // []\0
	  output_string_size+=ranges_string_size;
	  output_string=(char*)malloc(output_string_size*sizeof(char));
	  if(output_string!=NULL)
	    {
	      snprintf(output_string,output_string_size,"%s%s%s",
		       brackets_flag?"[":"",
		       ranges_string,
		       brackets_flag?"]":"");
	      fstatus=0;
	      *p_string=output_string;
	    }
	}
      free(ranges_string);
    }

  return fstatus;
}

int
bridge_idlist_get_extended_string(bridge_idlist_t* idlist,char** p_string)
{

  int fstatus=-1;

  size_t id_string_size=MAX_LONG_INT_STRING_SIZE;
  char id_string[id_string_size];

  char* output_string;
  size_t output_string_size=1024;



  long int i,j;

  if(idlist->rangelist.ranges_nb==0)
    return fstatus;
  
  output_string=(char*)malloc(output_string_size*sizeof(char));
  if(output_string)
    {
      output_string[0]='\0';
      for(i=0;i<idlist->rangelist.ranges_nb;i++)
	{
	  for(j=idlist->rangelist.array[i].from;j<=idlist->rangelist.array[i].to;j++)
	    {
	      snprintf(id_string,id_string_size,"%ld",j);
	      fstatus=bridge_common_string_appends_and_extends(&output_string,&output_string_size,id_string_size,id_string," ");
	      if(fstatus!=0)
		fstatus=-1;
	    }
	}

      if(fstatus!=0){
	free(output_string);
      }
      else{
	*p_string=output_string;
      }
    }

  return fstatus;
}
