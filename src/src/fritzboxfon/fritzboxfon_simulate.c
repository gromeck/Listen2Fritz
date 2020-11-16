/*
**	simulate.c
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
#include "fritzboxfon_simulate.h"
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
#include <signal.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <limits.h>
#include <readline/readline.h>
#include <readline/history.h>

extern int _shutdown;
static char _history_file[PATH_MAX] = "";
static struct termios _otty,_savetty;

/*
**	init the simulator
*/
int l2f_simulate_fritzboxfon_init(void)
{
	char *home;

	/*
	**	overwrite the configuration so that listen2fritz will connect
	**	the simulation
	*/
	l2f_FRITZBOXFON_configure("fritzboxfon",0,"host",FRITZBOXFON_SIMULATE_DEFAULT_HOST);
	l2f_FRITZBOXFON_configure("fritzboxfon",0,"port",util_ltoa(FRITZBOXFON_SIMULATE_DEFAULT_PORT));

	/*
	**	configure the terminal
	*/
	if (isatty(fileno(stdin))) {
		//chgwinsz(SIGWINCH);
		tcgetattr(fileno(stdin),&_otty);
		_savetty = _otty;
		_otty.c_lflag &= ~(ICANON|ECHO);
	}

	/*
	**  read the history file
	*/
	if ((home = getenv("HOME"))) {
		sprintf(_history_file,"%s/." __TITLE__ ".history",home);
		logmsg(LOG_NOTICE,"reading history file %s\n",_history_file);
		read_history(_history_file);
	}
	stifle_history(100);

	return 1;
}

/*
**	close the fritzboxfon simulator
*/
int l2f_simulate_fritzboxfon_exit(pid_t pid)
{
	/*
	**	restore the terminal
	*/
	if (isatty(fileno(stdin)))
		tcsetattr(fileno(stderr),TCSANOW,&_savetty);

	/*
	**	write back stuff to the users home
	*/
	logmsg(LOG_NOTICE,"writing history file %s\n",_history_file);
	write_history(_history_file);

	/*
	**	terminate listen2fritz itself
	*/
	logmsg(LOG_NOTICE,"terminating listen2fritz daemon (pid=%lu)\n",(unsigned long) pid);
	while (kill(pid,0) == 0) {
		kill(pid,SIGTERM);
		sleep(1);
	}
	logmsg(LOG_NOTICE,"terminating listen2fritz testmode\n",(unsigned long) pid);

	return 1;
}

/*
**	send the given string to the listening clients
*/
static int l2f_simulate_fritzboxfon_send(FILE *l2fd,char *format,...)
//	__attribute__ ((format (printf, 2, 3)));
{
	va_list args;
	char buffer[BUFSIZ];

	/*
	**	get the arguments
	*/
	va_start(args,format);
	vsnprintf(buffer,sizeof(buffer),format,args);
	va_end(args);

	/*
	**	print to the console
	*/
	fputs(buffer,stdout);
	fflush(stdout);

	/*
	**	send to the listener
	*/
	if (l2fd) {
		fputs(buffer,l2fd);
		fflush(l2fd);
	}

	return 1;
}

