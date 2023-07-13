/*****************************************************************************\
 *  lib/confparse/analyse.c -
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
 * \file    analyse.c
 * \author  $Author: deniel $
 * \date    $Date: 2005/11/28 17:02:31 $
 * \version	$Revision: 1.5 $ 
 * \brief   Building the syntax tree.
 *
 * Build the structure that represents a config file.
 *
 * CVS History :
 *
 * $Log: analyse.c,v $
 * Revision 1.5  2005/11/28 17:02:31  deniel
 * Added CeCILL headers
 *
 * Revision 1.4  2005/04/22 07:42:05  leibovic
 * Adding configuration print function.
 *
 * Revision 1.3  2005/04/19 13:59:56  leibovic
 * Modifying trace.
 *
 * Revision 1.2  2005/04/18 10:42:02  leibovic
 * Developing config file reading.
 *
 * Revision 1.1  2005/04/15 15:01:49  leibovic
 * Initial version for config file parsing.
 *
 *
 */

#include "analyse.h"
#include <stdlib.h>
#include <stdio.h>
/**
 *  Creation d'une list de blocks
 */
list_block * config_createlistblock(){

    list_block * new = (list_block *)malloc(sizeof(list_block));
    (*new)=NULL;
    return new;

}

/**
 *  Creation d'un block
 */
type_block * config_createblock(char * blockname,list_affect * list){
    
    type_block * new = (type_block *)malloc(sizeof(type_block));
    strncpy(new->name,blockname,MAXSTRLEN);
    new->list_def=(*list);
    new->next=NULL;
    
    return new;
    
}

/**
 *  Ajout d'un block a une list de blocks
 */
void config_addblock( list_block * list,type_block * block){
    
    if (!(*list)){
        (*list)=block;
    } else {
        block->next=(*list);
        (*list)=block;
    }
    
}


/**
 *  Creation d'une list d'affectations
 */
list_affect * config_createlistaffect(){
    
    list_affect * new = (list_affect *)malloc(sizeof(list_affect));
    (*new)=NULL;
    return new;

}

/**
 *  Creation d'une definition variable=valeur
 */
type_affect * config_createaffect(char * varname,char * varval){
    
    type_affect * new = (type_affect *)malloc(sizeof(type_affect));
    strncpy(new->varname,varname,MAXSTRLEN);
    strncpy(new->varvalue,varval,MAXSTRLEN);
    new->next=NULL;
    
    return new;

}


/**
 *  Ajout d'une definition a une list d'affectations
 */
void config_adddef(list_affect * list,type_affect * affect){
    if (!(*list)){
        (*list)=affect;
    } else {
        affect->next=(*list);
        (*list)=affect;
}

    }


/**
*   Affichage idente du contenu d'une list de blocks.
*/
void config_print_list( FILE * output, list_block * list ){
    
    type_block * curr_block;
    type_affect * curr_aff;
    
    /* sanity check */
    if (!list) return;
    
    curr_block=(*list);
    
    while (curr_block){
        
        fprintf(output,"BLOCK '%s'\n",curr_block->name);
        
        curr_aff=curr_block->list_def;
        
        while (curr_aff){
            
            fprintf(output,"\t KEY : '%s', VALUE : '%s'\n",curr_aff->varname,curr_aff->varvalue);
            
            curr_aff=curr_aff->next;
        }
        
        fprintf(output,"\n");
        
        curr_block=curr_block->next;
    }
    
        
}



static void free_block( type_block * block ){
  
    type_affect * curr_aff;
    type_affect * next_aff;
    
    curr_aff = block->list_def;
    
    while (curr_aff){
      
      next_aff = curr_aff->next;
      free( curr_aff );
      curr_aff = next_aff;
          
    }
    
    free( block );
    
    return ;
  
}



/**
 * config_free_list:
 * libere les ressources utilisees par une liste de blocks.
 */
void config_free_list(list_block * list){
  
    type_block * curr_block;
    type_block * next_block;
    
    curr_block=(*list);
    
    while (curr_block){
      
      next_block = curr_block->next;      
      free_block( curr_block );
      curr_block = next_block;
        
    }
    
    free( list );
    return ;
  
}












