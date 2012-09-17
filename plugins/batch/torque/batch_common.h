/*****************************************************************************\
 *  plugins/batch/torque/batch_common.h - 
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


#ifndef __BATCH_COMMON_H
#define __BATCH_COMMON_H

int extract_field(char* statusline,char* header,char** field);

int extract_mem(char* statusline,char* header,uint32_t* i);

int extract_float(char* statusline,char* header,float* load);

int extract_uint32(char* statusline,char* header,uint32_t* i);

int extract_cores(char* line,uint32_t* i);

int convert_mem(char* field,uint32_t* mem);

int convert_time(char* field,time_t* time);

#endif
