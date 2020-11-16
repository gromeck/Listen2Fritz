/*
**	lookup.c
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

/*
**	configure this module
*/
int l2f_LOOKUP_configure(const char *section,const int idx,const char *key,const char *value)
{
	if (!strcmp(section,"lookup:ldap"))
		return l2f_LOOKUP_LDAP_configure(idx,key,value);
	if (!strcmp(section,"lookup:mysql"))
		return l2f_LOOKUP_MYSQL_configure(idx,key,value);
	if (!strcmp(section,"lookup:script"))
		return l2f_LOOKUP_SCRIPT_configure(idx,key,value);
	return 0;
}

/*
**	dump the configuration
*/
void l2f_LOOKUP_configdump(void)
{
	l2f_LOOKUP_LDAP_configdump();
	l2f_LOOKUP_MYSQL_configdump();
	l2f_LOOKUP_SCRIPT_configdump();
}

/*
**	lookup one up
*/
static void lookup(const char *number,char *name,int maxlen)
{
	const char *s = NULL;

	logmsg(LOG_DEBUG,"looking up: %s\n",number);

	if (!s)
		s = l2f_LOOKUP_LDAP_process(number);
	if (!s)
		s = l2f_LOOKUP_MYSQL_process(number);
	if (!s)
		s = l2f_LOOKUP_SCRIPT_process(number);

	if (s) {
		strncpy(name,s,maxlen - 1);
		name[maxlen - 1] = '\0';
	}

	/*
	**	update the mysql database with the date we lookup
	*/
	l2f_LOOKUP_MYSQL_update(number,s);
}

/*
**	process this module
*/
int l2f_LOOKUP_process(L2F_EVENT_T *l2f_event)
{
	logmsg(LOG_DEBUG,"looking up numbers ...\n");
	l2f_EVENT_dump("event before lookup",l2f_event);

	/*
	**	lookup the caller
	*/
	if (l2f_event->caller_number[0])
		lookup(l2f_event->caller_number,l2f_event->caller_name,sizeof(l2f_event->caller_name));

	/*
	**	lookup the called
	*/
	if (l2f_event->called_number[0])
		lookup(l2f_event->called_number,l2f_event->called_name,sizeof(l2f_event->called_name));

	l2f_EVENT_dump("event after lookup",l2f_event);
	logmsg(LOG_DEBUG,"complete\n");

	return 0;
}/**/
