/*****************************************************************************\
 *  lib/bridge/bridge_range.c - 
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
#include <limits.h>
#include <ctype.h>

#include "bridge_common.h"

#define DEFAULT_RANGELIST_SIZE       16
#define DEFAULT_RANGELIST_INC_SIZE    8

/* local functions */
int bridge_rangelist_intersects(bridge_rangelist_t* a1,bridge_rangelist_t* a2);
int bridge_rangelist_includes(bridge_rangelist_t* a1,bridge_rangelist_t* a2);

int
bridge_range_set(bridge_range_t* r1,long int v1,long int v2)
{
  /* assert that lower and upper bound are in right order */
  if(v1<=v2)
    {
      r1->from=v1;
      r1->to=v2;
    }
  else
    {
      r1->from=v2;
      r1->to=v1;
    }
  return 0;
}

int
bridge_range_check(bridge_range_t* r1)
{
  if( (r1->from) <= (r1->to) )
    return 1;
  else
    return 0;
}

int
_bridge_range_compare(const void* a1,const void* a2){
  bridge_range_t* r1=(bridge_range_t*) a1;
  bridge_range_t* r2=(bridge_range_t*) a2;  
  return bridge_range_compare(r1,r2);
}

int
bridge_range_compare(bridge_range_t* r1,bridge_range_t* r2)
{
  int fstatus=0;
  if(!bridge_range_check(r1) || !bridge_range_check(r2))
    return fstatus;
  if(r1->from==r2->from && r1->to==r2->to)
    return 0;
  else if(r1->to < r2->from)
    return -1;
  else
    return 1;
  return fstatus;
}

int
bridge_range_intersects(bridge_range_t* r1,bridge_range_t* r2)
{
  int fstatus=0;
  if(!bridge_range_check(r1) || !bridge_range_check(r2))
    return fstatus;
  if(r2->to==r2->from)
    if(r1->from<=r2->to && r2->to<=r1->to)
      fstatus=1;
  if(r1->to==r1->from)
    if(r2->from<=r1->to && r1->to<=r2->to)
      fstatus=1;
  if(r2->to>=r1->from && r2->from<=r1->to)
    fstatus=1;
  if(r1->to>=r2->from && r1->from<=r2->to)
    fstatus=1;
  return fstatus;
}

int
bridge_range_intersection(bridge_range_t* r1,bridge_range_t* r2,bridge_range_t* rout)
{
  int fstatus=-1;
  if(!bridge_range_check(r1) || !bridge_range_check(r2))
    return fstatus;
  if(bridge_range_intersects(r1,r2)){
    rout->from =( r1->from > r2->from )?r1->from:r2->from;
    rout->to =( r1->to < r2->to )?r1->to:r2->to;
    fstatus=0;
  }
  return fstatus;
}

int
bridge_range_contiguous(bridge_range_t* r1,bridge_range_t* r2)
{
  int fstatus=-1;
  if(!bridge_range_check(r1) || !bridge_range_check(r2))
    return fstatus;
  if( (r1->to + 1) != r2->from && (r1->from - 1) != r2->to)
    fstatus=0;
  else if( r1->to < r2->from )
    fstatus=1;
  else
    fstatus=2;
  return fstatus;
}

int
bridge_range_includes(bridge_range_t* r1,bridge_range_t* r2)
{
  int fstatus=-1;
  if(!bridge_range_check(r1) || !bridge_range_check(r2)){
    return fstatus;
  }
  if( r2->from >= r1->from && r2->to <= r1->to)
    fstatus=1;
  else if(r1->from >= r2->from && r1->to <= r2->to)
    fstatus=2;
  else
    fstatus=0;
  return fstatus;
}

