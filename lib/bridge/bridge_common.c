/*****************************************************************************\
 *  lib/bridge/bridge_common.c - 
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

#define _GNU_SOURCE

#include "config.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <string.h>

#include "bridge_common.h"

#include "xternal/xlogger.h"
#define DEBUG_LOGGER xdebug
#define DEBUG2_LOGGER xdebug2
#define DEBUG3_LOGGER xdebug3
#define ERROR_LOGGER xerror

/* local functions */
int _bridge_common_string_get_token_common(char* string,char* separators_list,
					   int* p_token_nb,int token_id,
					   char** p_token);

/*
  ----------------------------------------------------------------------------------------------------------------
  bridge_common_string_get_tokens
  -------------------------------
*/
int bridge_common_string_get_token(char* string,char* separators_list,int token_id,char** p_token){
  int fstatus=-1;
  int token_nb=0;
  fstatus=_bridge_common_string_get_token_common(string,separators_list,&token_nb,token_id,p_token);
  if(*p_token!=NULL)
    fstatus=0;
  else
    fstatus=-1;
  return fstatus;
}
/*
  ----------------------------
  bridge_common_string_get_tokens
  ----------------------------------------------------------------------------------------------------------------
*/


/*
  ----------------------------------------------------------------------------------------------------------------
  bridge_common_string_get_tokens_quantity
  -------------------------------------
*/
int bridge_common_string_get_tokens_quantity(char* string,char* separators_list,int* p_token_nb){
  int fstatus=-1;
  fstatus=_bridge_common_string_get_token_common(string,separators_list,p_token_nb,0,NULL);
  return fstatus;
}
/*
  -------------------------------------
  bridge_common_string_get_tokens_quantity
  ----------------------------------------------------------------------------------------------------------------
*/

/*
  ----------------------------------------------------------------------------------------------------------------
  appends_and_extends_string
  --------------------------

  Appends a char* giving a char* to append and an optionnal separator (NULL if no separator)

  char** p_io_string : pointer on a char* that will be appended and maybe extended if no enough memory allocated
  size_t* p_current_length : pointer on a size_t structure that contains the current size of the char* that will be write
  size_t inc_length : the incrementation step that could be use to extend char* memory allocation
  char* string2append : string to append
  char* separator : separator to put between current char* and sting to append

  return 0 if it succeeds
  -1 otherwise
  ----------------------------------------------------------------------------------------------------------------
*/
int bridge_common_string_appends_and_extends(char** p_io_string,size_t* p_current_length,size_t inc_length,char* string2append,char* separator){
  int fstatus=-1;

  size_t new_output_length;
  size_t new_string_length;
  
  size_t output_string_length;
  size_t separator_length;
  size_t append_length;

  char* default_separator=" ";
  char* local_separator;

  if(*p_io_string!=NULL && string2append!=NULL){

    if(separator!=NULL)
      local_separator=separator;
    else
      local_separator=default_separator;

    if(strlen(*p_io_string)==0)
      local_separator="";

    output_string_length=strlen(*p_io_string);
    separator_length=strlen(local_separator);
    append_length=strlen(string2append);
    new_string_length=output_string_length+separator_length+append_length;
    if(new_string_length>*p_current_length){
      new_output_length=*p_current_length;
      while(new_string_length>new_output_length)
	new_output_length+=inc_length;
      *p_io_string=(char*)realloc(*p_io_string,(new_output_length+1)*sizeof(char));
      if(*p_io_string!=NULL)
	*p_current_length=new_output_length;
      else
	*p_current_length=0;
    }


    if(*p_io_string!=NULL){
      
      strncpy(*p_io_string+output_string_length,local_separator,separator_length);
      strncpy(*p_io_string+output_string_length+separator_length,string2append,append_length);
      *(*p_io_string+output_string_length+separator_length+append_length)='\0';
      fstatus=0;	
    }


  }

  return fstatus;
}
/*
  --------------------------
  appends_and_extends_string
  ----------------------------------------------------------------------------------------------------------------
*/


