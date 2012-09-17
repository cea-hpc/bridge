/*****************************************************************************\
 *  src/bridged/worker.h - 
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

#ifndef __BRIDGED_WORKER_H_
#define __BRIDGED_WORKER_H_

#include "bridged_engine.h"

#include "xternal/xqueue.h"
#include "bridged/bridge_rus.h"

typedef struct bridged_worker_args {

  pthread_t thread;

  int id;
  bridged_engine_t* engine;

  xqueue_t* socket_queue;
  
  bridge_rus_mgr_t* rus_mgr;

} bridged_worker_args_t;

int worker_process_request(void* p_args,int socket);

#endif
