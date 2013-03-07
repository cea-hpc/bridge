#include <unistd.h>
#include <limits.h>
#include <stdlib.h>
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <wordexp.h>

#define BRIDGE_RANKID  "BRIDGE_MPRUN_RANKID"
#define OPENMPI_RANKID "OMPI_COMM_WORLD_RANK"
#define SLURM_RANKID   "SLURM_PROCID"

#define ERROR_TASKID_EV_NOT_SET  2
#define ERROR_TASKID_EV_INVALID  3
#define ERROR_DESC_FILE_ACCESS  11
#define ERROR_DESC_FILE_INV_TSK 12
#define ERROR_DESC_FILE_INV_CMD 13

int bridge_mpmd_wrapper(int argc, char** argv)
{

	int rc = 1;

	char *progname;
	char *optstring = "hf:E:";
	char option;
	char* short_options_desc="\nUsage : %s [-h] [-E env_var] -f mpmd_desc\n\n";
	char* addon_options_desc="\
\tThis wrapper executes the command specified in the MPMD description file and that\n\
\tis associated to the current process based on its global ID in the job\n\n\
\t-h\t\tshow this message\n\
\t-E env_var\tEnv variable to use to get the global taskid of each process\n\
\t\t\tDefault is the first defined between " BRIDGE_RANKID ", \n\t\t\t" OPENMPI_RANKID " and " SLURM_RANKID "\n\
\t-f mpmd_desc\tFile containing the description of the various tasks to launch\n\
\t\t\tEach line must be a comment starting with '#' or respect the \n\t\t\tformat \"taskcount cmdline...\" \n\n";

	char *taskid_ev = NULL;
	char *taskid_str = NULL;

	long int taskid;
	long int taskcnt = 0;

	char *count_str;
	long int count = 0;

	FILE   *f;
	char   *path = NULL;
	char   *line = NULL;
	char   *cmd;
	char   *p;
	size_t s,l;
	int    ln;

	wordexp_t we;

	/* get current program name */
	progname=rindex(argv[0],'/');
	if(progname==NULL)
		progname=argv[0];
	else
		progname++;

	/* process options */
	while((option = getopt(argc,argv,optstring)) != -1)
	{
		switch(option)
		{
		case 'f' :
			path = strdup(optarg);
			break;
		case 'E' :
			taskid_ev = strdup(optarg);
			break;
		case 'h' :
		default :
			fprintf(stdout,short_options_desc,progname);
		fprintf(stdout,"%s\n",addon_options_desc);
		exit(0);
		break;
		}
	}

	if (path == NULL) {
		fprintf(stdout,short_options_desc,progname);
		exit(1);
	}

	/* 
	 * Get the current process id in the global application
	 *
	 * If no env var is specified on the command line, then :
	 *  - try to read the BRIDGE associated env var
	 *  - if not available, try to read the OpenMPI associated env var
	 *  - if not available, try to read the SLURM associated env var
	 */
	if (taskid_ev != NULL)
		taskid_str = getenv(taskid_ev);
	else {
		if (taskid_str == NULL)
			taskid_str = getenv(BRIDGE_RANKID);
		if (taskid_str == NULL)
			taskid_str = getenv(OPENMPI_RANKID);
		if (taskid_str == NULL) {
			taskid_str = getenv(SLURM_RANKID);
			taskid_ev = strdup(BRIDGE_RANKID " and " OPENMPI_RANKID
					   " and " SLURM_RANKID);
			if (taskid_ev == NULL)
				exit(255);
		}
	}
	if(taskid_str == NULL) {
		fprintf(stderr,"unable to read taskid env var '%s' : "
			"%s\n",taskid_ev,strerror(errno));
		rc = ERROR_TASKID_EV_NOT_SET;
		goto exit;
	}
	errno = 0;
	taskid = strtol(taskid_str,NULL,10);
	if(errno == ERANGE || errno == EINVAL) {
		fprintf(stderr,"unable to get current taskid using desc '%s' : "
			"%s\n",taskid_str,strerror(errno));
		rc = ERROR_TASKID_EV_INVALID;
		goto exit;
	}

	/* Open the description file and read the instructions */
	f = fopen(path,"r");
	if (!f) {
		fprintf(stderr,"task[%ld]: unable to open file '%s' : %s\n",
			taskid,path,strerror(errno));
		rc = ERROR_DESC_FILE_ACCESS;
		goto exit;
	}
	while ( getline(&line,&l,f) > 0 ) {

		ln++;

		/* skip comments and blank lines */
		if (line[0] == '#' || line[0] == '\n')
			goto next_line;

		/* search for the first \n and replace it with \0 */
		if ( (p = index(line,'\n')) != NULL )
			*p = '\0';

		/* check line format */
		count_str = line;
		s = strspn(count_str,"0123456789");
		if (s == 0 || count_str[s] != ' ') {
			fprintf(stderr,"task[%ld]: line '%d' has not a valid "
				"format (\"count cmdline\") : skipping invalid"
				" line '%s'\n",taskid,ln,line);
			goto next_line;
		}

		/* check tasks count */
		count_str[s] = '\0';
		errno = 0;
		count = strtol(count_str,NULL,10);
		if( (count == LONG_MIN || count == LONG_MAX) && 
		    (errno == ERANGE   || errno == EINVAL) ) {
			fprintf(stderr,"task[%ld]: line '%d' has an invalid "
				"tasks count : skipping invalid line '%s' : "
				"%s\n",taskid,ln,line,strerror(errno));
			rc = ERROR_DESC_FILE_INV_TSK;
			goto exit_file;
		}
		count_str[s] = ' ';
		
		/* if current process matches, execvp the instruction */
		if ( taskid < taskcnt + count ) {

			/* check command line */
			cmd = line + s + 1;
			errno = wordexp(cmd,&we,0);
			if (errno == 0) {
				rc = execvp(we.we_wordv[0],we.we_wordv);
				if (rc == -1) {
					fprintf(stderr,"task[%ld]: line '%d' has"
						" an invalid cmdline '%s' : "
						"unable to execute it : %s\n",
						taskid,ln,cmd,strerror(errno));
					rc = ERROR_DESC_FILE_INV_CMD;
					goto exit_file;
				}
			} else {
				fprintf(stderr,"task[%ld]: line '%d' has an "
					"invalid cmdline '%s' :  wordexp error"
					" = %u\n",taskid,ln,cmd,errno);
				rc = ERROR_DESC_FILE_INV_CMD;
				goto exit_file;
			}
			
		}

		/* increments tasks count */
		taskcnt += count;
		
	next_line:
		free(line);
		line = NULL;
	}


	/* only called when not enough lines to find the cmdline to launch */
	fprintf(stderr,"task[%ld]: not enough lines to find a cmd to launch : "
		"only %ld tasks defined in file '%s'\n",taskid,taskcnt,path);

exit_file:
	fclose(f);

exit:
	free(taskid_ev);
	exit(rc);

}
