#!/bin/sh
#*****************************************************************************\
#  scripts/batch_system/profiles/slurm.sh - 
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


export BRIDGE_MSUB_JOBID=${SLURM_JOB_ID}
export BRIDGE_MSUB_STEPID=${SLURM_STEP_ID}

export BRIDGE_MSUB_NNODE=${SLURM_NNODES:-1}
export BRIDGE_MSUB_NPROC=${SLURM_NPROCS:-1}

export BRIDGE_MSUB_QUEUE=$(squeue -o %P -h -j ${SLURM_JOB_ID})
export BRIDGE_MSUB_PROJECT=$(squeue -o %a -h -j ${SLURM_JOB_ID})

# not currently defined by Slurm in batch mode
# values will be inherited from msub command
#export BRIDGE_MSUB_NCORE=
