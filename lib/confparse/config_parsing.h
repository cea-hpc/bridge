/*****************************************************************************\
 *  lib/confparse/config_parsing.h -
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
 * \file    parsing.h
 * \author  $Author: deniel $
 * \date    $Date: 2005/11/28 17:03:22 $
 * \version	$Revision: 1.3 $ 
 * \brief   Configuration file parsing routines.
 *
 *  This API is not thread-safe.
 *
 * CVS History:
 *
 * $Log: config_parsing.h,v $
 * Revision 1.3  2005/11/28 17:03:22  deniel
 * Added CeCILL headers
 *
 * Revision 1.2  2005/04/22 07:42:20  leibovic
 * Adding config_Print.
 *
 * Revision 1.1  2005/04/18 10:46:30  leibovic
 * Interface for config parsing.
 *
 * Revision 1.1  2005/04/15 15:01:49  leibovic
 * Initial version for config file parsing.
 *
 *
 */


#ifndef _CONFIG_PARSING_H
#define _CONFIG_PARSING_H

#include <stdlib.h>
#include <stdio.h>

/* opaque type */
typedef caddr_t config_file_t;


/* config_ParseFile:
 * Reads the content of a configuration file and
 * stores it in a memory structure.
 * \return NULL on error.
 */   
config_file_t config_ParseFile(  char * file_path );


/* If config_ParseFile returns a NULL pointer,
 * config_GetErrorMsg returns a detailed message
 * to indicate the reason for this error.
 */
char * config_GetErrorMsg();


/**
 * config_Print:
 * Print the content of the syntax tree
 * to a file.
 */
void config_Print( FILE * output, config_file_t config );



/* Free the memory structure that store the configuration. */
void config_Free( config_file_t config );


/* Indicates how many blocks are defined into the config file.
 * \return A positive value if no error.
 *         Else return a negative error code.
 */
int config_GetNbBlocks( config_file_t config );


/* Indicates the name of a block (specified with its index). */
char * config_GetBlockName( config_file_t config , int block_no );


/* Indicates how many peers (key-value) are defined in a block
 * (specified with its index).
 */
int config_GetNbKeys( config_file_t config, int block_no );


/* Retrieves a key-value peer from the block index and the key index. */  
int config_GetKeyValue( 
                        config_file_t config,
                        int block_no,
                        int key_no,
                        char ** var_name,
                        char ** var_value
                       );


/* Returns the block with the specified name. */
int config_GetBlockIndexByName( config_file_t config,
                                char * block_name );


/* Returns the value of the key with the specified name. */
char * config_GetKeyValueByName( config_file_t config,
                                 int block_no,
                                 char * key_name );





        
#endif
