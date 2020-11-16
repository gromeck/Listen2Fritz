/*
**	lookup_script.c
**
**	Copyright (c) 2007 by Christian Lorenz
**
**	====================================================================
**
**	This file is part of listen2fritz.
**	
**	listen2fritz is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**	
**	listen2fritz is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**	
**	You should have received a copy of the GNU General Public License
**	along with listen2fritz.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "lookup.h"
#include "lookup_script.h"
#include "../phonenumber/phonenumber.h"

static CFG_LOOKUP_SCRIPT_T *_cfg_lookup_script = NULL;

/*
**	configure this module
*/
int l2f_LOOKUP_SCRIPT_configure(const int idx,const char *key,const char *value)
{
	CFG_LOOKUP_SCRIPT_T *cfg;

	CFG_FIND_OR_CREATE(lookup_script);

	CFG_SET_KEY_VALUE_STRING(lookup_script,script)
	else return 0;
	return 1;
}

/*
**	dump the configuration
*/
void l2f_LOOKUP_SCRIPT_configdump(void)
{
	CFG_LOOKUP_SCRIPT_T *cfg;

	CFG_LOOP(lookup_script) {
		CFG_DUMP_KEY_VALUE_STRING(lookup_script,script);
	}
}

/*
**	process this module
*/
const char *l2f_LOOKUP_SCRIPT_process(const char *number)
{
#if defined(HAVE_L2F_LOOKUP_SCRIPT)
	CFG_LOOKUP_SCRIPT_T *cfg;
	pid_t pid;
	int pfildes[2];
	char *normnumber = l2f_PHONENUMBER_normalize(number);
	static char *name = NULL;

	if (!CFG_1ST_RECORD(lookup_script)) {
		logmsg(LOG_DEBUG,"lookup via script not configured -- skipping\n");
		return NULL;
	}
	logmsg(LOG_DEBUG,"looking up via script ...\n");

	/*
	**	allocate memory for the buffer
	*/
	if (!name) {
		if (!(name = malloc(BUFSIZ))) {
			logmsg(LOG_CRIT,"malloc failed: %d %s\n",errno,strerror(errno));
			return NULL;
		}
	}
	*name = '\0';

	/*
	**	loop over the configuration records
	*/
	CFG_LOOP(lookup_script) {
		/*
		**	check if this module is configured
		*/
		if (!cfg->script) {
			logmsg(LOG_CRIT,"not configured -- skipped\n");
			continue;
		}

		logmsg(LOG_DEBUG,"looking up: %s\n",normnumber);

		/*
		**	Make pipes
		*/
		if (pipe(pfildes) < 0) {
			logmsg(LOG_CRIT,"couldn't create pipes: %d (%s)\n",errno,strerror(errno));
			continue;
		}

		/*
		**	for myself
		*/
		logmsg(LOG_DEBUG,"calling fork()\n");
		if ((pid = fork()) < 0) {
			logmsg(LOG_CRIT,"couldn't fork: %d (%s)\n",errno,strerror(errno));
			continue;
		}
		if (pid) {
			/*
			**	we are the father process
			*/
			FILE *fd;
			char *input;
			char buffer[BUFSIZ];

			logmsg(LOG_DEBUG,"waiting for input from spawned lookup script\n");
			close(pfildes[1] );
			fd = fdopen(pfildes[0],"r");
			while ((input = fgets(buffer,sizeof(buffer) - 1,fd))) {
				input = util_strdark(input);
				logmsg(LOG_DEBUG,"from child: %s\n",input);
				if (strlen(name) + strlen(input) < BUFSIZ)
					strcat(name,input);
				else
					break;
			}
			fclose(fd);
		}
		else {
			/*
			**	child process
			*/
			logmsg(LOG_DEBUG,"spawning lookup script: %s\n",cfg->script);
			close(pfildes[0]);
			dup2(pfildes[1],1);
			close(pfildes[1]);
			if (execl(cfg->script,cfg->script,normnumber,NULL) < 0) {
				logmsg(LOG_CRIT,"couldn't execl: %d (%s)\n",errno,strerror(errno));
			}
			logmsg(LOG_DEBUG,"spawning lookup script failed -- terminating\n");
			exit(1);
		}

		logmsg(LOG_DEBUG,"looked up: %s=%s\n",normnumber,(name[0]) ? name : "NULL");
		if (name[0])
			break;
	}
	logmsg(LOG_DEBUG,"complete\n");
	return (name[0]) ? name : NULL;
#else
	return NULL;
#endif
}/**/
