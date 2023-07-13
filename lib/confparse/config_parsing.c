/*****************************************************************************\
 *  lib/confparse/config_parsing.c -
 *****************************************************************************
 *  Copyright  CEA/DAM/DIF (2005)
 *
 *  Written by : Philippe DENIEL   philippe.deniel@cea.fr
 *               Thomas LEIBOVICI  thomas.leibovici@cea.fr
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

/*
 * \file    parsing.c
 * \author  $Author: leibovic $
 * \date    $Date: 2006/01/17 14:21:00 $
 * \version	$Revision: 1.7 $ 
 * \brief   Configuration file parsing routines.
 *
 * CVS History:
 *
 * $Log: config_parsing.c,v $
 * Revision 1.7  2006/01/17 14:21:00  leibovic
 * Fixing init bug.
 *
 * Revision 1.6  2005/11/28 17:02:31  deniel
 * Added CeCILL headers
 *
 * Revision 1.5  2005/04/22 07:42:05  leibovic
 * Adding configuration print function.
 *
 * Revision 1.4  2005/04/22 07:26:58  leibovic
 * Configuration becomes case unsensitive.
 *
 * Revision 1.3  2005/04/19 14:48:33  leibovic
 * Correcting Bug.
 *
 * Revision 1.2  2005/04/18 10:42:02  leibovic
 * Developping config file reading.
 *
 * Revision 1.1  2005/04/15 15:01:49  leibovic
 * Initial version for config file parsing.
 */

#include "config_parsing.h"
#include "analyse.h"
#include <stdio.h>
#include <errno.h>
#include <string.h>

/* case unsensitivity */
#define STRNCMP   strncasecmp


typedef struct config_struct_t
{
  
  /* Syntax tree */
  
  list_block * syntax_tree ;

  /* cache pour l'optimisation du parcours de la liste */

  /* cache le nombre d'elements */
  int cache_nb_items ;
  
  /* cache pour optimiser l'acces a la liste chainee des items */
  type_block  * last_item_ptr ;
  int           last_item_index ;

} config_struct_t ;



/***************************************
 * ACCES AUX VARIABLES EXTERNES
 ***************************************/
 
/* fichier d'entree du lexer */
extern FILE * yyin;
/* routine de parsing */
extern int yyparse();
/* variable renseignee lors du parsing */
extern list_block * program_result;
/* message d'erreur */
extern char extern_errormsg[1024];


/* config_ParseFile:
 * Reads the content of a configuration file and
 * stores it in a memory structure.
 */   
config_file_t config_ParseFile(  char * file_path ){
  
  FILE * configuration_file;
  config_struct_t * output_struct;
  
  /* Inits error message */
  
  extern_errormsg[0] = '\0';
  
    
  /* Sanity check */
  
  if ( !file_path || !file_path[0] ){
    strcpy( extern_errormsg, "Invalid arguments" );
    return NULL;
  }
    
  
  /* First, opens the file. */
  
  configuration_file = fopen( file_path, "r" );
  
  if ( !configuration_file ){
    strcpy( extern_errormsg, strerror(errno) );
    return NULL;
  }
  
    
  /* Then, parse the file. */
  
  yyin = configuration_file;
  if (yyparse()){
    goto error_exit;
  }  

  /** @todo : yyparse fait exit en cas d'erreur. Remedier au probleme. */
    
  
  /* Finally, build the output struct. */
  
  output_struct = (config_struct_t *)malloc(sizeof(config_struct_t));
  
  if (!output_struct){
    strcpy( extern_errormsg, strerror(errno) );
    goto error_exit;
  }
  
  output_struct->cache_nb_items = -1;
  output_struct->last_item_ptr  = NULL;
  output_struct->last_item_index  = -1;
      
  output_struct->syntax_tree = program_result;
  
  fclose(configuration_file);

  /* converts pointer to pointer */
  return (config_file_t) output_struct;
    
  error_exit :

    fclose(configuration_file);

  return NULL;
}



