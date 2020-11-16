/*
**	lookup_mysql.c
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
#include "lookup.h"
#include "lookup_mysql.h"
#include "../phonenumber/phonenumber.h"
#include <string.h>
#if defined(HAVE_LIBMYSQL) || defined(HAVE_LIBMYSQLCLIENT)
#include <mysql/mysql.h>
#endif

static CFG_LOOKUP_MYSQL_T *_cfg_lookup_mysql = NULL;

/*
**	configure this module
*/
int l2f_LOOKUP_MYSQL_configure(const int idx,const char *key,const char *value)
{
#if defined(HAVE_L2F_LOOKUP_MYSQL)
	CFG_LOOKUP_MYSQL_T *cfg;

	CFG_FIND_OR_CREATE(lookup_mysql);

	CFG_SET_KEY_VALUE_STRING(lookup_mysql,host)
	else CFG_SET_KEY_VALUE_STRING(lookup_mysql,username)
	else CFG_SET_KEY_VALUE_STRING(lookup_mysql,password)
	else CFG_SET_KEY_VALUE_STRING(lookup_mysql,database)
	else CFG_SET_KEY_VALUE_STRING(lookup_mysql,table)
	else return 0;
#else
	logmsg(LOG_CRIT,"no mysql support enabled -- skipped\n");
#endif
	return 1;
}

/*
**	dump the configuration
*/
void l2f_LOOKUP_MYSQL_configdump(void)
{
	CFG_LOOKUP_MYSQL_T *cfg;

	CFG_LOOP(lookup_mysql) {
		CFG_DUMP_KEY_VALUE_STRING(lookup_mysql,host);
		CFG_DUMP_KEY_VALUE_STRING(lookup_mysql,username);
		CFG_DUMP_KEY_VALUE_STRING(lookup_mysql,password);
		CFG_DUMP_KEY_VALUE_STRING(lookup_mysql,database);
		CFG_DUMP_KEY_VALUE_STRING(lookup_mysql,table);
	}
}

/*
**	process this module
*/
const char *l2f_LOOKUP_MYSQL_process(const char *number)
{
#if defined(HAVE_L2F_LOOKUP_MYSQL)
	CFG_LOOKUP_MYSQL_T *cfg;
	MYSQL mysql;
	MYSQL_RES *result_set;
	MYSQL_ROW row;
	int n,matchdigits;
#define FILTER_MAX_DIGITS 4
	char filterpattern[FILTER_MAX_DIGITS * 2 + 2 + 1];
	char *normnumber = l2f_PHONENUMBER_normalize(number);
	char query[BUFSIZ];
	static char *name = NULL;

	if (!CFG_1ST_RECORD(lookup_mysql)) {
		logmsg(LOG_DEBUG,"lookup via mysql not configured -- skipping\n");
		return NULL;
	}
	logmsg(LOG_DEBUG,"looking up via mysql database ...\n");

	/*
	**	allocate memory for the buffer
	*/
	if (!name) {
		if (!(name = malloc(BUFSIZ))) {
			logmsg(LOG_CRIT,"malloc failed: %d %s\n",errno,strerror(errno));
			return NULL;
		}
	}
	*name = '\0';

	CFG_LOOP(lookup_mysql) {
		/*
		**	check if this module is configured
		*/
		if (!cfg->host ||
			!cfg->username || !cfg->password ||
			!cfg->database || !cfg->table) {
			logmsg(LOG_CRIT,"[%d] not configured -- skipped\n",cfg->idx);
			continue;
		}

		/*
		**	open the database
		*/
		if (!mysql_init(&mysql)) {
			logmsg(LOG_CRIT,"[%d] mysql_init() failed:%d %s\n",cfg->idx,
				mysql_errno(&mysql),mysql_error(&mysql));
			continue;
		}
		if (!mysql_real_connect(&mysql,cfg->host,cfg->username,cfg->password,cfg->database,0,NULL,0)) {
			logmsg(LOG_CRIT,"[%d] mysql_init() failed:%d %s\n",cfg->idx,
				mysql_errno(&mysql),mysql_error(&mysql));
			continue;
		}

		/*
		**	setup the filter pattern
		**
		**	We will filter for numbers which contain "*A*B*C*D*"
		**	where ABCD are the last 4 digits of the requested
		**	phone number. This should reduce the result dramatically.
		*/
		strcpy(filterpattern,"%");
		matchdigits = (strlen(normnumber) > FILTER_MAX_DIGITS) ? FILTER_MAX_DIGITS : strlen(normnumber);
		for (n = strlen(normnumber) - matchdigits;normnumber[n];n++)
			sprintf(&filterpattern[strlen(filterpattern)],"%c%%",normnumber[n]);

		/*
		**	do the lookup
		*/
		sprintf(query,"SELECT name,number FROM %s WHERE number LIKE '%s';",
			cfg->table,filterpattern);
		logmsg(LOG_DEBUG,"[%d] query: %s\n",cfg->idx,query);
		if (mysql_query(&mysql,query)) {
			logmsg(LOG_CRIT,"[%d] mysql_query() failed: %d %s\n",cfg->idx,
				mysql_errno(&mysql),mysql_error(&mysql));
			logmsg(LOG_CRIT,"[%d] statement was: %s\n",cfg->idx,query);
			continue;
		}

		/*
		**	scan the resulting rows
		*/
		if ((result_set = mysql_store_result(&mysql))) {
			while ((row = mysql_fetch_row(result_set))) {
				logmsg(LOG_DEBUG,"[%d] %s has number %s\n",cfg->idx,row[0],row[1]);
				if (l2f_PHONENUMBER_match(number,row[1])) {
					/*
					**	phone number may match
					*/
					strcpy(name,row[0]);
					logmsg(LOG_DEBUG,"[%d] %s has number %s which matches %s!\n",
							cfg->idx,row[0],row[1],number);
					break;
				}
			}
			mysql_free_result(result_set);
		}
		else {
			logmsg(LOG_DEBUG,"[%d] empty result set\n",cfg->idx);
		}

		/*
		**	close the database
		*/
		mysql_close(&mysql);

		logmsg(LOG_DEBUG,"[%d] looked up: %s=%s\n",cfg->idx,normnumber,(*name) ? name : "NULL");
		if (*name)
			break;
	}
	logmsg(LOG_DEBUG,"complete\n");
	return (*name) ? name : NULL;
#endif
	return NULL;
}

