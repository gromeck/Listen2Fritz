/*
**	event.c
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
#include "event.h"
#include "../phonenumber/phonenumber.h"

/*
**	convert a n L2F_EVENT_TYPE_* into a string
*/
char *l2f_EVENT_type2string(const int type)
{
#define L2F_EVENT_TYPE2STR(x)  case x: return # x;
	switch (type) {
		L2F_EVENT_TYPE2STR(L2F_EVENT_TYPE_UNKNOWN)
		L2F_EVENT_TYPE2STR(L2F_EVENT_TYPE_CALL)
		L2F_EVENT_TYPE2STR(L2F_EVENT_TYPE_RING)
		L2F_EVENT_TYPE2STR(L2F_EVENT_TYPE_CONNECT)
		L2F_EVENT_TYPE2STR(L2F_EVENT_TYPE_DISCONNECT)
	}
#undef L2F_EVENT_TYPE2STR
	return "unknown";
}

/*
**	dump the given event
*/
void l2f_EVENT_dump(const char *title,const L2F_EVENT_T *l2fevent)
{
	logmsg(LOG_DEBUG,"%s: type:%s(%d)\n",title,l2f_EVENT_type2string(l2fevent->type),l2fevent->type);
	logmsg(LOG_DEBUG,"%s: incoming:%d\n",title,l2fevent->incoming);
	logmsg(LOG_DEBUG,"%s: pretype:%s(%d)\n",title,l2f_EVENT_type2string(l2fevent->pretype),l2fevent->pretype);
	logmsg(LOG_DEBUG,"%s: line:%d\n",title,l2fevent->line);
	logmsg(LOG_DEBUG,"%s: duration:%d\n",title,l2fevent->duration);
	logmsg(LOG_DEBUG,"%s: caller_number:%s\n",title,l2fevent->caller_number);
	logmsg(LOG_DEBUG,"%s: caller_name:%s\n",title,l2fevent->caller_name);
	logmsg(LOG_DEBUG,"%s: called_number:%s\n",title,l2fevent->called_number);
	logmsg(LOG_DEBUG,"%s: called_name:%s\n",title,l2fevent->called_name);
}

/*
**	format the given event
**
**	the following directives are supported
**
**	%t	type as number
**	%T	type as string
**	%i	incoming as number (0/1)
**	%I	incoming as string (IN/OUT)
**	%l  line number
**	%d  duration of the call in seconds
**	%D	duration of the call in MM:SS
**	%c	caller number
**	%C	caller name
**	%a	called number
**	%A	called name
**	%w	timestamp as UNIX time
**	%W	timestamp as string in the form DD.MM.YYYY HH:MM:SS
**
**	furthermore the following escape sequences are
**	processed
**
**	\a	alert
**	\n	newline
**	\r	rewind
**	\t	tab
*/
char *l2f_EVENT_format(const char *format,const L2F_EVENT_T *l2fevent)
{
	static char *buffer = NULL;
	const char *s = format;
	char *t;
	int len;
	struct tm *tm = localtime(&l2fevent->time);
	
	/*
	**	allocate memory for the buffer
	*/
	if (!buffer) {
		if (!(buffer = malloc(BUFSIZ))) {
			logmsg(LOG_CRIT,"malloc failed: %d %s\n",errno,strerror(errno));
			return NULL;
		}
	}
	*buffer = '\0';

	t = buffer;
	while (*s) {
		/*
		**	is there still space available in the buffer
		*/
		if ((len = buffer + BUFSIZ - 1 - t) <= 0)
			break;

		if (*s == '%') {
			/*
			**	check for directives
			*/
			switch (*++s) {
				case 't':
					/*
					**	type as number
					*/
					t += snprintf(t,len,"%d",l2fevent->type);
					break;
				case 'T':
					/*
					**	type as name
					*/
					t += snprintf(t,len,"%s",l2f_EVENT_type2string(l2fevent->type));
					break;
				case 'p':
					/*
					**	previous type as number
					*/
					t += snprintf(t,len,"%d",l2fevent->pretype);
					break;
				case 'P':
					/*
					**	previous type as name
					*/
					t += snprintf(t,len,"%s",l2f_EVENT_type2string(l2fevent->pretype));
					break;
				case 'i':
					/*
					**	direction as number
					*/
					t += snprintf(t,len,"%d",l2fevent->incoming);
					break;
				case 'I':
					/*
					**	direction as string
					*/
					t += snprintf(t,len,"%s",(l2fevent->incoming) ? "IN" : "OUT");
					break;
				case 'l':
					/*
					**	line number
					*/
					t += snprintf(t,len,"%d",l2fevent->line);
					break;
				case 'd':
					/*
					**	duration
					*/
					t += snprintf(t,len,"%d",l2fevent->duration);
					break;
				case 'D':
					/*
					**	duration
					*/
					t += snprintf(t,len,"%d:%02d",l2fevent->duration / 60,l2fevent->duration % 60);
					break;
				case 'c':
					/*
					**	caller number
					*/
					t += snprintf(t,len,"%s",(l2fevent->caller_number[0]) ? l2fevent->caller_number : l2f_PHONENUMBER_nonumber());
					break;
				case 'C':
					/*
					**	caller name
					*/
					t += snprintf(t,len,"%s",l2fevent->caller_name);
					break;
				case 'a':
					/*
					**	called number
					*/
					t += snprintf(t,len,"%s",(l2fevent->called_number[0]) ? l2fevent->called_number : l2f_PHONENUMBER_nonumber());
					break;
				case 'A':
					/*
					**	called name
					*/
					t += snprintf(t,len,"%s",l2fevent->called_name);
					break;
				case 'H':
					/*
					**	hour
					*/
					t += snprintf(t,len,"%02d",tm->tm_hour);
					break;
				case 'M':
					/*
					**	minute
					*/
					t += snprintf(t,len,"%02d",tm->tm_min);
					break;
				case 'S':
					/*
					**	second
					*/
					t += snprintf(t,len,"%02d",tm->tm_sec);
					break;
				case 'w':
					/*
					**	timestamp as UNIX time
					*/
					t += snprintf(t,len,"%lu",l2fevent->time);
					break;
				case 'W':
					/*
					**	timestamp as string in the form DD.MM.YYYY HH:MM:SS
					*/
					t += snprintf(t,len,"%s",util_datetime2string(l2fevent->time));
					break;
				case '%':
					/*
					**	add a %
					*/
					*t++ = '%';
					break;
				default:
					/*
					**	per default add %x
					*/
					*t++ = '%';
					*t++ = *s;
			}
			s++;
		}
		else if (*s == '\\') {
			/*
			**	check for escape sequences
			*/
			switch (*++s) {
				case '\\':
					/*
					**	a tab
					*/
					*t++ = '\\';
					break;
				case 'a':
					/*
					**	bing
					*/
					*t++ = '\a';
					break;
				case 'n':
					/*
					**	newline
					*/
					*t++ = '\n';
					break;
				case 'r':
					/*
					**	rewind
					*/
					*t++ = '\r';
					break;
				case 't':
					/*
					**	tab
					*/
					*t++ = '\t';
					break;
				default:
					/*
					**	per default add \x
					*/
					*t++ = '\\';
					*t++ = *s;
			}
			s++;
		}
		else
			*t++ = *s++;
	}
	*t = '\0';

	return buffer;
}

/*
**	configure this module
*/
int l2f_EVENT_configure(const char *section,const int idx,const char *key,const char *value)
{
	if (!strcmp(section,"event")) {
		return 1;
	}
	return 0;
}

/*
**	dump configuration
*/
void l2f_EVENT_configdump(void)
{
}/**/
