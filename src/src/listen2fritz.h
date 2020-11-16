/*
**	listen2fritz.h
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
#ifndef __LISTEN2FRITZ_H__
#define __LISTEN2FRITZ_H__ 1

#include "../config.h"
#include "configuration.h"
#include <stdio.h>
#include <time.h>

#define __TITLE__				PACKAGE

#define DEFAULT_CONFIG_FILE		"/etc/listen2fritz.conf"

/*
**	structure which describes an event
*/
typedef struct _listen2fritz_event {
	time_t time;				// time of the event
	int type;					// type of the event
	int incoming;				// direction of call (!=0: incoming; ==0: outgoing)
	int pretype;				// in case of a DISCONNECT, this holds the previous type (CALL or RING)
	int line;					// line number
	int duration;				// duration in seconds
	char caller_number[20];		// caller number
	char caller_name[BUFSIZ];	// caller name
	char called_number[20];		// called number
	char called_name[BUFSIZ];	// called name
} L2F_EVENT_T;

/*
**	type for a callback
*/
typedef int (L2F_CALLBACK_T)(L2F_EVENT_T *event);

/*
**	different types of calls
*/
#define L2F_EVENT_TYPE_UNKNOWN			0
#define L2F_EVENT_TYPE_CALL				1
#define L2F_EVENT_TYPE_RING				2
#define L2F_EVENT_TYPE_CONNECT			3
#define L2F_EVENT_TYPE_DISCONNECT		4

/*
**	prototypes
*/
char *l2f_EVENT_type2string(const int type);
void l2f_EVENT_dump(const char *title,const L2F_EVENT_T *l2f_event);
char *l2f_EVENT_format(const char *format,const L2F_EVENT_T *l2f_event);

#endif

/**/
