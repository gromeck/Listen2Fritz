/*
**	notify_mail.h
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
#ifndef __NOTIFY_MAIL_H__
#define __NOTIFY_MAIL_H__	1

#include "notify.h"

/*
**	configuration structure
*/
typedef struct _cfg_notify_mail_t {
	CFG_COMMON_PART(notify_mail);
	char *fromaddr;			// mail address used as the sender
	char *toaddr;			// receivers mail addresses
	char *subject;			// subject of the mail
	char *text;				// text to send
	char *caller_number;	// caller number
	char *called_number;	// called number
} CFG_NOTIFY_MAIL_T;

/*
**	configuration defaults
*/

/*
**	prototypes
*/
int l2f_NOTIFY_MAIL_configure(const int idx,const char *key,const char *value);
void l2f_NOTIFY_MAIL_configdump(void);
int l2f_NOTIFY_MAIL_process(const L2F_EVENT_T *l2f_event);

#endif

/**/
