#!/bin/sh

export BRIDGE_MSUB_REQID=${JOBID}
export BRIDGE_MSUB_QUEUE=${PBS_QUEUE}

# not currently defined by PBS
# values will be inherited from msub command
#export BRIDGE_MSUB_NCORE=
#export BRIDGE_MSUB_NNODE=
#export BRIDGE_MSUB_NPROC=
#export BRIDGE_MSUB_PROJECT=


