/*****************************************************************************\
 *  lib/confparse/analyse.h - 
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

/**
 *
 * \file    analyse.h
 * \author  $Author: leibovic $
 * \date    $Date: 2005/04/22 07:42:05 $
 * \version	$Revision: 1.3 $ 
 * \brief   Building the syntax tree.
 *
 * Build the structure that represents a config file.
 *
 * CVS History :
 *
 * $Log: analyse.h,v $
 * Revision 1.3  2005/04/22 07:42:05  leibovic
 * Adding configuration print function.
 *
 * Revision 1.2  2005/04/18 10:42:02  leibovic
 * Developing config file reading.
 *
 * Revision 1.1  2005/04/15 15:01:49  leibovic
 * Initial version for config file parsing.
 *
 *
 */


#ifndef CONFPARSER_H
#define CONFPARSER_H

#include <string.h>
#include <stdio.h>


#define MAXSTRLEN   1024

typedef struct _type_affect_ {
    char varname[MAXSTRLEN];
    char varvalue[MAXSTRLEN];
    struct _type_affect_ * next; /* chained list */
}type_affect;

typedef type_affect * list_affect;

typedef struct _type_block_ {
    
    char name[MAXSTRLEN];
    list_affect list_def; /* list of blockk definitions */
    struct _type_block_ * next; /* chained list */

} type_block;

typedef type_block * list_block;


/**
 *  Creation d'une list de blocks
 */
list_block * config_createlistblock();

/**
 *  Creation d'un block
 */
type_block * config_createblock(char * blockname,list_affect * list);

/**
 *  Ajout d'un block a une list de blocks
 */
void config_addblock( list_block * list,type_block * block );


/**
 *  Creation d'une list d'affectations
 */
list_affect * config_createlistaffect();

/**
 *  Creation d'une definition variable=valeur
 */
type_affect * config_createaffect(char * varname,char * varval);


/**
 *  Ajout d'une definition a une list d'affectations
 */
void config_adddef(list_affect * list,type_affect * affect);


/**
*   Affichage idente du contenu d'une list de blocks.
*/
void config_print_list(FILE * output, list_block * list);


/**
 * config_free_list:
 * libere les ressources utilisees par une liste de blocks.
 */
void config_free_list(list_block * list);




#endif
