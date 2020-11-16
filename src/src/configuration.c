/*
**	configuration.c
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
#include "util/util.h"
#include "configuration.h"
#include "fritzboxfon/fritzboxfon.h"
#include "event/event.h"
#include "phonenumber/phonenumber.h"
#include "lookup/lookup.h"
#include "notify/notify.h"
#include "log/log.h"

int configuration_read(const char *file)
{
	FILE *fd;
	int line = 0;
	char *s;
	char section[BUFSIZ];
	char buffer[BUFSIZ];
	char *key,*value;
	int idx = -1;
	int rc;

	/*
	**	open the config file
	*/
	if (!(fd = fopen(file,"r"))) {
		logmsg(LOG_CRIT,"couldn't open configuration file '%s': %d(%s)\n",file,errno,strerror(errno));
		return 0;
	}

	/*
	**	read the config file
	*/
	while ((s = fgets(buffer,sizeof(buffer),fd))) {
		line++;
		s = util_strdark(s);
		if (*s == '#' || *s == '\0') {
			/*
			**	skip empty lines and comments
			*/
			continue;
		}

		if (*s == '[') {
			/*
			**	new section start
			*/
			if (s[strlen(s) - 1] != ']') {
				logmsg(LOG_CRIT,"missing closing ']' in line %d in %s\n",line,file);
				return 0;
			}
			strcpy(section,s + 1);
			section[strlen(section) - 1] = '\0';
			idx++;
		}
		else if (idx >= 0) {
			/*
			**	inside a section: get key and value
			*/
			key = s;
			if (!(value = strchr(s,'='))) {
				logmsg(LOG_CRIT,"invalid configuration line in line %d in %s\n",line,file);
				return 0;
			}
			*value++ = '\0';
			key = util_strdark(key);
			value = util_strdark(value);

			/*
			**	dispatch the key & value into the sections
			*/
			rc = l2f_FRITZBOXFON_configure(section,idx,key,value);
			if (!rc)
				rc = l2f_EVENT_configure(section,idx,key,value);
			if (!rc)
				rc = l2f_PHONENUMBER_configure(section,idx,key,value);
			if (!rc)
				rc = l2f_LOOKUP_configure(section,idx,key,value);
			if (!rc)
				rc = l2f_NOTIFY_configure(section,idx,key,value);
			if (!rc)
				rc = l2f_LOG_configure(section,idx,key,value);
			if (!rc) {
				/*
				**	unknown section or key
				*/
				fclose(fd);
				logmsg(LOG_CRIT,"unknown section '%s' or key '%s' in line %d in %s\n",section,key,line,file);
				return 0;
			}
		}
		else {
			/*
			**	unknown section
			*/
			fclose(fd);
			logmsg(LOG_CRIT,"no section opened in line %d in %s\n",line,file);
			return 0;
		}
	}

	/*
	**	close the config file
	*/
	fclose(fd);
	return 1;
}/**/
