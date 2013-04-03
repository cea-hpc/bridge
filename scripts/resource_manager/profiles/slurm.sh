#!/bin/sh
#*****************************************************************************\
#  scripts/resource_manager/profiles/slurm.sh - 
#******************************************************************************
#  Copyright  CEA/DAM/DIF (2012)
#
#  This file is part of Bridge, an abstraction layer to ease batch system and
#  resource manager usage in heterogeneous HPC environments.
#
#  Bridge is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  Bridge is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with Bridge.  If not, see <http://www.gnu.org/licenses/>
#*****************************************************************************/


export BRIDGE_MPRUN_REQID=${SLURM_JOB_ID}

export BRIDGE_MPRUN_NNODE=${SLURM_NNODES}
export BRIDGE_MPRUN_NPROC=${SLURM_NPROCS}
export BRIDGE_MPRUN_NCORE=${SLURM_CPUS_PER_TASK}

# Do not use squeue here to get information as it could
# severely limit the scalability of large jobs using
# this profile and/or overload the slurm controller
# when used by too much jobs simultaneously
# (MPMD script use it a lot)
#
# Use inherited MSUB variables if available instead
#
#export BRIDGE_MPRUN_QUEUE=$(squeue -o %R -h -j ${SLURM_JOB_ID})
#export BRIDGE_MPRUN_PROJECT=$(squeue -o %a -h -j ${SLURM_JOB_ID})
export BRIDGE_MPRUN_QUEUE=${BRIDGE_MSUB_QUEUE}
export BRIDGE_MPRUN_PROJECT=${BRIDGE_MSUB_PROJECT}

export BRIDGE_MPRUN_NODEID=${SLURM_NODEID}

export BRIDGE_MPRUN_PROCID=${SLURM_LOCALID}
if [[ -n ${OMPI_COMM_WORLD_LOCAL_RANK} ]]
then
    export BRIDGE_MPRUN_PROCID=${OMPI_COMM_WORLD_LOCAL_RANK}
fi

export BRIDGE_MPRUN_RANKID=${SLURM_PROCID}
if [[ -n ${OMPI_COMM_WORLD_RANK} ]]
then
    export BRIDGE_MPRUN_RANKID=${OMPI_COMM_WORLD_RANK}
fi
