#!/bin/sh
#*****************************************************************************\
#  scripts/batch_system/profiles/lsf.sh - 
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


export BRIDGE_MSUB_REQID=${LSB_JOBID}

# not currently defined by LSF
# values will be inherited from msub command
#export BRIDGE_MSUB_NCORE=
#export BRIDGE_MSUB_NNODE=
#export BRIDGE_MSUB_NPROC=

export BRIDGE_MSUB_QUEUE=${LSB_QUEUE}
export BRIDGE_MSUB_PROJECT=${LSB_UNIXGROUP}