int
bridge_range_union(bridge_range_t* r1,bridge_range_t* r2,bridge_range_t* rout)
{
  int fstatus=-1;
  if(!bridge_range_check(r1) || !bridge_range_check(r2))
    return fstatus;
  if(!bridge_range_intersects(r1,r2))
    {
    if(!bridge_range_contiguous(r1,r2))
      return fstatus;
    }
  rout->from =( r1->from < r2->from )?r1->from:r2->from;
  rout->to =( r1->to > r2->to )?r1->to:r2->to;
  fstatus=0;
  return fstatus;
}

int
bridge_rangelist_init(bridge_rangelist_t* array)
{
  int fstatus=-1;
  array->pre_allocated_ranges=DEFAULT_RANGELIST_SIZE;
  array->ranges_nb=0;
  array->array=(bridge_range_t*)malloc(array->pre_allocated_ranges*sizeof(bridge_range_t));
  if(array->array!=NULL)
    {
      fstatus=0;
    }
  else
    {
      array->pre_allocated_ranges=0;
    }
  return fstatus;
}

int
bridge_rangelist_init_by_copy(bridge_rangelist_t* array,bridge_rangelist_t* a2c)
{
  int fstatus=-11;
  int i;
  array->pre_allocated_ranges=a2c->pre_allocated_ranges;
  array->ranges_nb=a2c->ranges_nb;
  array->array=(bridge_range_t*)malloc(array->pre_allocated_ranges*sizeof(bridge_range_t));
  if(array->array!=NULL)
    {
      for(i=0;i<array->ranges_nb;i++)
	{
	  memcpy(((array->array)+i),((a2c->array)+i),sizeof(bridge_range_t));
	}
      fstatus=0;
    }
  else
    {
      array->pre_allocated_ranges=0;
    }
  return fstatus;
}

int
bridge_rangelist_free_contents(bridge_rangelist_t* array)
{
  int fstatus=0;
  array->pre_allocated_ranges=0;
  array->ranges_nb=0;
  if(array->array!=NULL){
    free(array->array);
    array->array=NULL;
  }
  return fstatus;
}

int
bridge_rangelist_incremente_size(bridge_rangelist_t* array)
{
  
  int fstatus=-1;
  array->pre_allocated_ranges+=DEFAULT_RANGELIST_INC_SIZE;
  array->array=(bridge_range_t*)realloc(array->array,array->pre_allocated_ranges*sizeof(bridge_range_t));
  if(array->array!=NULL)
    fstatus=0;
  return fstatus;
}

int
bridge_rangelist_add_range(bridge_rangelist_t* array,bridge_range_t* rin)
{
  int fstatus=-1;
  int already_added_flag=0;
  long int id;

  bridge_range_t r;
  bridge_rangelist_t work_array;

  memcpy(&r,rin,sizeof(bridge_range_t));
  if(array->ranges_nb==0)
    {
      memcpy(array->array,&r,sizeof(bridge_range_t));
      array->ranges_nb++;
      fstatus=0;
    }
  else
    {
      /* test if range is already present */
      for(id=0;id<array->ranges_nb;id++)
	{
	  already_added_flag=bridge_range_includes(&(array->array[id]),&r);
	  if(already_added_flag==1)
	    break;
	  already_added_flag=0;
	}
      /* add range if not already present */
      if(already_added_flag)
	{
	  fstatus=0;
	}
      else
	{
	  /* initialize working ranges array */
	  bridge_rangelist_init(&work_array);
	  /* process sequentially input ranges array 's ranges*/
	  for(id=0;id<array->ranges_nb;id++)
	    {
	      /* if range to add doesn't intersect or is not contiguous to currently tested range */
	      /* of the input ranges array, we add it to working ranges array */
	      /* otherwise, we merge it with current tested range of the input ranges array and go */
	      /* to the next range */
	      if(!bridge_range_intersects(&(array->array[id]),&r) && !bridge_range_contiguous(&(array->array[id]),&r))
		{
		  if(work_array.ranges_nb==work_array.pre_allocated_ranges)
		    bridge_rangelist_incremente_size(&work_array);
		  memcpy(work_array.array+work_array.ranges_nb,&(array->array[id]),sizeof(bridge_range_t));
		  work_array.ranges_nb++;
		}
	      else
		{
		  bridge_range_union(&(array->array[id]),&r,&r);
		}
            }
	  /* add range to add (which may be bigger now ) */
	  if(work_array.ranges_nb==work_array.pre_allocated_ranges)
	    bridge_rangelist_incremente_size(&work_array);
	  memcpy(work_array.array+work_array.ranges_nb,&r,sizeof(bridge_range_t));
	  work_array.ranges_nb++;
	  bridge_rangelist_sort(&work_array);

	  bridge_rangelist_free_contents(array);
	  fstatus=bridge_rangelist_init_by_copy(array,&work_array);
	}
    }

  return fstatus;
}

