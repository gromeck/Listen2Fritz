/*
**	listen2fritz.c
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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <getopt.h>
#include "listen2fritz.h"
#include "util/util.h"
#include "util/pidfile.h"
#include "fritzboxfon/fritzboxfon.h"
#include "fritzboxfon/fritzboxfon_simulate.h"
#include "phonenumber/phonenumber.h"
#include "event/event.h"
#include "lookup/lookup.h"
#include "notify/notify.h"
#include "log/log.h"

int _shutdown = 0;

/*
**  the shutdown handler sets <_shutdown> to non-zero
*/
static void shutdown_handler(int sig)
{
	logmsg(LOG_CRIT,"received signal %d (%s) -- shutting down\n",
			sig,strsignal(sig));
	l2f_FRITZBOXFON_close();
	_shutdown = 1;
}

/*
**  the child handler waits for terminating childs
*/
static void child_handler(int sig)
{
	int status;
	pid_t pid;

#if 0
	logmsg(LOG_CRIT,"received signal %d (%s) -- waiting for a child to terminate\n",
			sig,strsignal(sig));
	while ((pid = waitpid(-1,&status,WNOHANG)) > 0)
		logmsg(LOG_CRIT,"waitpid() returned %lu\n",(unsigned long) pid);
#else
	while ((pid = waitpid(-1,&status,WNOHANG)) > 0)
		;
#endif
}

/*
**	print the usage
*/
static void usage(const char *argv0)
{
	fprintf(stderr,"Usage: %s [options]\n",argv0);
	fprintf(stderr,"Options:\n");
	fprintf(stderr," -V\n --version\n"
					"          print version number an exit\n");
	fprintf(stderr," -?\n --help\n"
					"          show this help\n");
	fprintf(stderr," -c <file>\n --config <file>\n"
					"          use <file> to read configuration\n");
	fprintf(stderr," -p\n --print\n"
					"          read config file and dump the configuration, then terminate\n");
	fprintf(stderr," -t\n --testmode\n"
					"          testmode operation\n");
	fprintf(stderr," -f\n --foreground\n"
					"          normal operation, but run in foreground\n");
	exit(-1);
}

