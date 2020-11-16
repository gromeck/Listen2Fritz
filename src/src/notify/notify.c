/*
**	notify.c
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

/*
**	configure this module
*/
int l2f_NOTIFY_configure(const char *section,const int idx,const char *key,const char *value)
{
	if (!strcmp(section,"notify:dreambox"))
		return l2f_NOTIFY_DREAMBOX_configure(idx,key,value);
	if (!strcmp(section,"notify:irc"))
		return l2f_NOTIFY_IRC_configure(idx,key,value);
	if (!strcmp(section,"notify:mail"))
		return l2f_NOTIFY_MAIL_configure(idx,key,value);
	if (!strcmp(section,"notify:script"))
		return l2f_NOTIFY_SCRIPT_configure(idx,key,value);
	if (!strcmp(section,"notify:callback"))
		return l2f_NOTIFY_CALLBACK_configure(idx,key,value);
	return 0;
}

/*
**	dump the configuration
*/
void l2f_NOTIFY_configdump(void)
{
	l2f_NOTIFY_DREAMBOX_configdump();
	l2f_NOTIFY_IRC_configdump();
	l2f_NOTIFY_MAIL_configdump();
	l2f_NOTIFY_SCRIPT_configdump();
	l2f_NOTIFY_CALLBACK_configdump();
}

/*
**	process this module
*/
int l2f_NOTIFY_process(L2F_EVENT_T *l2f_event)
{
	logmsg(LOG_DEBUG,"notifying ...\n");
	l2f_EVENT_dump(__FUNC__,l2f_event);

	l2f_NOTIFY_DREAMBOX_process(l2f_event);
	l2f_NOTIFY_IRC_process(l2f_event);
	l2f_NOTIFY_MAIL_process(l2f_event);
	l2f_NOTIFY_SCRIPT_process(l2f_event);
	l2f_NOTIFY_CALLBACK_process(l2f_event);

	logmsg(LOG_DEBUG,"complete\n");

	return 0;
}/**/