/*
**	process this module
*/
void l2f_LOOKUP_MYSQL_update(const char *number,const char *name)
{
#if defined(HAVE_L2F_LOOKUP_MYSQL)
	CFG_LOOKUP_MYSQL_T *cfg;
	MYSQL mysql;
	char *normnumber = l2f_PHONENUMBER_normalize(number);
	char query[BUFSIZ];

	if (!name || !*name) {
		logmsg(LOG_DEBUG,"given name is empty -- skipping\n");
		return;
	}

	if (!CFG_1ST_RECORD(lookup_mysql)) {
		logmsg(LOG_DEBUG,"store into mysql not configured -- skipping\n");
		return;
	}
	logmsg(LOG_DEBUG,"lookup update into mysql database ...\n");

	CFG_LOOP(lookup_mysql) {
		/*
		**	check if this module is configured
		*/
		if (!cfg->host ||
			!cfg->username || !cfg->password ||
			!cfg->database || !cfg->table) {
			logmsg(LOG_CRIT,"[%d] not configured -- skipped\n",cfg->idx);
			continue;
		}

		/*
		**	open the database
		*/
		if (!mysql_init(&mysql)) {
			logmsg(LOG_CRIT,"[%d] mysql_init() failed:%d %s\n",cfg->idx,
				mysql_errno(&mysql),mysql_error(&mysql));
			continue;
		}
		if (!mysql_real_connect(&mysql,cfg->host,cfg->username,cfg->password,cfg->database,0,NULL,0)) {
			logmsg(LOG_CRIT,"[%d] mysql_init() failed:%d %s\n",cfg->idx,
				mysql_errno(&mysql),mysql_error(&mysql));
			continue;
		}

		/*
		**	update the database
		*/
		sprintf(query,"INSERT IGNORE INTO %s SET number='%s',name='%s';",
			cfg->table,number,name);
		logmsg(LOG_DEBUG,"[%d] query: %s\n",cfg->idx,query);
		if (mysql_query(&mysql,query)) {
			logmsg(LOG_CRIT,"[%d] mysql_query() failed: %d %s\n",cfg->idx,
				mysql_errno(&mysql),mysql_error(&mysql));
			logmsg(LOG_CRIT,"[%d] statement was: %s\n",cfg->idx,query);
			continue;
		}

		/*
		**	close the database
		*/
		mysql_close(&mysql);

		logmsg(LOG_DEBUG,"[%d] look updated: %s=%s\n",cfg->idx,normnumber,name);
	}
	logmsg(LOG_DEBUG,"complete\n");
#endif
}/**/
