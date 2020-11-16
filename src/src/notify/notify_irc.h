/*
**	notify_irc.h
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
#ifndef __NOTIFY_IRC_H__
#define __NOTIFY_IRC_H__	1

#include "notify.h"

/*
**	configuration structure
*/
typedef struct _cfg_notify_irc_t {
	CFG_COMMON_PART(notify_irc);
	char *host;				// DNS or IP address of the IRC server
	int port;				// port to connect to
	char *nick;				// nickname which will be used as the sender
	char *realname;			// realname which will be used as the sender
	char *buddies;			// colon separated list of nick to notify
	char *text;				// text to send
	char *caller_number;	// caller number
	char *called_number;	// called number
} CFG_NOTIFY_IRC_T;

/*
**	configuration defaults
*/
#define L2F_NOTIFY_IRC_DEFAULT_HOST		"localhost"
#define L2F_NOTIFY_IRC_DEFAULT_PORT		6667

/*
**	prototypes
*/
int l2f_NOTIFY_IRC_configure(const int idx,const char *key,const char *value);
void l2f_NOTIFY_IRC_configdump(void);
int l2f_NOTIFY_IRC_process(const L2F_EVENT_T *l2f_event);

#endif

/**/