int
bridge_rangelist_add_rangelist(bridge_rangelist_t* array,bridge_rangelist_t* rlin)
{
  int fstatus=0;
  
  int i;

  for(i=0;i<rlin->ranges_nb;i++){
    fstatus+=bridge_rangelist_add_range(array,&(rlin->array[i]));
  }

  return fstatus;
}

int
bridge_rangelist_remove_range(bridge_rangelist_t* array,bridge_range_t* rin)
{
  int fstatus=-1;
  int intersects_flag=0;
  long int id;

  bridge_range_t* pr;
  bridge_range_t r;
  bridge_range_t wr1;
  bridge_rangelist_t work_array;

  if(array->ranges_nb==0)
    {
      fstatus=0;
    }
  else
    {
      memcpy(&r,rin,sizeof(bridge_range_t));
      /* initialize working ranges array */
      bridge_rangelist_init(&work_array);
      /* test if range intersects with this rangelist */
      fstatus=0;
      for(id=0;id<array->ranges_nb;id++)
	{
	  pr=&(array->array[id]);
	  intersects_flag=bridge_range_intersects(pr,&r);
	  if(!intersects_flag){
	    /* add this range to work array */
	    fstatus+=bridge_rangelist_add_range(&work_array,pr);
	  }
	  else{
	    /* extract any hypothetic non intersecting part of the range */
	    /* and add them to work_array range list*/	    
	    if(pr->from!=pr->to){
	      /* check that second range doesn't include the first one */
	      if(bridge_range_includes(&r,pr)!=1){
		/* [pr[r... */
		if(pr->from<r.from){
		  bridge_range_set(&wr1,pr->from,r.from-1);
		  fstatus+=bridge_rangelist_add_range(&work_array,&wr1);
		}
		/* ...r]pr] */
		if(pr->to>r.to){
		  bridge_range_set(&wr1,r.to+1,pr->to);
		  fstatus+=bridge_rangelist_add_range(&work_array,&wr1);
		}

	      }
	      /*_*/
	    }
	    /*_*/
	      
	  }

	  if(fstatus)
	    break;

	}

      /* success, replace array with the new range list */
      if(fstatus==0){
	bridge_rangelist_free_contents(array);
	fstatus=bridge_rangelist_init_by_copy(array,&work_array);
      }

      bridge_rangelist_free_contents(&work_array);
    }

  return fstatus;
}

int
bridge_rangelist_remove_rangelist(bridge_rangelist_t* array,bridge_rangelist_t* rlin)
{
  int fstatus=0;
  
  int i;

  for(i=0;i<rlin->ranges_nb;i++){
    fstatus+=bridge_rangelist_remove_range(array,&(rlin->array[i]));
  }

  return fstatus;
}

