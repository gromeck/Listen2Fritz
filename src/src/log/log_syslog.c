/*
**	log_syslog.c
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
#include "log.h"
#include "log_syslog.h"
#include <string.h>

static CFG_LOG_SYSLOG_T *_cfg_log_syslog;

/*
**	configure this module
*/
int l2f_LOG_SYSLOG_configure(const int idx,const char *key,const char *value)
{
#if defined(HAVE_L2F_LOGGING_SYSLOG)
	CFG_LOG_SYSLOG_T *cfg;

	CFG_FIND_OR_CREATE(log_syslog);

	CFG_SET_KEY_VALUE_STRING(log_syslog,ident)
	else CFG_SET_KEY_VALUE_STRING(log_syslog,format)
	else return 0;
#else
	logmsg(LOG_CRIT,"no file logging support enabled -- skipped\n");
#endif
	return 1;
}

/*
**	dump the configuration
*/
void l2f_LOG_SYSLOG_configdump(void)
{
	CFG_LOG_SYSLOG_T *cfg;

	CFG_LOOP(log_syslog) {
		CFG_DUMP_KEY_VALUE_STRING(log_syslog,ident)
		CFG_DUMP_KEY_VALUE_STRING(log_syslog,format)
	}
}

/*
**	process this module
*/
int l2f_LOG_SYSLOG_process(const L2F_EVENT_T *l2f_event)
{
#if defined(HAVE_L2F_LOGGING_SYSLOG)
	CFG_LOG_SYSLOG_T *cfg;

	if (!CFG_1ST_RECORD(log_syslog)) {
		logmsg(LOG_DEBUG,"logging to file not configured -- skipping\n");
		return 0;
	}
	logmsg(LOG_DEBUG,"logging to file ...\n");

	/*
	**	loop over the configuration records
	*/
	CFG_LOOP(log_syslog) {
		/*
		**	check if this module is configured
		*/
		if (!cfg->ident) {
			logmsg(LOG_CRIT,"[%d] not configured -- skipped\n",cfg->idx);
			continue;
		}

		/*
		**	open the logfile
		*/
		logmsg(LOG_DEBUG,"[%d] opening syslog %s\n",cfg->idx,cfg->ident);
		openlog(cfg->ident,LOG_CONS|LOG_NDELAY|LOG_PID,LOG_USER);

		/*
		**	write the record
		*/
		logmsg(LOG_DEBUG,"[%d] logging event\n",cfg->idx);
		syslog(LOG_USER|LOG_INFO,"%s",l2f_EVENT_format(cfg->format,l2f_event));

		/*
		**	close the logfile
		*/
		logmsg(LOG_DEBUG,"[%d] closing syslog\n",cfg->idx);
		closelog();
	}
	logmsg(LOG_DEBUG,"complete\n");
#endif
	return 0;
}/**/
