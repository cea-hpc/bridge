/*****************************************************************************\
 *  lib/xternal/xlogger.h - 
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

/**
 * \file xlogger.h
 * \author M. Hautreux
 * \date 07/01/2008
 * \brief External logger headers
 */
#ifndef __XLOGGER_H_
#define __XLOGGER_H_

#include <stdarg.h>

/*! \addtogroup XTERNAL
 *  @{
 */

/*! \addtogroup XLOGGER
 *  @{
 */

#define XVERBOSE_LEVEL_1   1
#define XVERBOSE_LEVEL_2   2
#define XVERBOSE_LEVEL_3   3

#define XDEBUG_LEVEL_1    1
#define XDEBUG_LEVEL_2    2
#define XDEBUG_LEVEL_3    3

/*!
 * \fn xerror_setmaxlevel(int level)
 * \brief set error max level that can be displayed
 *
 * \param level max level to set
 *
*/
void
xerror_setmaxlevel(int level);

/*!
 * \fn xerror_setstream(FILE* stream)
 * \brief set stream to use when printing error messages
 *
 * \param stream stream to use (default is stderr)
*/
void
xerror_setstream(FILE* stream);

/*!
 * \fn xerror(char* format,...)
 * \brief print error message of level 1
 *
 * \param format format to print optionnaly followed by args
*/
void
xerror(char* format,...);


/*!
 * \fn xverbose_setmaxlevel(int level)
 * \brief set verbose max level that can be displayed
 *
 * \param level max level to set
*/
void
xverbose_setmaxlevel(int level);

/*!
 * \fn xverbose_setstream(FILE* stream)
 * \brief set stream to use when printing verbose messages
 *
 * \param stream stream to use (default is stdout)
*/
void
xverbose_setstream(FILE* stream);

/*!
 * \fn xverbose(char* format,...)
 * \brief print verbose message of level 1
 *
 * \param format format to print optionnaly followed by args
*/
void
xverbose(char* format,...);

/*!
 * \fn xverbose2(char* format,...)
 * \brief print verbose message of level 2
 *
 * \param format format to print optionnaly followed by args
*/
void
xverbose2(char* format,...);

/*!
 * \fn xverbose3(char* format,...)
 * \brief print verbose message of level 3
 *
 * \param format format to print optionnaly followed by args
*/
void
xverbose3(char* format,...);

/*!
 * \fn xverboseN(int level,char* format,...)
 * \brief print verbose message of given level
 *
 * \param level level of the message (1<=N<=9)
 * \param format format to print optionnaly followed by args
*/
void
xverboseN(int level,char* format,...);


/*!
 * \fn xdebug_setmaxlevel(int level)
 * \brief set debug max level that can be displayed
 *
 * \param level max level to set
*/
void
xdebug_setmaxlevel(int level);

/*!
 * \fn xdebug_setstream(FILE* stream)
 * \brief set stream to use when printing debug messages
 *
 * \param stream stream to use (default is stdout)
*/
void
xdebug_setstream(FILE* stream);

/*!
 * \fn xdebug(char* format,...)
 * \brief print debug message of level 1
 *
 * \param format format to print optionnaly followed by args
*/
void
xdebug(char* format,...);

/*!
 * \fn xdebug2(char* format,...)
 * \brief print debug message of level 2
 *
 * \param format format to print optionnaly followed by args
*/
void
xdebug2(char* format,...);

/*!
 * \fn xdebug3(char* format,...)
 * \brief print debug message of level 3
 *
 * \param format format to print optionnaly followed by args
*/
void
xdebug3(char* format,...);

/*!
 * \fn xdebugN(int level,char* format,...)
 * \brief print debug message of given level
 *
 * \param level level of the message (1<=N<=9)
 * \param format format to print optionnaly followed by args
*/
void
xdebugN(int level,char* format,...);

/*!
 * @}
*/

/*!
 * @}
*/

#endif /* !__XLOGGER_H_ */
