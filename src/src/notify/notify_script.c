/*
**	notify_script.c
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
#include "notify.h"
#include "notify_script.h"
#include "../phonenumber/phonenumber.h"

static CFG_NOTIFY_SCRIPT_T *_cfg_notify_script = NULL;

/*
**	configure this module
*/
int l2f_NOTIFY_SCRIPT_configure(const int idx,const char *key,const char *value)
{
#if defined(HAVE_L2F_NOTIFY_SCRIPT)
	CFG_NOTIFY_SCRIPT_T *cfg;

	CFG_FIND_OR_CREATE(notify_script);

	CFG_SET_KEY_VALUE_STRING(notify_script,script)
	else CFG_SET_KEY_VALUE_STRING(notify_script,parameters)
	else CFG_SET_KEY_VALUE_STRING(notify_script,call)
	else CFG_SET_KEY_VALUE_STRING(notify_script,caller_number)
	else CFG_SET_KEY_VALUE_STRING(notify_script,called_number)
	else return 0;
#else
	logmsg(LOG_CRIT,"no notification support via script enabled -- skipped\n");
#endif
	return 1;
}

/*
**	dump the configuration
*/
void l2f_NOTIFY_SCRIPT_configdump(void)
{
	CFG_NOTIFY_SCRIPT_T *cfg;

	CFG_LOOP(notify_script) {
		CFG_DUMP_KEY_VALUE_STRING(notify_script,script);
		CFG_DUMP_KEY_VALUE_STRING(notify_script,parameters);
		CFG_DUMP_KEY_VALUE_STRING(notify_script,call);
		CFG_DUMP_KEY_VALUE_STRING(notify_script,caller_number);
		CFG_DUMP_KEY_VALUE_STRING(notify_script,called_number);
	}
}

/*
**	process this module
*/
int l2f_NOTIFY_SCRIPT_process(const L2F_EVENT_T *l2f_event)
{
#if defined(HAVE_L2F_NOTIFY_SCRIPT)
	CFG_NOTIFY_SCRIPT_T *cfg;
	pid_t pid;
	char cmdline[BUFSIZ];

	if (!CFG_1ST_RECORD(notify_script)) {
		logmsg(LOG_DEBUG,"notification via script not configured -- skipping\n");
		return 0;
	}
	logmsg(LOG_DEBUG,"notifying via script ...\n");

	/*
	**	loop over the configuration records
	*/
	CFG_LOOP(notify_script) {
		/*
		**	check if this module is configured
		*/
		if (!cfg->script || !cfg->parameters) {
			logmsg(LOG_CRIT,"[%d] not configured -- skipped\n",cfg->idx);
			continue;
		}

		/*
		**	check if we have the right event type
		*/
		if (!(cfg->call &&
				((!strcasecmp(cfg->call,"ring") && l2f_event->type == L2F_EVENT_TYPE_RING) ||
				(!strcasecmp(cfg->call,"missed") && l2f_event->type == L2F_EVENT_TYPE_DISCONNECT && !l2f_event->duration && l2f_event->pretype == L2F_EVENT_TYPE_RING)))) {
			logmsg(LOG_CRIT,"[%d] event type doesn't match config -- skipped\n",cfg->idx);
			continue;
		}

		/*
		**	check if the caller number matches
		*/
		if (cfg->caller_number && !l2f_PHONENUMBER_match(l2f_event->caller_number,cfg->caller_number)) {
			logmsg(LOG_CRIT,"[%d] configured caller_number (%s) doesn't match the caller number (%s) -- skipped\n",
					cfg->idx,cfg->caller_number,l2f_event->caller_number);
			continue;
		}

		/*
		**	check if the called number matches
		*/
		if (cfg->called_number && !l2f_PHONENUMBER_match(l2f_event->called_number,cfg->called_number)) {
			logmsg(LOG_CRIT,"[%d] configured called_number (%s) doesn't match the called number (%s) -- skipped\n",
					cfg->idx,cfg->called_number,l2f_event->called_number);
			continue;
		}

		/*
		**	for myself
		*/
		logmsg(LOG_CRIT,"[%d] calling fork()\n",cfg->idx);
		if ((pid = fork()) < 0) {
			logmsg(LOG_CRIT,"couldn't fork: %d (%s)\n",errno,strerror(errno));
			continue;
		}
		logmsg(LOG_CRIT,"[%d] fork returned %lu\n",cfg->idx,(unsigned long) pid);
		if (!pid) {
			/*
			**	child process
			*/
			sprintf(cmdline,"%s %s",cfg->script,l2f_EVENT_format(cfg->parameters,l2f_event));
			logmsg(LOG_DEBUG,"[%d] spawning notification script: %s\n",cfg->idx,cmdline);
			if (system(cmdline) < 0)
				logmsg(LOG_DEBUG,"[%d] spawning lookup script failed\n",cfg->idx);
			logmsg(LOG_DEBUG,"[%d] terminating!\n",cfg->idx);
			exit(1);
		}
		logmsg(LOG_CRIT,"[%d] looping\n",cfg->idx);
	}
	logmsg(LOG_DEBUG,"complete\n");
#endif
	return 1;
}/**/