/* If config_ParseFile returns a NULL pointer,
 * config_GetErrorMsg returns a detailed message
 * to indicate the reason for this error.
 */
char * config_GetErrorMsg(){

  return extern_errormsg;
  
}




/**
 * config_Print:
 * Print the content of the syntax tree
 * to a file.
 */
void config_Print( FILE * output, config_file_t config ){
  
    /* sanity check */
    if (!config) return ;
    
    config_print_list( output, ((config_struct_t *)config)->syntax_tree );
    
  
}






/** 
 * config_Free:
 * Free the memory structure that store the configuration.
 */

void config_Free( config_file_t config ){
  
  config_struct_t * config_struct = (config_struct_t *)config ;
  
  if ( !config_struct ) return;
  
  config_free_list( config_struct->syntax_tree );
  
  free( config_struct );
  
  return ;
  
}




/**
 * config_GetNbBlocks:
 * Indicates how many blocks are defined into the config file.
 */
int config_GetNbBlocks( config_file_t config ){
  
  config_struct_t * config_struct = (config_struct_t *)config ;
  
  if ( !config_struct ) return -EFAULT;
  
  
  /* si le cache est renseigne, on renvoie la valeur */
  if (config_struct->cache_nb_items != -1)
    return config_struct->cache_nb_items;

  /* on regarde si la liste est vide */
  if ( !(config_struct->syntax_tree) ) {
      config_struct->cache_nb_items = 0;
      return 0;
  }
  /* on compte le nombre d'elements */
  else
  {
      /* il y a au moins un element : le premier */
      type_block * curr_block = (*config_struct->syntax_tree);
      int nb = 1;

      /*
       * on en profite pour renseigner le cache
       * des blocks
       */
      config_struct->last_item_ptr = curr_block;
      config_struct->last_item_index  = 0;

      while (curr_block = curr_block->next){nb++;}

      /* on cache le nbr de blocks et on renvoie ce nombre */
      config_struct->cache_nb_items = nb;
      return nb;
  }
    
    
}


/* utilisation interne :
 * charge un element dans le cache.
 * \return <>0 si le'element n'est pas trouve.
 */
static int load_item_into_cache( config_struct_t * config_struct, int item_no )
{
    
    /* Sanity checks */
    if (!config_struct || !config_struct->syntax_tree)
      return -1;
    
    /* On verifie que l'index est correct */
    if ( (item_no<0)
         ||(item_no >= config_GetNbBlocks((config_file_t)config_struct)) )
      return -1;
    
    /* L'element est-il deja dans le cache ? */
    if (item_no == config_struct->last_item_index)
      return 0;
    
    /* L'element n'est pas dans le cache, on le cherche */
    /*else*/{
        /* premier element */
        type_block * curr_block = (*config_struct->syntax_tree);
        int index = 0;
        
        while (index != item_no){
            curr_block=curr_block->next;
            if (!curr_block) return -1; /* on est arrive a la fin ss trouver*/
            index++;
        }
        
        if (index == item_no){
            /* on le memorise ds le cache */
            config_struct->last_item_index = index;
            config_struct->last_item_ptr = curr_block;
            return 0;
        } else {
            /* pas normal du tout*/
            return -1;
        }
    }

}




/**
 * config_GetBlockIndexByName:
 * Returns the index of the block with the specified name.
 */
