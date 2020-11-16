/*
**	pidfile.c
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
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <errno.h>
#include "config.h"
#include "util.h"
#include "pidfile.h"

#define PIDFILE_PATH	LOCALSTATEDIR

/*
**	return a pointer to the pidfile
*/
static char *pidfile(const char *progname)
{
	static char file[PATH_MAX];
	char *s;

	if ((s = strrchr(progname,'/')))
		progname = ++s;

	sprintf(file,"%s/%s.pid",PIDFILE_PATH,progname);

	return file;
}

/*
**	check the pidfile
*/
int pidfile_check(const char *progname)
{
	pid_t pid;

	if ((pid = pidfile_read(progname))) {
		/*
		**	if the old pid doesn't exists, we will
		**	remove the pidfile
		*/
		if (kill(pid,0) < 0)
			pidfile_remove(progname);
		else
			return 0;
	}

	/*
	**	write a new exclusive pidfile
	*/
	return pidfile_write(progname,1);
}

/*
**	write the pidfile
**
**	if <exclusive> is set and the pid file already exists,
**	this function will fail.
*/
int pidfile_write(const char *progname,int exclusive)
{
	char *file = pidfile(progname);
	char pid[20];
	int fd;

	sprintf(pid,"%ld\n",(long) getpid());
	if ((fd = open(file,O_CREAT|((exclusive) ? O_EXCL : 0)|O_TRUNC|O_RDWR,S_IRWXU)) < 0) {
		logmsg(LOG_CRIT,"couldn't create pidfile '%s': %d (%s)\n",file,errno,strerror(errno));
		return 0;
	}
	if (write(fd,pid,strlen(pid)) < 0) {
		close(fd);
		logmsg(LOG_CRIT,"couldn't write pidfile '%s': %d (%s)\n",file,errno,strerror(errno));
		return 0;
	}
	close(fd);
	return 1;
}

/*
**	read the pidfile and return the pid
**
**	if the pid file can not be found, 0 is returned
**	otherwise the pid is returned
*/
pid_t pidfile_read(const char *progname)
{
	char *file = pidfile(progname);
	int fd;
	char pid[20];
	int len;

	if ((fd = open(file,O_RDONLY)) < 0) {
		return 0;
	}
	if ((len = read(fd,pid,sizeof(pid))) < 0) {
		close(fd);
		logmsg(LOG_CRIT,"couldn't read pidfile '%s': %d (%s)\n",file,errno,strerror(errno));
		return 0;
	}
	close(fd);
	pid[len] = '\0';
	return atol(pid);
}

/*
**	remove the given pidfile
**
**	return 1 for success, otherwise 0
*/
int pidfile_remove(const char *progname)
{
	char *file = pidfile(progname);

	if (unlink(file) < 0) {
		logmsg(LOG_CRIT,"couldn't remove pidfile '%s': %d (%s)\n",file,errno,strerror(errno));
		return 0;
	}
	return 1;
}/**/
