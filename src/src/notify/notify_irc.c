/*
**	notify_irc.c
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
#include "notify_irc.h"
#include "../phonenumber/phonenumber.h"
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

static CFG_NOTIFY_IRC_T *_cfg_notify_irc = NULL;
static int _stop = 0;

/*
**	configure this module
*/
int l2f_NOTIFY_IRC_configure(const int idx,const char *key,const char *value)
{
#if defined(HAVE_L2F_NOTIFY_IRC)
	CFG_NOTIFY_IRC_T *cfg;

	CFG_FIND_OR_CREATE(notify_irc);

	CFG_SET_KEY_VALUE_STRING(notify_irc,host)
	else CFG_SET_KEY_VALUE_INTEGER(notify_irc,port)
	else CFG_SET_KEY_VALUE_STRING(notify_irc,nick)
	else CFG_SET_KEY_VALUE_STRING(notify_irc,realname)
	else CFG_SET_KEY_VALUE_STRING(notify_irc,buddies)
	else CFG_SET_KEY_VALUE_STRING(notify_irc,text)
	else CFG_SET_KEY_VALUE_STRING(notify_irc,caller_number)
	else CFG_SET_KEY_VALUE_STRING(notify_irc,called_number)
	else return 0;
	return 1;
#else
	logmsg(LOG_CRIT,"no notification support via irc enabled -- skipped\n");
#endif
}

/*
**	dump the configuration
*/
void l2f_NOTIFY_IRC_configdump(void)
{
	CFG_NOTIFY_IRC_T *cfg;

	CFG_LOOP(notify_irc) {
		CFG_DUMP_KEY_VALUE_STRING(notify_irc,host);
		CFG_DUMP_KEY_VALUE_INTEGER(notify_irc,port);
		CFG_DUMP_KEY_VALUE_STRING(notify_irc,nick);
		CFG_DUMP_KEY_VALUE_STRING(notify_irc,realname);
		CFG_DUMP_KEY_VALUE_STRING(notify_irc,buddies);
		CFG_DUMP_KEY_VALUE_STRING(notify_irc,text);
		CFG_DUMP_KEY_VALUE_STRING(notify_irc,caller_number);
		CFG_DUMP_KEY_VALUE_STRING(notify_irc,called_number);
	}
}

/*
**  the alarm handler
*/
static void alarm_handler(int sig)
{
	logmsg(LOG_CRIT,"received signal %d (%s)\n",sig,strsignal(sig));
	_stop = 1;
}