int
bridge_rangelist_add_list(bridge_rangelist_t* array,char* list)
{
  int fstatus=0;
  char* in_list;
  size_t in_list_size;
  char* work_buffer;
  char* begin;
  char* end;

  long int start_val;
  long int value;
  long int work_val;
  int start_flag=0;
  int stop_flag=0;

  int padding=0;

  in_list=list;
  in_list_size=strlen(in_list);

  /* create working buffer */
  work_buffer=(char*)malloc(in_list_size+1);
  if(work_buffer==NULL)
    {
      fstatus=-1;
    }
  else
    {
    /* set entry point */
    begin=in_list;
    if(*begin=='[')
      begin++;
    end=begin;
    
    /* process input list */
    while(*end!='\0' && end<in_list+in_list_size+1)
      {
	while(isdigit(*end))
	  end++;
	/* do something only if end was incremented */
	if(end-begin)
	  {
	    /* extract the read value */
	    strncpy(work_buffer,begin,(end-begin));
	    work_buffer[end-begin]='\0';
	    /* get long int value and test its validity */
	    value=strtoll(work_buffer,NULL,10);
	    if(value==LONG_MIN || value==LONG_MAX)
	      {
		fstatus=2;
		break;
	      }
	    /* try to get padding */
	    if(*work_buffer=='0'){
	      int max_length=strlen(work_buffer);
	      if(max_length>padding){
		padding=max_length;
	      }
	    }

	    /* check how many value must be added */
	    if(*end=='\0' || *end==',' || *end==']')
	      {
		if(!start_flag)
		  {
		    start_flag=1;
		    start_val=value;
		  }
		stop_flag=1;
	      }
	    /* current lemme is a range */
	    else if(*end=='-')
	      {
		start_flag=1;
		start_val=value;
	      }
	    /* current lemme has a invalid format */
	    else
	      {
		fstatus=3;
		break;
	      }
	
	    /* test if value(s) must be added now */
	    if(start_flag && stop_flag)
	      {
		if(value<start_val)
		  {
		    work_val=start_val;
		    start_val=value;
		    value=work_val;
		  }
		/* add value(s) */
		bridge_range_t br;
		bridge_range_set(&br,start_val,value);
		bridge_rangelist_add_range(array,&br);
		start_flag=0;
		stop_flag=0;
	      }
	  }
	/* go to next lemme */
	end++;
	begin=end;
      }
    
    /* free working buffer */
    free(work_buffer);
    
    }

  /* at this point fstatus=0 if process was done successfully, we may update it to padding value */
  if(fstatus!=0)
    fstatus=-1;
  else
    fstatus=padding;
	     
  return fstatus;
}

int
bridge_rangelist_sort(bridge_rangelist_t* array)
{
  int fstatus=-1;
  qsort(array->array,array->ranges_nb,sizeof(bridge_range_t),_bridge_range_compare);
  return fstatus;
}

int bridge_rangelist_intersects(bridge_rangelist_t* a1,bridge_rangelist_t* a2)
{
  int fstatus=0;
  int i,j;
  for(i=0;i<a1->ranges_nb;i++){
    for(j=0;j<a2->ranges_nb;j++){
      fstatus=bridge_range_intersects(&(a1->array[i]),&(a2->array[j]));
      if(fstatus)
	break;
    }
    if(fstatus)
      break;
  }
  return fstatus;
}

int
bridge_rangelist_includes(bridge_rangelist_t* a1,bridge_rangelist_t* a2)
{
  int fstatus=0;
  int i,j;
  int valid_ranges_nb=0;
  for(i=0;i<a2->ranges_nb;i++)
    {
    for(j=0;j<a1->ranges_nb;j++)
      {
	if(bridge_range_includes(&(a1->array[j]),&(a2->array[i]))==1)
	  {
	    valid_ranges_nb++;
	    break;
	  }
      }
    }
  if(valid_ranges_nb==a2->ranges_nb)
    fstatus=1;
  return fstatus;
}

int
bridge_rangelist_show(bridge_rangelist_t* array)
{
  long int id;
  fprintf(stdout,"----------------------------------\n");
  fprintf(stdout,"Ranges nb : %ld\n",array->ranges_nb);
  for(id=0;id<array->ranges_nb;id++)
    {
      fprintf(stdout,"[%ld-%ld]\n",array->array[id].from,array->array[id].to);
    }
  fprintf(stdout,"----------------------------------\n");

  return 0;
}
