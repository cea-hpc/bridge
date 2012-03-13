#!/bin/sh

export BRIDGE_MSUB_REQID=${LSB_JOBID}

# not currently defined by LSF
# values will be inherited from msub command
#export BRIDGE_MSUB_NCORE=
#export BRIDGE_MSUB_NNODE=
#export BRIDGE_MSUB_NPROC=

export BRIDGE_MSUB_QUEUE=${LSB_QUEUE}
export BRIDGE_MSUB_PROJECT=${LSB_UNIXGROUP}
