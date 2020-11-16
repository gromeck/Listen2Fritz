/*
**	lookup_mysql.h
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
#ifndef __LOOKUP_MYSQL_H__
#define __LOOKUP_MYSQL_H__	1

#include "lookup.h"

/*
**	configuration structure
*/
typedef struct _cfg_lookup_mysql_t {
	CFG_COMMON_PART(lookup_mysql);
	char *host;			// DNS or IP of the MySQL server
	char *username;		// username for MySQL access
	char *password;		// password for MySQL access
	char *database;		// name of the database to use
	char *table;		// name of the database table to use
} CFG_LOOKUP_MYSQL_T;

/*
**	configuration defaults
*/

/*
**	prototypes
*/
int l2f_LOOKUP_MYSQL_configure(const int idx,const char *key,const char *value);
void l2f_LOOKUP_MYSQL_configdump(void);
const char *l2f_LOOKUP_MYSQL_process(const char *number);
void l2f_LOOKUP_MYSQL_update(const char *number,const char *name);

#endif

/**/
