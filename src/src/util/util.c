/*
**	util.c
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
#include <syslog.h>
#include <time.h>
#include <sys/fcntl.h>
#include "util.h"

/*
**  make a string out of a long value
*/
char *util_ltoa(long x)
{
#define ROTATE  50
	static int rotate = 0;
	static char rotate_buffer[ROTATE][20];
	char *buffer = rotate_buffer[rotate++ % ROTATE];
	char *s = buffer + sizeof(rotate_buffer[0]);
	int negative = 0;

	if (x < 0) {
		/*
		**  x is negative
		*/
		x = -x;
		negative = 1;
	}

	/*
	**  add the '\0' and do a little horner schema
	*/
	*--s = '\0';
	do
		*--s = '0' + (x % 10);
	while (x /= 10);

	/*
	**  add the '-' sign if neccessary
	*/
	if (negative)
		*--s = '-';

	return s;
}

/*
**	parse date & time of the form
**
**	DD.MM.YYYY HH:MM:SS
*/
time_t util_parsedatetime(char *s)
{
	time_t now = time(NULL);
	struct tm tm;

	/*
	**	init the struct with the current time
	**	this will at least take over the current
	**	daylight saving flag -- all other values
	**	will be overwritten
	*/
	localtime_r(&now,&tm);

	/*
	**	parse the input
	*/
	sscanf(s,"%d.%d.%d %d:%d:%d",
		&tm.tm_mday,&tm.tm_mon,&tm.tm_year,
		&tm.tm_hour,&tm.tm_min,&tm.tm_sec);

	/*
	**	do some corrections
	*/
	tm.tm_mon -= 1;
	if (tm.tm_year < 100)
		tm.tm_year += 2000;
	tm.tm_year -= 1900;

	/*
	**	return the UNIX time
	*/
	return mktime(&tm);
}

/*
**	genereate a time stamp of the form
**
**	DD.MM.YYYY HH:MM:SS
*/
char *util_datetime2string(time_t datetime)
{
#define ROTATION	10
	static char _buffer[ROTATION][24];
	static int rotation = 0;
	char *buffer = _buffer[rotation++ % ROTATION];
#undef ROTATION

	strftime(buffer,24,"%d.%m.%Y %H:%M:%S",localtime(&datetime));

	return buffer;
}

/*
**	split up an input line into an array of strings
**	semicolon is used as a seperator
*/
int util_splitinput(const char sep,const char *input,char *params[])
{
	static char buffer[BUFSIZ];
	int n = 0;
	char *s;

	strcpy(buffer,input);
	params[n++] = buffer;
	for (s = buffer;*s;s++)
		if (*s == sep) {
			*s = '\0'; 
			params[n++] = s + 1;
		}
	if (!*params[0])
		n = 0;
	return n;
}

/*  
**	make a process a daemon by detaching the TTY
*/
int util_daemonize(void)
{   
	int fd;

	/* 
	**  detach the current TTY by closing all
	**  descriptors
	*/
	for (fd = getdtablesize();fd >= 0;--fd)
		close(fd);

	/*
	**  recreate stdin, stdout & stderr
	*/
	fd = open("/dev/null",O_RDWR);
	dup(fd);
	dup(fd);

	/*
	**  start a new session
	*/
	return setsid();
}

/*
**  do the logging
**
**  if enabled, we log all given messages to syslog; everything which is not
**  LOG_INFO will also be written to stderr;
**  if the syslog feature is disabled, every message will be printed to stderr
**
**  if line < 0 no location information (file, func, line) will be added to the output
*/
void util_logmsg(int prio,const char *file,const char *func,int line,const char *fmt,...)
{
	static int init = 0;
	int do_syslog = 0,do_print = 1;
	const char *type = "UNKNOWN";
	va_list va;
	static char location[BUFSIZ];
	static char buffer[BUFSIZ];
	time_t now = time(NULL);

	if (!init) {
		/*
		**  open the syslog
		*/
		openlog(PACKAGE, (LOG_CONS | LOG_NDELAY | LOG_PID), LOG_USER);
		init = 1;
	}

	/*
	**  format the output
	*/
	va_start(va,fmt);

#if defined (__GNU_LIBRARY__) && __GLIBC__ >= 2 && __GLIBC_MINOR__ < 1
	if (vsnprintf(buffer,sizeof(buffer),fmt,va) < 0) {
#else
	if (vsnprintf(buffer,sizeof(buffer),fmt,va) > sizeof(buffer)) {
#endif
		buffer[sizeof(buffer)-1]='\0'; /* force string termination */
	}
	va_end(va);

	/*
	**  print this also to stderr
	*/
	switch (prio) {
		case LOG_EMERG:     type = "EMERG";     do_syslog = 1;  do_print = 1; break;
		case LOG_ALERT:     type = "ALERT";     do_syslog = 1;  do_print = 1; break;
		case LOG_CRIT:      type = "CRIT";      do_syslog = 1;  do_print = 1; break;
		case LOG_ERR:       type = "ERR";       do_syslog = 1;  do_print = 1; break;
		case LOG_WARNING:   type = "WARNING";   do_syslog = 1;  do_print = 1; break;
		case LOG_NOTICE:    type = "NOTICE";    do_syslog = 1;  do_print = 1; break;
		case LOG_INFO:      type = "INFO";      do_syslog = 0;  do_print = 1; break;
		case LOG_DEBUG:     type = "DEBUG";     do_syslog = 0;  do_print = 1; break;
	}

	/*
	**  format the location info
	*/
	sprintf(location,"%s:%s:%d",file,func,line);

	/*
	**  do sys logging
	*/
	if (do_syslog) {
		if (line < 0) {
			/*
			**  log without location info
			*/
			syslog(prio, "%s", buffer);
		}
		else {
			/*
			**  log with location info
			*/
			syslog(prio, "%s: %s", location, buffer);
		}
	}

	if (do_print) {
		struct tm *timestamp = localtime(&now);

		fprintf(stderr,"%04d-%02d-%02d %02d:%02d:%02d %s[%lu]:%s:%s: %s",
			timestamp->tm_year + 1900,timestamp->tm_mon + 1,timestamp->tm_mday,
			timestamp->tm_hour,timestamp->tm_min,timestamp->tm_sec,
			PACKAGE,(unsigned long) getpid(),type,location,buffer);
	}
}

/*
**	dark a string - skip white at head & tail of a string
*/
char *util_strdark(char *s)
{
	if (s && *s) {
		char *t = s + strlen(s) - 1;

		while (isspace(*s))
			s++;
		while (isspace(*t))
			*t-- = '\0';
	}
	return s;
}/**/
