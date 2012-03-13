#!/bin/bash

if [[ ${SPANK_STATS_DIGEST} == "yes" ]] || [[ ${SPANK_STATS_DIGEST} == "1" ]]
then
    /usr/libexec/spank-stats-script post ${SLURM_JOBID} ${SLURM_STEPID}
fi
