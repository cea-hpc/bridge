#!/bin/sh

export BRIDGE_MSUB_JOBID=${SLURM_JOB_ID}
export BRIDGE_MSUB_STEPID=${SLURM_STEP_ID}

export BRIDGE_MSUB_NNODE=${SLURM_NNODES:-1}
export BRIDGE_MSUB_NPROC=${SLURM_NPROCS:-1}

export BRIDGE_MSUB_QUEUE=$(squeue -o %R -h -j ${SLURM_JOB_ID})
export BRIDGE_MSUB_PROJECT=$(squeue -o %a -h -j ${SLURM_JOB_ID})

# not currently defined by Slurm in batch mode
# values will be inherited from msub command
#export BRIDGE_MSUB_NCORE=
