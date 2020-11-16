/*
**	fritzboxfon.c
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
#include "fritzboxfon.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

extern int _shutdown;

static CFG_FRITZBOXFON_T _cfg_fritzboxfon;

/*
**	parse an input line and fill up a fritzbox event struct
**
**	the input lines are of the form
**
**	outgoing call without connection:
**	17.02.08 10:59:53;CALL;1;4;879136;916142;SIP2;
**	17.02.08 11:00:00;DISCONNECT;1;0;
**
**	outgoing call with connection:
**	17.02.08 11:01:46;CALL;1;4;879136;916137;SIP2;
**	17.02.08 11:01:48;CONNECT;1;4;916137;
**	17.02.08 11:02:00;DISCONNECT;1;13;
**
**	incoming call without connection:
**	17.02.08 10:59:54;RING;2;06158879136;916142;ISDN;
**	17.02.08 11:00:00;DISCONNECT;2;0;
**
**	incoming call with connection:
**	17.02.08 11:01:47;RING;2;06158879136;916137;ISDN;
**	17.02.08 11:01:48;CONNECT;2;4;06158879136;
**	17.02.08 11:02:01;DISCONNECT;2;13;
*/
static int parse_input(const char *input,L2F_EVENT_T *l2fevent)
{
	int numparams;
	char *params[10];

	/*
	**	reset the l2fevent
	*/
	memset(l2fevent,0,sizeof(L2F_EVENT_T));

	/*
	**	split up the input string
	*/
	if ((numparams = util_splitinput(';',input,params)) < 2) {
		logmsg(LOG_WARNING,"too few parameters in input (\"%s\")\n",input);
		return 0;
	}

	/*
	**	parse the timestamp & type
	*/
	l2fevent->time = util_parsedatetime(params[0]);

	/*
	**	switch upon the type
	*/
	if (!strcasecmp(params[1],"CALL")) {
		/*
		**	outgoing call
		*/
		if (numparams < 3) {
			logmsg(LOG_WARNING,"too few parameters in input (\"%s\")\n",input);
			return 0;
		}
		l2fevent->type = L2F_EVENT_TYPE_CALL;
		l2fevent->incoming = 0;
		l2fevent->line = atoi(params[2]);
		strcpy(l2fevent->caller_number,l2f_PHONENUMBER_normalize(params[4]));
		strcpy(l2fevent->called_number,l2f_PHONENUMBER_normalize(params[5]));
	}
	else if (!strcasecmp(params[1],"RING")) {
		/*
		**	incoming call
		*/
		if (numparams < 5) {
			logmsg(LOG_WARNING,"too few parameters in input (\"%s\")\n",input);
			return 0;
		}
		l2fevent->type = L2F_EVENT_TYPE_RING;
		l2fevent->incoming = 1;
		l2fevent->line = atoi(params[2]);
		strcpy(l2fevent->caller_number,l2f_PHONENUMBER_normalize(params[3]));
		strcpy(l2fevent->called_number,l2f_PHONENUMBER_normalize(params[4]));
	}
	else if (!strcasecmp(params[1],"CONNECT")) {
		/*
		**	connect
		*/
		if (numparams < 4) {
			logmsg(LOG_WARNING,"too few parameters in input (\"%s\")\n",input);
			return 0;
		}
		l2fevent->type = L2F_EVENT_TYPE_CONNECT;
		l2fevent->line = atoi(params[2]);
		strcpy(l2fevent->caller_number,l2f_PHONENUMBER_normalize(params[4]));
	}
	else if (!strcasecmp(params[1],"DISCONNECT")) {
		/*
		**	disconnect
		*/
		if (numparams < 4) {
			logmsg(LOG_WARNING,"too few parameters in input (\"%s\")\n",input);
			return 0;
		}
		l2fevent->type = L2F_EVENT_TYPE_DISCONNECT;
		l2fevent->line = atoi(params[2]);
		l2fevent->duration = atoi(params[3]);
	}
	return 1;
}

/*
**	the socket/file descriptor for the connection to the
**	fritzbox -- this is global in here to ensure that
**	we are able to close the connection asyncronusly
*/
static FILE *_fd_fritzbox = NULL;

void l2f_FRITZBOXFON_close(void)
{
	if (_fd_fritzbox) {
		logmsg(LOG_NOTICE,"closing connection to fritzbox\n");
		fclose(_fd_fritzbox);
		_fd_fritzbox = NULL;
	}
}

/*
**	configure this module
*/
int l2f_FRITZBOXFON_configure(const char *section,const int idx,const char *key,const char *value)
{
	CFG_FRITZBOXFON_T *cfg = &_cfg_fritzboxfon;
	// TODO support multiple config sections
	
	if (!strcmp(section,"fritzboxfon")) {
		CFG_SET_KEY_VALUE_STRING(fritzboxfon,host)
		else CFG_SET_KEY_VALUE_INTEGER(fritzboxfon,port)
		else return 0;
		return 1;
	}
	return 0;
}

/*
**	dump the configuration
*/
void l2f_FRITZBOXFON_configdump(void)
{
	CFG_FRITZBOXFON_T *cfg = &_cfg_fritzboxfon;

	CFG_DUMP_KEY_VALUE_STRING(fritzboxfon,host);
	CFG_DUMP_KEY_VALUE_INTEGER(fritzboxfon,port);
}