/*
**	run the fritzboxfon simulator
**
**	the output lines are of the form
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
int l2f_simulate_fritzboxfon_main(void)
{
	struct hostent *hostentry;
	struct sockaddr_in saddr;
	int sd,sdconn;
	FILE *l2fd;
	static char *prompt = "\n" __TITLE__ "> ";
	int var_line = 4;
	int var_ring_time = 3;
	int var_talk_time = 10;

	/*
	**	open the configured socket
	*/
	bzero(&saddr,sizeof(saddr));
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(FRITZBOXFON_SIMULATE_DEFAULT_PORT);
	if (!inet_aton(FRITZBOXFON_SIMULATE_DEFAULT_HOST,&saddr.sin_addr)) {
		if (!(hostentry = gethostbyname(FRITZBOXFON_SIMULATE_DEFAULT_HOST))) {
			logmsg(LOG_CRIT,"gethostbyname() failed: %d (%s)\n",errno,strerror(errno));
			return 0;
		}
		memcpy(&saddr.sin_addr.s_addr,hostentry->h_addr,sizeof(saddr.sin_addr.s_addr));
	}

	/*
	**  open the socket
	*/
	if ((sd = socket(AF_INET,SOCK_STREAM,0)) < 0) {
		logmsg(LOG_CRIT,"couldn't open socket: %d (%s)\n",errno,strerror(errno));
		return 0;
	}

	/*
	**	bind the socket
	*/
	if (bind(sd,&saddr,sizeof(struct sockaddr_in)) < 0) {
		logmsg(LOG_CRIT,"couldn't bind socket: %d (%s)\n",errno,strerror(errno));
		return 0;
	}

	/*
	**	listen to the socket
	*/
	if (listen(sd,1) < 0) {
		logmsg(LOG_CRIT,"couldn't listen to socket: %d (%s)\n",errno,strerror(errno));
		return 0;
	}

	/*
	**	await an incoming connection
	*/
	fprintf(stdout,"waiting for " __TITLE__ " to connect ...\n");
	if ((sdconn = accept(sd,0,0)) < 0) {
		logmsg(LOG_CRIT,"couldn't accept: %d (%s)\n",errno,strerror(errno));
		return 0;
	}

	/*
	**  create a stream
	*/
	if (!(l2fd = fdopen(sdconn,"w"))) {
		logmsg(LOG_CRIT,"couldn't associate stream with socket: %d (%s)\n",errno,strerror(errno));
		close(sd);
		return 0;
	}

	fprintf(stdout,"connection established.\n");

	fprintf(stdout,"*** Welcome to the " __TITLE__ " simulator ***\n");

	while (!_shutdown) {
		/*
		**	read users input
		*/
		char *input;
		char *params[10];
		int nparams;

		if (isatty(fileno(stdin))) {
			/*
			**  get input from the user via readline
			*/
			tcsetattr(fileno(stderr),TCSANOW,&_savetty);
			if (!(input = readline(prompt)))
				break;
			tcsetattr(fileno(stderr),TCSANOW,&_otty);
		}
		else {
			/*
			**  get input from a stream via fgets
			*/
			static char buffer[BUFSIZ];

			fprintf(stdout,"%s",prompt);
			fflush(stdout);
			if (!(input = fgets(buffer,sizeof(buffer),stdin)))
				break;
			input = strdup(input);
			fprintf(stdout,"%s",input);
			fflush(stdout);
		}

		/*
		**  split up the input string
		*/
		if ((nparams = util_splitinput(' ',util_strdark(input),params)) <= 0)
			continue;

		if (!strcasecmp(params[0],"help")) {
			/*
			**	print help
			*/
			fprintf(stdout,"HELP\n\n"
				"SET line <line number>\n"
				"SET ring_time <ring duration in seconds>\n"
				"SET talk_time <duration of the call when connected in seconds>\n"
				"SHOW\n"
				"CALL [IN|OUT] [WITH CONNECTION|WITHOUT CONNECTION] [<caller number>|-] <called number>\n"
				);
		}
		else if (!strcasecmp(params[0],"show")) {
			/*
			**	show the variables
			*/
			fprintf(stderr,"line=%d\n",var_line);
			fprintf(stderr,"ring_time=%d\n",var_ring_time);
			fprintf(stderr,"talk_time=%d\n",var_talk_time);
		}
		else if (!strcasecmp(params[0],"set")) {
			/*
			**	set a variable
			*/
			if (nparams != 3) {
				fprintf(stdout,"*** command 'set' expects 3 parameters\n");
				continue;
			}
			if (!strcasecmp(params[1],"line")) {
				var_line = atoi(params[2]);
			}
			else if (!strcasecmp(params[1],"ring_time")) {
				var_ring_time = atoi(params[2]);
			}
			else if (!strcasecmp(params[1],"talk_time")) {
				var_talk_time = atoi(params[2]);
			}
			else
				fprintf(stdout,"*** '%s': unknown variable to set\n",params[1]);
		}
		else if (!strcasecmp(params[0],"call")) {
			/*
			**	simulate a call
			*/
			int call_in;
			int call_with_connection;
			char *call_caller_number;
			char *call_called_number;
			int duration = 0;

			if (nparams != 6) {
				fprintf(stdout,"*** command 'call' expects 6 parameters\n");
				continue;
			}
			if (!strcasecmp(params[1],"in") || !strcasecmp(params[1],"out"))
				call_in = !strcasecmp(params[1],"in");
			else  {
				fprintf(stdout,"*** unexpected token '%s' for command 'call'\n",params[1]);
				continue;
			}
			if (!strcasecmp(params[2],"with") || !strcasecmp(params[2],"without"))
				call_with_connection = !strcasecmp(params[2],"with");
			else  {
				fprintf(stdout,"*** unexpected token '%s' for command 'call'\n",params[2]);
				continue;
			}
			if (strcasecmp(params[3],"connection")) {
				fprintf(stdout,"*** unexpected token '%s' for command 'call'\n",params[3]);
				continue;
			}
			call_caller_number = params[4];
			call_called_number = params[5];

			/*
			**	simulate the call
			*/
			if (call_in) {
				l2f_simulate_fritzboxfon_send(l2fd,"%s;RING;2;%s;%s;ISDN;\n",
						util_datetime2string(time(NULL)),call_caller_number,call_called_number);
				sleep(var_ring_time);
				if (call_with_connection) {
					/*
					**	incoming call with connection
					*/
					l2f_simulate_fritzboxfon_send(l2fd,"%s;CONNECT;2;%d;%s;\n",
							util_datetime2string(time(NULL)),var_line,call_caller_number);
					sleep(duration = var_talk_time);
				}
				l2f_simulate_fritzboxfon_send(l2fd,"%s;DISCONNECT;2;%d;\n",
						util_datetime2string(time(NULL)),duration);
			}
			else {
				l2f_simulate_fritzboxfon_send(l2fd,"%s;CALL;1;%d:%s;%s;SIP2;\n",
						util_datetime2string(time(NULL)),var_line,call_caller_number,call_called_number);
				sleep(var_ring_time);
				if (call_with_connection) {
					/*
					**	outgoing call with connection
					*/
					l2f_simulate_fritzboxfon_send(l2fd,"%s;CONNECT;1;%d;%s;\n",
							util_datetime2string(time(NULL)),var_line,call_called_number);
					sleep(duration = var_talk_time);
				}
				l2f_simulate_fritzboxfon_send(l2fd,"%s;DISCONNECT;1;%d;\n",
						util_datetime2string(time(NULL)),duration);
			}
		}
		else if (!strcasecmp(params[0],"quit") || !strcasecmp(params[0],"exit")) {
			/*
			**	quit the simulator
			*/
			_shutdown = 1;
		}
		else {
			fprintf(stdout,"*** '%s': unknown command; type HELP\n",params[0]);
		}

		/*
		**	add the command to the hostory
		*/
		if (!_shutdown) {
			int pos;

			if ((pos = history_search_pos(input,0,0)) >= 0) {
				HIST_ENTRY *entry;

				if ((entry = remove_history(pos))) {
					free(entry->line);
					free(entry);
				}
			}
			add_history(input);
		}
	}

	fprintf(stdout,"Bye, bye.\n");
	return 1;
}/**/
