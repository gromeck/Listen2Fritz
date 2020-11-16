/*
**	notify_mail.c
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
#include "notify_mail.h"
#include "../phonenumber/phonenumber.h"

#define PASS_FROM_ADDR_AS_ARG	1

static CFG_NOTIFY_MAIL_T *_cfg_notify_mail = NULL;

/*
**	configure this module
*/
int l2f_NOTIFY_MAIL_configure(const int idx,const char *key,const char *value)
{
#if defined(HAVE_L2F_NOTIFY_MAIL)
	CFG_NOTIFY_MAIL_T *cfg;

	CFG_FIND_OR_CREATE(notify_mail);

	CFG_SET_KEY_VALUE_STRING(notify_mail,fromaddr)
	else CFG_SET_KEY_VALUE_STRING(notify_mail,toaddr)
	else CFG_SET_KEY_VALUE_STRING(notify_mail,subject)
	else CFG_SET_KEY_VALUE_STRING(notify_mail,text)
	else CFG_SET_KEY_VALUE_STRING(notify_mail,caller_number)
	else CFG_SET_KEY_VALUE_STRING(notify_mail,called_number)
	else return 0;
#else
	logmsg(LOG_CRIT,"no notification support via mail enabled -- skipped\n");
#endif
	return 1;
}

/*
**	dump the configuration
*/
void l2f_NOTIFY_MAIL_configdump(void)
{
	CFG_NOTIFY_MAIL_T *cfg;

	CFG_LOOP(notify_mail) {
		CFG_DUMP_KEY_VALUE_STRING(notify_mail,fromaddr);
		CFG_DUMP_KEY_VALUE_STRING(notify_mail,toaddr);
		CFG_DUMP_KEY_VALUE_STRING(notify_mail,subject);
		CFG_DUMP_KEY_VALUE_STRING(notify_mail,text);
		CFG_DUMP_KEY_VALUE_STRING(notify_mail,caller_number);
		CFG_DUMP_KEY_VALUE_STRING(notify_mail,called_number);
	}
}

/*
**	process this module
*/
int l2f_NOTIFY_MAIL_process(const L2F_EVENT_T *l2f_event)
{
#if defined(HAVE_L2F_NOTIFY_MAIL)
	pid_t pid;
	int pfildes[2];
	char subject[BUFSIZ];
	char mailtext[BUFSIZ];
	CFG_NOTIFY_MAIL_T *cfg;

	if (!CFG_1ST_RECORD(notify_mail)) {
		logmsg(LOG_DEBUG,"notification via mail not configured -- skipping\n");
		return 0;
	}
	logmsg(LOG_DEBUG,"notifying via mail ...\n");

	/*
	**	filter only disconnected rings without connection
	*/
	if (l2f_event->type != L2F_EVENT_TYPE_DISCONNECT || l2f_event->duration || l2f_event->pretype != L2F_EVENT_TYPE_RING) {
		logmsg(LOG_DEBUG,"event doesn't match module policy -- event ignoring\n");
		return 0;
	}

	/*
	**	loop over the configuration records
	*/
	CFG_LOOP(notify_mail) {
		/*
		**	check if this module is configured
		*/
		if (!cfg->fromaddr || !cfg->toaddr ||
			!cfg->subject || !cfg->text) {
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

		logmsg(LOG_DEBUG,"[%d] registered disconnect with duration == 0\n",cfg->idx);

		/*
		**	setup the subject and the mail text
		*/
		strcpy(subject,l2f_EVENT_format(cfg->subject,l2f_event));
		strcpy(mailtext,l2f_EVENT_format(cfg->text,l2f_event));

		/*
		**	Make pipes
		*/
		if (pipe(pfildes) < 0) {
			logmsg(LOG_CRIT,"[%d] couldn't create pipes: %d (%s)\n",cfg->idx,errno,strerror(errno));
			continue;
		}

		/*
		**	for myself
		*/
		if ((pid = fork()) < 0) {
			logmsg(LOG_CRIT,"[%d] couldn't fork: %d (%s)\n",cfg->idx,errno,strerror(errno));
			continue;
		}
		if (pid) {
			/*
			**	we are the father process
			*/
			FILE *fd;

			logmsg(LOG_DEBUG,"[%d] sending mail text to spawned mail\n",cfg->idx);
			close(pfildes[0]);
			if ((fd = fdopen(pfildes[1],"w"))) {
#if !PASS_FROM_ADDR_AS_ARG
				fprintf(fd,"From: %s\n",cfg->fromaddr);
#endif
				fputs(mailtext,fd);
				fclose(fd);
			}
			else
				logmsg(LOG_CRIT,"[%d] fdopen() failed\n",cfg->idx);
		}
		else {
			/*
			**	child process
			*/
			logmsg(LOG_DEBUG,"[%d] spawning mail\n",cfg->idx);
			dup2(pfildes[0],0);
//			close(pfildes[0]);
			close(pfildes[1]);
			if (execlp("mail","mail","-s",subject,
#if PASS_FROM_ADDR_AS_ARG
					"-r",cfg->fromaddr,
#endif
					cfg->toaddr,NULL) < 0) {
				logmsg(LOG_CRIT,"[%d] couldn't execlp: %d (%s)\n",cfg->idx,errno,strerror(errno));
			}
			logmsg(LOG_DEBUG,"[%d] spawning mail failed\n",cfg->idx);
			exit(1);
		}
	}
	logmsg(LOG_DEBUG,"complete\n");
#endif
	return 1;
}/**/