/*
**	open the connection to the fritzbox and listen
*/
int l2f_FRITZBOXFON_process(L2F_CALLBACK_T *callbacks[])
{
	struct hostent *hostentry;
	struct sockaddr_in daddr;
	int sd,n;
	char buffer[BUFSIZ];
	char *input;
	L2F_EVENT_T lastcall;
	L2F_EVENT_T lastring;
	CFG_FRITZBOXFON_T *cfg = &_cfg_fritzboxfon;

	/*
	**	set up the destination address
	*/
	bzero(&daddr,sizeof(daddr));
	daddr.sin_family = AF_INET;
	daddr.sin_port = htons(L2F_CFG_FRITZBOXFON_PORT);
	if (!inet_aton(L2F_CFG_FRITZBOXFON_HOST,&daddr.sin_addr)) {
		if (!(hostentry = gethostbyname(L2F_CFG_FRITZBOXFON_HOST))) {
			logmsg(LOG_CRIT,"gethostbyname() failed: %d (%s)\n",errno,strerror(errno));
			return 0;
		}
		memcpy(&daddr.sin_addr.s_addr,hostentry->h_addr,sizeof(daddr.sin_addr.s_addr));
	}

	/*
	**	open the socket
	*/
	if ((sd = socket(AF_INET,SOCK_STREAM,0)) < 0) {
		logmsg(LOG_CRIT,"couldn't open socket: %d (%s)\n",errno,strerror(errno));
		return 0;
	}

	/*
	**	connect the fritzbox
	*/
	logmsg(LOG_NOTICE,"connecting host %s(%s) on port %d\n",L2F_CFG_FRITZBOXFON_HOST,
			inet_ntoa(daddr.sin_addr),L2F_CFG_FRITZBOXFON_PORT);
	if (connect(sd,(struct sockaddr *) &daddr,sizeof(daddr)) < 0 && errno != EINPROGRESS) {
		logmsg(LOG_CRIT,"couldn't connect to host: %d (%s)\n",errno,strerror(errno));
		close(sd);
		return 0;
	}

	/*
	**	create a stream
	*/
	if (!(_fd_fritzbox = fdopen(sd,"r"))) {
		logmsg(LOG_CRIT,"couldn't associate stream with socket: %d (%s)\n",errno,strerror(errno));
		close(sd);
		return 0;
	}

	/*
	**	listen for incoming messages
	*/
	memset(&lastcall,0,sizeof(lastcall));
	memset(&lastring,0,sizeof(lastring));
	while (!_shutdown) {
		L2F_EVENT_T l2fevent;

		/*
		**	wait for incoming message
		*/
		logmsg(LOG_DEBUG,"waiting for message from fritzbox ...\n");
		if (!(input = fgets(buffer,sizeof(buffer),_fd_fritzbox)))
			break;
		input = util_strdark(input);
		logmsg(LOG_DEBUG,"incoming message from fritzbox: %s\n",input);

		/*
		**	parse the incoming message
		*/
		if (!parse_input(input,&l2fevent))
			continue;

		switch (l2fevent.type) {
			case L2F_EVENT_TYPE_CALL:
				/*
				**	if it was a call, store this event
				**	for the next disconnect
				*/
				lastcall = l2fevent;
				break;
			case L2F_EVENT_TYPE_RING:
				/*
				**	if it was a ring, store this event
				**	for the next disconnect
				*/
				lastring = l2fevent;
				break;
			case L2F_EVENT_TYPE_CONNECT:
				/*
				**	this connect is a connect upon
				**	a previous ring or call
				*/
				if (l2fevent.line == lastcall.line) {
					l2fevent.incoming = 0;
					l2fevent.pretype = L2F_EVENT_TYPE_CALL;
					strcpy(l2fevent.caller_number,lastcall.caller_number);
					strcpy(l2fevent.called_number,lastcall.called_number);
				}
				if (l2fevent.line == lastring.line) {
					l2fevent.incoming = 1;
					l2fevent.pretype = L2F_EVENT_TYPE_RING;
					strcpy(l2fevent.caller_number,lastring.caller_number);
					strcpy(l2fevent.called_number,lastring.called_number);
				}
				break;
			case L2F_EVENT_TYPE_DISCONNECT:
				/*
				**	this disconnect maybe a disconnect upon
				**	a previous ring or call
				*/
				if (l2fevent.line == lastcall.line) {
					l2fevent.incoming = 0;
					l2fevent.pretype = L2F_EVENT_TYPE_CALL;
					strcpy(l2fevent.caller_number,lastcall.caller_number);
					strcpy(l2fevent.called_number,lastcall.called_number);
					memset(&lastcall,0,sizeof(lastcall));
				}
				if (l2fevent.line == lastring.line) {
					l2fevent.incoming = 1;
					l2fevent.pretype = L2F_EVENT_TYPE_RING;
					strcpy(l2fevent.caller_number,lastring.caller_number);
					strcpy(l2fevent.called_number,lastring.called_number);
					memset(&lastring,0,sizeof(lastring));
				}
				break;
		}

		/*
		**	dump the event
		*/
		l2f_EVENT_dump(__FUNC__,&l2fevent);

		/*
		**	process the callback functions
		*/
		for (n = 0;callbacks[n];n++)
			(callbacks[n])(&l2fevent);
	}

	/*
	**	close the stream
	*/
	l2f_FRITZBOXFON_close();

	return 1;
}/**/