/*
**	process this module
*/
int l2f_NOTIFY_IRC_process(const L2F_EVENT_T *l2f_event)
{
#if defined(HAVE_L2F_NOTIFY_IRC)
	struct hostent *hostentry;
	struct sockaddr_in daddr;
	int sd;
	FILE *fd;
	char *s,*nl;
	char buffer[BUFSIZ];
	char *input;
	CFG_NOTIFY_IRC_T *cfg;

	if (!CFG_1ST_RECORD(notify_irc)) {
		logmsg(LOG_DEBUG,"notification via irc not configured -- skipping\n");
		return 0;
	}
	logmsg(LOG_DEBUG,"notification via irc ...\n");

	/*
	**	filter only incoming rings
	*/
	if (l2f_event->type != L2F_EVENT_TYPE_RING) {
		logmsg(LOG_DEBUG,"event doesn't match module policy -- event ignoring\n");
		return 0;
	}

	/*
	**	loop over the configuration records
	*/
	CFG_LOOP(notify_irc) {
		/*
		**	check if this module is configured
		*/
		if (!cfg->host || !cfg->port ||
			!cfg->nick || !cfg->realname ||
			!cfg->buddies || !cfg->text) {
			logmsg(LOG_CRIT,"[%d] not configured -- skipped\n",cfg->idx);
			continue;
		}

		/*
		**	check if the caller number matches
		*/
		if (cfg->caller_number && !l2f_PHONENUMBER_match(l2f_event->caller_number,cfg->caller_number)) {
			logmsg(LOG_CRIT,"[%d] configured caller_number (%s) doesn't match the caller number (%s) -- skipped\n",
					cfg->idx,cfg->caller_number,l2f_event->caller_number);
			continue;
		}

		/*
		**	check if the called number matches
		*/
		if (cfg->called_number && !l2f_PHONENUMBER_match(l2f_event->called_number,cfg->called_number)) {
			logmsg(LOG_CRIT,"[%d] configured called_number (%s) doesn't match the called number (%s) -- skipped\n",
					cfg->idx,cfg->called_number,l2f_event->called_number);
			continue;
		}

		/*
		**	set up the destination address
		*/
		bzero(&daddr,sizeof(daddr));
		daddr.sin_family = AF_INET;
		daddr.sin_port = htons(cfg->port);
		if (!inet_aton(cfg->host,&daddr.sin_addr)) {
			if (!(hostentry = gethostbyname(cfg->host))) {
				logmsg(LOG_CRIT,"[%d] gethostbyname() failed: %d (%s)\n",
						cfg->idx,errno,strerror(errno));
				continue;
			}
			memcpy(&daddr.sin_addr.s_addr,hostentry->h_addr,sizeof(daddr.sin_addr.s_addr));
		}

		/*
		**	open the socket
		*/
		if ((sd = socket(AF_INET,SOCK_STREAM,0)) < 0) {
			logmsg(LOG_CRIT,"[%d] couldn't open socket: %d (%s)\n",
					cfg->idx,errno,strerror(errno));
			continue;
		}

		/*
		**	connect the IRC server
		*/
		logmsg(LOG_NOTICE,"[%d] connecting IRC server %s(%s)\n",
				cfg->idx,cfg->host,inet_ntoa(daddr.sin_addr));
		if (connect(sd,(struct sockaddr *) &daddr,sizeof(daddr)) < 0 && errno != EINPROGRESS) {
			logmsg(LOG_CRIT,"[%d] couldn't connect to IRC server: %d (%s)\n",
					cfg->idx,errno,strerror(errno));
			close(sd);
			continue;
		}

		/*
		**	create a stream
		*/
		if (!(fd = fdopen(sd,"r+"))) {
			logmsg(LOG_CRIT,"[%d] couldn't associate stream with socket: %d (%s)\n",
					cfg->idx,errno,strerror(errno));
			close(sd);
			continue;
		}

		/*
		**	login
		*/
		logmsg(LOG_DEBUG,"[%d] logging into IRC server %s(%s)\n",
				cfg->idx,cfg->host,inet_ntoa(daddr.sin_addr));
		fprintf(fd,"NICK %s\n",cfg->nick);
		fprintf(fd,"USER %s 0 * :%s\n",cfg->nick,cfg->realname);

		/*
		**	see what the IRC server has to say
		*/
		signal(SIGALRM,alarm_handler);
		_stop = 0;
		alarm(2);
		while (!_stop && (input = fgets(buffer,sizeof(buffer),fd))) {
			input = util_strdark(input);
			logmsg(LOG_DEBUG,"IRC-Server: %s\n",input);
		}
		alarm(0);

		/*
		**	send the message to my buddies
		*/
		logmsg(LOG_DEBUG,"[%d] notifying buddies \"%s\"\n",
				cfg->idx,cfg->buddies);
		strcpy(buffer,l2f_EVENT_format(cfg->text,l2f_event));
		for (s = buffer;s;s = (nl) ? nl + 1 : NULL) {
			if ((nl = strchr(s,'\n')))
				*nl = '\0';
			if (*s)
				fprintf(fd,"PRIVMSG %s :%s\n",cfg->buddies,s);
			logmsg(LOG_DEBUG,"IRC-Client: %s: %s\n",cfg->buddies,s);
		}

		/*
		**	see what the IRC server has to say
		*/
		signal(SIGALRM,alarm_handler);
		alarm(1);
		while (!_stop && (input = fgets(buffer,sizeof(buffer),fd))) {
			input = util_strdark(input);
			logmsg(LOG_DEBUG,"IRC-Server: %s\n",input);
		}
		alarm(0);

		/*
		**	close the IRC stream
		*/
		logmsg(LOG_DEBUG,"[%d] closing connection to IRC server %s(%s)\n",
				cfg->idx,cfg->host,inet_ntoa(daddr.sin_addr));
		fclose(fd);
	}
	logmsg(LOG_DEBUG,"complete\n");
#endif
	return 0;
}/**/
