/*
**	log.c
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

/*
**	configure this module
*/
int l2f_LOG_configure(const char *section,const int idx,const char *key,const char *value)
{
	if (!strcmp(section,"log:file"))
		return l2f_LOG_FILE_configure(idx,key,value);
	if (!strcmp(section,"log:mysql"))
		return l2f_LOG_MYSQL_configure(idx,key,value);
	if (!strcmp(section,"log:syslog"))
		return l2f_LOG_SYSLOG_configure(idx,key,value);
	return 0;
}

/*
**	dump the configuration
*/
void l2f_LOG_configdump(void)
{
	l2f_LOG_FILE_configdump();
	l2f_LOG_MYSQL_configdump();
	l2f_LOG_SYSLOG_configdump();
}

/*
**	process this module
*/
int l2f_LOG_process(L2F_EVENT_T *l2f_event)
{
	logmsg(LOG_DEBUG,"logging event\n");

	l2f_LOG_FILE_process(l2f_event);
	l2f_LOG_MYSQL_process(l2f_event);
	l2f_LOG_SYSLOG_process(l2f_event);

	logmsg(LOG_DEBUG,"complete\n");

	return 0;
}/**/
