/*
**	log_file.c
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
#include "log_file.h"
#include <string.h>

static CFG_LOG_FILE_T *_cfg_log_file;

/*
**	configure this module
*/
int l2f_LOG_FILE_configure(const int idx,const char *key,const char *value)
{
#if defined(HAVE_L2F_LOGGING_FILE)
	CFG_LOG_FILE_T *cfg;

	CFG_FIND_OR_CREATE(log_file);

	CFG_SET_KEY_VALUE_STRING(log_file,file)
	else CFG_SET_KEY_VALUE_STRING(log_file,format)
	else return 0;
#else
	logmsg(LOG_CRIT,"no file logging support enabled -- skipped\n");
#endif
	return 1;
}

/*
**	dump the configuration
*/
void l2f_LOG_FILE_configdump(void)
{
	CFG_LOG_FILE_T *cfg;

	CFG_LOOP(log_file) {
		CFG_DUMP_KEY_VALUE_STRING(log_file,file)
		CFG_DUMP_KEY_VALUE_STRING(log_file,format)
	}
}

/*
**	process this module
*/
int l2f_LOG_FILE_process(const L2F_EVENT_T *l2f_event)
{
#if defined(HAVE_L2F_LOGGING_FILE)
	CFG_LOG_FILE_T *cfg;
	FILE *log_fd = NULL;

	if (!CFG_1ST_RECORD(log_file)) {
		logmsg(LOG_DEBUG,"logging to file not configured -- skipping\n");
		return 0;
	}
	logmsg(LOG_DEBUG,"logging to file ...\n");

	/*
	**	loop over the configuration records
	*/
	CFG_LOOP(log_file) {
		/*
		**	check if this module is configured
		*/
		if (!cfg->file) {
			logmsg(LOG_CRIT,"[%d] not configured -- skipped\n",cfg->idx);
			continue;
		}

		/*
		**	open the logfile
		*/
		logmsg(LOG_DEBUG,"[%d] opening logfile %s\n",cfg->idx,cfg->file);
		if (!(log_fd = fopen(cfg->file,"a+"))) {
			logmsg(LOG_CRIT,"[%d] couldn't open log file '%s': %d (%s)\n",cfg->idx,cfg->file,errno,strerror(errno));
			continue;
		}

		/*
		**	write the record
		*/
		logmsg(LOG_DEBUG,"[%d] logging event\n",cfg->idx);
		if (fprintf(log_fd,"%s",l2f_EVENT_format(cfg->format,l2f_event)) < 0) {
			logmsg(LOG_CRIT,"[%d] writing to log file '%s failed: %d (%s)\n",cfg->idx,cfg->file,errno,strerror(errno));
		}

		/*
		**	close the logfile
		*/
		logmsg(LOG_DEBUG,"[%d] closing logfile\n",cfg->idx);
		if (fclose(log_fd) < 0) {
			logmsg(LOG_CRIT,"[%d] closing of log file '%s' failed: %d (%s)\n",cfg->idx,cfg->file,errno,strerror(errno));
			continue;
		}
	}
	logmsg(LOG_DEBUG,"complete\n");
#endif
	return 0;
}/**/