int config_GetBlockIndexByName( config_file_t config,
                                char * block_name )
{

    config_struct_t * config_struct = (config_struct_t *)config ;
    
    /* L'element est-il deja dans le cache ? */
    
    if ( ( config_struct->last_item_ptr != NULL )
         && !STRNCMP( config_struct->last_item_ptr->name , block_name,MAXSTRLEN ) )
       return config_struct->last_item_index ;
    
    
    /* L'element n'est pas dans le cache, on le cherche */
    /*else*/{
        /* premier element */
        type_block * curr_block = (*config_struct->syntax_tree);
        int index = 0;
        
        while (STRNCMP( curr_block->name , block_name,MAXSTRLEN )){
            curr_block=curr_block->next;
            if (!curr_block) return -1; /* on est arrive a la fin ss trouver*/
            index++;
        }
        
        if (!STRNCMP( curr_block->name , block_name,MAXSTRLEN )){
            /* on le memorise ds le cache */
            config_struct->last_item_index = index;
            config_struct->last_item_ptr = curr_block;
            return index;
        } else {
            /* pas normal du tout*/
            return -1;
        }
    }
      
}




/**
 * config_GetBlockName:
 * Indicates the name of a block (specified with its index).
 */

char * config_GetBlockName( config_file_t config , int block_no )
{
  config_struct_t * config_struct = (config_struct_t *)config ;
  
  /* met l'element dans le cache */
  
  if (load_item_into_cache(config_struct,block_no))
    /* sort en cas d'erreur*/
    return NULL;
  else
    /* renvoie le nom sinon*/
    return config_struct->last_item_ptr->name;
  
}




/**
 * config_GetNbKeys:
 * Indicates how many peers (key-value) are defined in a block
 * (specified with its index).
 */

int config_GetNbKeys( config_file_t config, int block_no )
{
  config_struct_t * config_struct = (config_struct_t *)config ;
  
  /* met l'element dans le cache */
  if (load_item_into_cache(config_struct,block_no))
    return -1; /* sort en cas d'erreur*/
  else
  {
      /* recupere la premiere variable */
      type_affect * curr_def = config_struct->last_item_ptr->list_def;
      int nb_vars = 0;

      while (curr_def){
          nb_vars++;
          curr_def=curr_def->next;
      }
      /* elements trouves */
      return nb_vars;

  }
  
}




/**
 * config_GetKeyValue:
 * Retrieves a key-value peer from the block index and the key index.
 */

int config_GetKeyValue( 
                        config_file_t config,
                        int block_no,
                        int key_no,
                        char ** var_name,
                        char ** var_value
                       )
{

    config_struct_t * config_struct = (config_struct_t *)config ;  

    /* met l'element dans le cache */
    if ( load_item_into_cache( config_struct, block_no ) )
      return -1; /* sort en cas d'erreur*/
    
    /* cherche la variable dont l'index est celui donne en parametre */
    /*else*/
    {
        /* recupere la premiere variable */
        type_affect * curr_def = config_struct->last_item_ptr->list_def;
        int index=0;
        
        if (!curr_def) return -1;
                
        while (index != key_no){
            index++;
            curr_def=curr_def->next;
            if (!curr_def) return -1; /* on est arrives a la fin ss trouver*/
        }
        
        if (index == key_no){
            (*var_name) = curr_def->varname;
            (*var_value) = curr_def->varvalue;
            return 0;
        } else {
            /* pas trouve*/
            return -1;
        }
            
    }
  
}




/**
 * config_GetKeyValueByName:
 * Returns the value of the key with the specified name.
 */

char * config_GetKeyValueByName( config_file_t config,
                                 int block_no,
                                 char * key_name )
{
  
    config_struct_t * config_struct = (config_struct_t *)config ;  

    /* met l'element dans le cache */
    if ( load_item_into_cache( config_struct, block_no ) )
      return NULL; /* sort en cas d'erreur*/
  
    /* cherche la variable dont le nom est celui donne en parametre */
    /*else*/
    {
        /* recupere la premiere variable */
        type_affect * curr_def = config_struct->last_item_ptr->list_def ;
                
        while (curr_def){
            if (!STRNCMP(curr_def->varname,key_name,MAXSTRLEN)){
                return curr_def->varvalue;
            } else {
                curr_def=curr_def->next;
            }
        }
        /* pas d'element trouve */
        return NULL;        
            
    }
    
}







