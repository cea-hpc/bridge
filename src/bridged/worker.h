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