int main(int argc,char *argv[])
{
	int c,n,m;
	int printmode = 0;
	int testmode = 0;
	int foreground = 0;
	int backoff = 1;
	L2F_CALLBACK_T *callbacks[] = {
			l2f_LOOKUP_process,
			l2f_NOTIFY_process,
			l2f_LOG_process,
			NULL };

	char *config_file = NULL;
	char short_options[BUFSIZ];
	struct option long_options[] = {
		{ "version",				0,	0,	'V' },
		{ "help",					0,	0,	'?' },
		{ "config",					1,	0,	'c' },
		{ "printmode",				0,	0,	'p' },
		{ "testmode",				0,	0,	't' },
		{ "foreground",				0,	0,	'f' },
		{ NULL,						0,	0,	0	},
	};

	/*
	**	setup the short option string
	*/
	for (n = m = 0;long_options[n].name;n++) {
		short_options[m++] = long_options[n].val;
		if (long_options[n].has_arg)
			short_options[m++] = ':';
	}
	short_options[m++] = '\0';

	while ((c = getopt_long(argc,argv,short_options,long_options,NULL)) >= 0) {
		switch (c) {
			case 'V':	/*
						**	print version and exit
						*/
						printf("*** %s Version %s ***\n",__TITLE__,__VERSION_NR__);
						printf("(c) 2007-2015 by Christian Lorenz\n");

						printf("support for logging to file is "
#if defined(HAVE_L2F_LOGGING_FILE)
						"enabled"
#else
						"disabled"
#endif
						"\n");
						printf("support for logging to mysql is "
#if defined(HAVE_L2F_LOGGING_MYSQL)
						"enabled"
#else
						"disabled"
#endif
						"\n");
						printf("support for logging to syslog is "
#if defined(HAVE_L2F_LOGGING_SYSLOG)
						"enabled"
#else
						"disabled"
#endif
						"\n");
						printf("support for lookup via ldap is "
#if defined(HAVE_L2F_LOOKUP_LDAP)
						"enabled"
#else
						"disabled"
#endif
						"\n");
						printf("support for lookup via mysql is "
#if defined(HAVE_L2F_LOOKUP_MYSQL)
						"enabled"
#else
						"disabled"
#endif
						"\n");
						printf("support for lookup via script is "
#if defined(HAVE_L2F_LOOKUP_SCRIPT)
						"enabled"
#else
						"disabled"
#endif
						"\n");
						printf("support for notify via dreambox is "
#if defined(HAVE_L2F_NOTIFY_DREAMBOX)
						"enabled"
#else
						"disabled"
#endif
						"\n");
						printf("support for notify via irc is "
#if defined(HAVE_L2F_NOTIFY_IRC)
						"enabled"
#else
						"disabled"
#endif
						"\n");
						printf("support for notify via mail is "
#if defined(HAVE_L2F_NOTIFY_MAIL)
						"enabled"
#else
						"disabled"
#endif
						"\n");
						printf("support for notify via script is "
#if defined(HAVE_L2F_NOTIFY_SCRIPT)
						"enabled"
#else
						"disabled"
#endif
						"\n");
						exit(0);
						break;
			case '?':	/*
						**	print the usage
						*/
						goto usage;
						break;
			case 'c':	/*
						**	config file
						*/
						config_file = optarg;
						break;
			case 'p':	/*
						**	run in printmode
						*/
						printmode = 1;
						foreground = 1;
						break;
			case 't':	/*
						**	run in testmode
						*/
						testmode = 1;
						break;
			case 'f':	/*
						**	run in foreground
						*/
						foreground = 1;
						break;
			default:
			usage:		/*
						**	usage
						*/
						fprintf(stderr,"%s: unknown option %c\n",
							__TITLE__,c);
						usage(argv[0]);
						exit(-1);
						break;
		}
	}

	/*
	**	set signal handlers
	*/
	signal(SIGHUP,SIG_IGN);
	signal(SIGINT,shutdown_handler);
	signal(SIGQUIT,shutdown_handler);
	signal(SIGABRT,shutdown_handler);
	signal(SIGTERM,shutdown_handler);
	signal(SIGUSR1,SIG_IGN);
	signal(SIGUSR2,SIG_IGN);
	signal(SIGCHLD,child_handler);
	signal(SIGTRAP,SIG_IGN);
	signal(SIGALRM,SIG_IGN);
	signal(SIGURG,SIG_IGN);
	signal(SIGPIPE,SIG_IGN);
	signal(SIGVTALRM,SIG_IGN);

	/*
	**	load the configuration
	*/
	if (!configuration_read((config_file) ? config_file : DEFAULT_CONFIG_FILE))
		exit(1);

	/*
	**	dump the configuration
	*/
	if (printmode) {
		l2f_FRITZBOXFON_configdump();
		l2f_EVENT_configdump();
		l2f_PHONENUMBER_configdump();
		l2f_LOOKUP_configdump();
		l2f_NOTIFY_configdump();
		l2f_LOG_configdump();
		exit(1);
	}

	/*
	**	if in testmode, we have to setup the
	**	configuration to let listen2fritz
	**	connect locally
	*/
	if (testmode) {
		l2f_simulate_fritzboxfon_init();
		foreground = 0;
	}

	/*
	**	start myself into the background
	*/
	if (foreground) {
		logmsg(LOG_NOTICE,"running in foreground\n");
	}
	else {
		/*
		**	start in the background
		*/
		pid_t pid;
		
		if ((pid = fork()) < 0) {
			logmsg(LOG_CRIT,"couldn't fork: %d(%s) -- aborting\n",errno,strerror(errno));
			exit(1);
		}
		if (pid) {
			/*
			**	father process
			**
			**	if not on testmode simply terminate
			*/
			if (testmode) {
				/*
				**	if the testmode is set, we will
				**	run the simulator in foreground
				*/
				l2f_simulate_fritzboxfon_main();
				l2f_simulate_fritzboxfon_exit(pid);
			}
			exit(0);
		}

		/*
		**	the child will detach the tty
		*/
		if (!testmode)
			util_daemonize();
	}

	/*
	**  create the pid file
	*/
	if (!testmode && !pidfile_check(argv[0])) {
		logmsg(LOG_CRIT,"pidfile_check() failed!\n");
		exit(-1);
	}

	/*
	**	start the listener in a loop to ensure
	**	a reconnect after a disconnect
	*/
	do {
		if (!l2f_FRITZBOXFON_process(callbacks)) {
			/*
			**	the connect failed, so maybe the
			**	fritzbox got unreachable
			*/
			backoff = min(backoff * 2,60 * 60);
			logmsg(LOG_NOTICE,"connect to fritzbox failed ... delaying reconnect by %u seconds\n",backoff);
		}
		else
			backoff = 1;

		sleep(backoff);
	} while (!_shutdown);

	/*
	**	remove the pidfile
	*/
	if (!testmode)
		pidfile_remove(argv[0]);

	return 0;
}/**/
