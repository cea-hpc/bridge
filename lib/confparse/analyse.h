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
 * Developping config file reading.
 *
 * Revision 1.1  2005/04/15 15:01:49  leibovic
 * Initial version for config file parsing.
 *
 *
 */


#ifndef CONFPARSER_H
#define CONFPARSER_H

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
