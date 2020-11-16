/*
**	util.h
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
#ifndef __UTIL_H__
#define __UTIL_H__ 1

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <syslog.h>
#include "../../config.h"

/**/
/*
**  use the GNU feature macro __(PRETTY_)FUNCTION__ if possible
*/
#ifdef __GNUC__
#define __FUNC__                        (__FUNCTION__)
#else
#define __FUNC__                        ("")
#endif

/*
**	some macros
*/
#define min(a,b)	((a) < (b) ? (a) : (b))

/*
**  define the logmsg() as a macro
*/
#define logmsg(prio,format...)       util_logmsg(prio,__FILE__,__FUNC__,__LINE__,format)

char *util_ltoa(long x);
time_t util_parsedatetime(char *s);
char *util_datetime2string(time_t datetime);
int util_splitinput(const char sep,const char *input,char *params[]);
int util_daemonize(void);
void util_logmsg(int prio,const char *file,const char *func,int line,const char *fmt,...);
char *util_strdark(char *s);

#endif
