/**
 *
 *
 * Copyright CEA/DAM/DIF  (2005)
 * contributeur : Philippe DENIEL   philippe.deniel@cea.fr
 *                Thomas LEIBOVICI  thomas.leibovici@cea.fr
 * Ce logiciel est un programme informatique servant � surveiller des
 * journaux et des tests de non-r�gression pour y d�tecter des situations
 * probl�matiques et des dysfonctionements.
 *
 * Ce logiciel est r�gi par la licence CeCILL soumise au droit fran�ais et
 * respectant les principes de diffusion des logiciels libres. Vous pouvez
 * utiliser, modifier et/ou redistribuer ce programme sous les conditions
 * de la licence CeCILL telle que diffus�e par le CEA, le CNRS et l'INRIA
 * sur le site "http://www.cecill.info".
 *
 * En contrepartie de l'accessibilit� au code source et des droits de copie,
 * de modification et de redistribution accord�s par cette licence, il n'est
 * offert aux utilisateurs qu'une garantie limit�e.  Pour les m�mes raisons,
 * seule une responsabilit� restreinte p�se sur l'auteur du programme,  le
 * titulaire des droits patrimoniaux et les conc�dants successifs.
 *
 * A cet �gard  l'attention de l'utilisateur est attir�e sur les risques
 * associ�s au chargement,  � l'utilisation,  � la modification et/ou au
 * d�veloppement et � la reproduction du logiciel par l'utilisateur �tant
 * donn� sa sp�cificit� de logiciel libre, qui peut le rendre complexe �
 * manipuler et qui le r�serve donc � des d�veloppeurs et des professionnels
 * avertis poss�dant  des  connaissances  informatiques approfondies.  Les
 * utilisateurs sont donc invit�s � charger  et  tester  l'ad�quation  du
 * logiciel � leurs besoins dans des conditions permettant d'assurer la
 * s�curit� de leurs syst�mes et ou de leurs donn�es et, plus g�n�ralement,
 * � l'utiliser et l'exploiter dans les m�mes conditions de s�curit�.
 *
 * Le fait que vous puissiez acc�der � cet en-t�te signifie que vous avez
 * pris connaissance de la licence CeCILL, et que vous en avez accept� les
 * termes.
 *
 * ---------------------
 *
 * Copyright CEA/DAM/DIF (2005)
 *  Contributor: Philippe DENIEL  philippe.deniel@cea.fr
 *               Thomas LEIBOVICI thomas.leibovici@cea.fr
 *
 *
 * This software is a computer program whose purpose is to manage logs and
 * non-regression tests in order to detect problems and system failures.
 *
 * This software is governed by the CeCILL  license under French law and
 * abiding by the rules of distribution of free software.  You can  use,
 * modify and/ or redistribute the software under the terms of the CeCILL
 * license as circulated by CEA, CNRS and INRIA at the following URL
 * "http://www.cecill.info".
 *
 * As a counterpart to the access to the source code and  rights to copy,
 * modify and redistribute granted by the license, users are provided only
 * with a limited warranty  and the software's author,  the holder of the
 * economic rights,  and the successive licensors  have only  limited
 * liability.
 *
 * In this respect, the user's attention is drawn to the risks associated
 * with loading,  using,  modifying and/or developing or reproducing the
 * software by the user in light of its specific status of free software,
 * that may mean  that it is complicated to manipulate,  and  that  also
 therefore means  that it is reserved for developers  and  experienced
 * professionals having in-depth computer knowledge. Users are therefore
 * encouraged to load and test the software's suitability as regards their
 * requirements in conditions enabling the security of their systems and/or
 * data to be ensured and,  more generally, to use and operate it in the
 * same conditions as regards security.
 *
 * The fact that you are presently reading this means that you have had
 * knowledge of the CeCILL license and that you accept its terms.
 * ---------------------------------------
 *
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
 * config_GetErrorMsg returns a detailled message
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
