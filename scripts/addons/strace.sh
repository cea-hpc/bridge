#! /bin/bash
# exec 2>/tmp/trx_cea_strace_$$ ; set -x
: numargs=$# : $@

function usage
{
	cat <<+++
usage: $ARGV0 [-o <dir>] [-O <opts>] [-R <mpi_rank>] -- command args
       -o <dir>         to write trace outputs under directory <dir> (default: home -t)
       -O <opt>		to transmit <opt> option to the tracer
       -R <mpi_rank>	to trace only some MPI processes
       -t <tracer>  	to use <tracer> command instead of strace

	Mpi ranks may be specified 
	- as a list of numerical ranges
	- or using 'nodeset' command synatx with 'mpi' as nodename

	Ex: -R 1-4,8-9
	    -R mpi[1-4,8-9]
            -R 'mpi[1-9]!mpi[5-7]'
		trace only MPI rank: 1, 2, 3, 4, 8, 9.

	traces are generated using the following name convention:
	\$(home -t)/strace_{JOBID}.{STEPID}/{HOSTNAME}/trx_proc_{MPIRANK}

+++
	exit 1
}

# canonicalize a mpi range
# return 1 if any error
canonic()
{
	typeset range
	if [[ $1 = mpi\[*\] ]] ; then
		range="$1"
	else
		range="mpi[$1]"
	fi
	# nodeset will generate and check mpi range
	nodeset -f $range 2>/dev/null
}

# check in current mpi process is in the MPI_RANGE
# return 1 if not
i_must_be_traced()
{
	(( $(nodeset -c $MPI_RANGE\&mpi$SLURM_PROCID) == 1 ))
}

# count the number of traced MPI process
#
nb_proc_traced()
{
	nodeset -c $MPI_RANGE\&$MPI_ALLPROCS
}

# 
# main
#
# try to guess which command must be used to get scratch directory
if type cea_home >/dev/null 2>&1 ; then
    BDIR=$(cea_home -t)
elif type ccc_home >/dev/null 2>&1 ; then
    BDIR=$(ccc_home -t)
else 
    BDIR=
fi
typeset -i  errflag=0
typeset ARGV0=${0##*/}			# basename de la commande
typeset OPTSTRING=":o:O:R:t:"		# liste des options
typeset STRACE_OPTIONS=""		# addtional strace options
typeset MPI_ALLPROCS="mpi[0-$((SLURM_NTASKS - 1))]"	# all mpi process
typeset MPI_RANGE="$MPI_ALLPROCS"	# by default, trace all process
typeset tracer="strace"
#
while getopts $OPTSTRING option
do
	case "$option" in
	o) BDIR="$OPTARG" ;;
	O) STRACE_OPTIONS="$STRACE_OPTIONS $OPTARG" ;;
	R) MPI_RANGE="$( canonic $OPTARG )" || {
		echo "$ARGV0: syntaxe invalide $OPTARG" >&2
		exit 1
	   }
	   ;;
	t) tracer=$OPTARG ;;
	?) errflag=1 ;;
	esac
done
shift $(( OPTIND - 1 ))
if (( (errflag != 0) || ($# == 0) ))  ; then
	usage
fi

[[ -z $SLURM_JOB_ID ]] && {
	echo "$ARGV0: ERROR: this script must be used with slurm & bridge" >&2
	exit 1
}
[[ -z $BDIR ]] && {
	echo "$ARGV0: ERROR: no directory specified to put traces" >&2
	exit 1
}
(( $(nb_proc_traced) == 0 )) && {
	echo "$ARGV0: WARNING, no process will be traced !!!" >&2
}
type $tracer >/dev/null 2>&1 || {
	echo "$ARGV0: ERROR, command $tracer not found !!!" >&2
	exit 1
}


STRACE_DIR=$BDIR/strace_${SLURM_JOB_ID}.${SLURM_STEPID}/$(hostname -s)
STRACE_FILE=$STRACE_DIR/trx_proc_${SLURM_PROCID}
if (( SLURM_PROCID == 0 )) ; then
	echo "$ARGV0: INFO: $tracer will be ran on MPI processes $MPI_RANGE, see trace files under $BDIR/strace_${SLURM_JOB_ID}.${SLURM_STEPID}"
fi
if i_must_be_traced ; then
	# introduce some asynchronism to create directory
	usleep $(( $SLURM_PROCID * 2 ))
	[[ -d $STRACE_DIR ]] || mkdir -p $STRACE_DIR 2>/dev/null
	exec ${tracer} ${STRACE_OPTIONS} $@ 2>${STRACE_FILE}
else
	# execute directly application
	exec $@
fi
