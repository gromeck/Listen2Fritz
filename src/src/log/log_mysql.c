/*
**	log_mysql.c
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
#include "log.h"
#include "log_mysql.h"
#if defined(HAVE_LIBMYSQL) || defined(HAVE_LIBMYSQLCLIENT)
#include <mysql/mysql.h>
#endif

static CFG_LOG_MYSQL_T *_cfg_log_mysql;

/*
**	configure this module
*/
int l2f_LOG_MYSQL_configure(const int idx,const char *key,const char *value)
{
#if defined(HAVE_L2F_LOGGING_MYSQL)
	CFG_LOG_MYSQL_T *cfg;

	CFG_FIND_OR_CREATE(log_mysql);

	CFG_SET_KEY_VALUE_STRING(log_mysql,host)
	else CFG_SET_KEY_VALUE_STRING(log_mysql,username)
	else CFG_SET_KEY_VALUE_STRING(log_mysql,password)
	else CFG_SET_KEY_VALUE_STRING(log_mysql,database)
	else CFG_SET_KEY_VALUE_STRING(log_mysql,table)
	else return 0;
#else
	logmsg(LOG_CRIT,"no mysql support enabled -- skipped\n");
#endif
	return 1;
}

/*
**	dump the configuration
*/
void l2f_LOG_MYSQL_configdump(void)
{
	CFG_LOG_MYSQL_T *cfg;

	CFG_LOOP(log_mysql) {
		CFG_DUMP_KEY_VALUE_STRING(log_mysql,host);
		CFG_DUMP_KEY_VALUE_STRING(log_mysql,username);
		CFG_DUMP_KEY_VALUE_STRING(log_mysql,password);
		CFG_DUMP_KEY_VALUE_STRING(log_mysql,database);
		CFG_DUMP_KEY_VALUE_STRING(log_mysql,table);
	}
}

/*
**	process this module
*/
int l2f_LOG_MYSQL_process(const L2F_EVENT_T *l2f_event)
{
#if defined(HAVE_L2F_LOGGING_MYSQL)
	CFG_LOG_MYSQL_T *cfg;
	MYSQL mysql;
	char query[BUFSIZ];

	if (!CFG_1ST_RECORD(log_mysql)) {
		logmsg(LOG_DEBUG,"logging to mysql database not configured -- skipping\n");
		return 0;
	}
	logmsg(LOG_DEBUG,"logging to mysql database ...\n");

	/*
	**	loop over the configuration records
	*/
	CFG_LOOP(log_mysql) {
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
		logmsg(LOG_DEBUG,"[%d] calling mysql_init()\n",cfg->idx);
		if (!mysql_init(&mysql)) {
			logmsg(LOG_CRIT,"[%d] mysql_init() failed:%d %s\n",cfg->idx,
				mysql_errno(&mysql),mysql_error(&mysql));
			continue;
		}
		logmsg(LOG_DEBUG,"[%d] calling mysql_real_connect(host=%s,username=%s,password=*****,database=%s)\n",
				cfg->idx,cfg->host,cfg->username,cfg->database);
		if (!mysql_real_connect(&mysql,cfg->host,cfg->username,cfg->password,cfg->database,0,NULL,0)) {
			logmsg(LOG_CRIT,"[%d] mysql_init() failed:%d %s\n",cfg->idx,
				mysql_errno(&mysql),mysql_error(&mysql));
			continue;
		}

		/*
		**	setup the query string
		*/
		sprintf(query,"INSERT INTO %s SET "
				"time=FROM_UNIXTIME(%lu), "
				"type=%d, "
				"incoming=%d, "
				"line=%d, "
				"duration=%d, ",
			cfg->table,
			(unsigned long) l2f_event->time,
			l2f_event->type,
			l2f_event->incoming,
			l2f_event->line,
			l2f_event->duration);
		if (l2f_event->caller_number)
			sprintf(&query[strlen(query)],"caller_number='%s', ",l2f_event->caller_number);
		else
			sprintf(&query[strlen(query)],"caller_number=NULL, ");
		if (l2f_event->called_number)
			sprintf(&query[strlen(query)],"called_number='%s';",l2f_event->called_number);
		else
			sprintf(&query[strlen(query)],"called_number=NULL;");

		/*
		**	push the data into the db
		*/
		if (mysql_query(&mysql,query)) {
			logmsg(LOG_CRIT,"[%d] mysql_query() failed: %d %s\n",cfg->idx,
				mysql_errno(&mysql),mysql_error(&mysql));
			logmsg(LOG_CRIT,"[%d] statement was: %s\n",cfg->idx,query);
			continue;
		}

		/*
		**	close the database
		*/
		mysql_commit(&mysql);
		mysql_close(&mysql);
	}
	logmsg(LOG_DEBUG,"complete\n");
#endif
	return 0;
}/**/