/*
  ----------------------------------------------------------------------------------------------------------------
  bridge_common_extended2condensed_nodelist
  -------------------------------------------

  Construit une liste de noeuds condensee a partir d'une liste de noeuds etendue

  code retour :
  le nombre de noeud si ok
  -1 si erreur

  Rq :  il est necessaire de liberer la memoire associee a *p_dst_list via un appel a free une fois son 
  utilisation terminee
  ----------------------------------------------------------------------------------------------------------------
*/
int bridge_common_extended2condensed_nodelist(char* src_list,char** p_dst_list){

  int fstatus=-1,status;

  bridge_nodelist_t nodelist;

  status=bridge_nodelist_init(&nodelist,&src_list,1);
  if(status==0){
    if(bridge_nodelist_get_compacted_string(&nodelist,p_dst_list)==0)
      fstatus=bridge_nodelist_nodes_quantity(&nodelist);
    else
      fstatus=-1;
    bridge_nodelist_free_contents(&nodelist);
  }
  else{
    fstatus=-1;
  }

  return fstatus;

}
/*
  -------------------------------------------
  bridge_common_extended2condensed_nodelist
  ----------------------------------------------------------------------------------------------------------------
*/



/*
  ----------------------------------------------------------------------------------------------------------------
  bridge_common_condensed2extended_nodelist
  -------------------------------------------

  Construit une liste de noeuds etendue a partir d'une liste de noeuds condensee

  code retour :
  le nombre de noeud si ok
  -1 si erreur

  Rq :  il est necessaire de liberer la memoire associee a *p_dst_list via un appel a free une fois son 
  utilisation terminee
  ----------------------------------------------------------------------------------------------------------------
*/
int bridge_common_condensed2extended_nodelist(char* src_list,char** p_dst_list){

  int fstatus,status;

  bridge_nodelist_t nodelist;

  status=bridge_nodelist_init(&nodelist,&src_list,1);
  if(status==0){
    if(bridge_nodelist_get_extended_string(&nodelist,p_dst_list)==0)
      fstatus=bridge_nodelist_nodes_quantity(&nodelist);
    else
      fstatus=-1;
    bridge_nodelist_free_contents(&nodelist);
  }
  else{
    fstatus=-1;
  }

  return fstatus;
}
/*
  -------------------------------------------
  bridge_common_condensed2extended_nodelist
  ----------------------------------------------------------------------------------------------------------------
*/



/*
  ----------------------------------------------------------------------------------------------------------------
  PRIVATE PRIVATE PRIVATE PRIVATE PRIVATE PRIVATE PRIVATE PRIVATE PRIVATE PRIVATE PRIVATE PRIVATE 
  ----------------------------------------------------------------------------------------------------------------
*/
int _bridge_common_string_get_token_common(char* string,char* separators_list,int* p_token_nb,int token_id,char** p_token){
  int fstatus=-1;

  int i;

  size_t string_length;
  size_t separators_list_length;

  char* working_string;

  char* current_pointer;
  char* best_pointer;
  char* old_pointer;

  size_t copy_length;

  int local_token_nb;
  int end_of_loop;

  /*
    First we check that pointers are not NULL
  */
  if(string!=NULL && separators_list!=NULL){
    string_length=strlen(string);
    separators_list_length=strlen(separators_list);
    /*
      Then, that their lengths are not null
    */
    if(string_length!=0 && separators_list_length!=0){
      /*
	Then, the separators research loop start
      */
      working_string=string;
      old_pointer=working_string;
      local_token_nb=1;
      end_of_loop=0;
      while(!end_of_loop){
	best_pointer=NULL;
	/*
	  Search the first occurence of a separator
	*/
	for(i=0;i<separators_list_length;i++){
	  current_pointer=strchr(working_string,*(separators_list+i));
	  if(best_pointer==NULL){
	    best_pointer=current_pointer;
	  }
	  else if(best_pointer>current_pointer && current_pointer!=NULL){
	    best_pointer=current_pointer;
	  }
	}
	/*
	  If this token must be extracted, extract it
	*/
	if(token_id==local_token_nb && (*p_token)==NULL){
	  if(best_pointer==NULL)
	    copy_length=strlen(old_pointer);
	  else
	    copy_length=(size_t)(best_pointer-old_pointer);
	  *p_token=(char*)malloc((copy_length+1)*sizeof(char));
	  if(*p_token!=NULL){
	    (*p_token)[copy_length]='\0';
	    strncpy(*p_token,old_pointer,copy_length);
	    fstatus++;
	  }
	  else{
	    fstatus=-2;
	  }
	}
	/*
	  If no more occurences, break the loop
	*/
	if(best_pointer==NULL){
	  end_of_loop=1;
	}
	/*
	  Otherwise, increment token counter and adjust working string
	*/
	else{
	  local_token_nb++;
	  working_string=best_pointer+1;
	  old_pointer=working_string;
	}
      }
      *p_token_nb=local_token_nb;
      fstatus++;
    }
  }

  return fstatus;
}
