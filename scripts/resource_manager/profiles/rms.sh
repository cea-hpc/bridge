#!/bin/sh
#*****************************************************************************\
#  scripts/resource_manager/profiles/rms.sh - 
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


export BRIDGE_MPRUN_JOBID=${RMS_RESOURCE}
export BRIDGE_MPRUN_STEPID=${RMS_JOBID}

export BRIDGE_MPRUN_NNODE=${RMS_NNODES}
export BRIDGE_MPRUN_NPROC=${RMS_NPROCS}
export BRIDGE_MPRUN_NCORE=${RMS_NCPUS}

export BRIDGE_MPRUN_QUEUE=${RMS_PARTITION}

export BRIDGE_MPRUN_NODEID=${RMS_NODEID}
export BRIDGE_MPRUN_PROCID=${RMS_LOCALID}
export BRIDGE_MPRUN_RANKID=${RMS_PROCID}

# not set in RMS
# values will be inherited from mprun command
#export BRIDGE_MPRUN_PROJECT=
