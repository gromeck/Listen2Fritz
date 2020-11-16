/*
**	configuration.h
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
#ifndef __CONFIGURATION_H__
#define __CONFIGURATION_H__ 1

#include "../config.h"
#include "listen2fritz.h"

/*
**	common stuff for the configuration structures
*/
#define CFG_COMMON_PART(typename)	\
	int idx; \
	struct _cfg_ ## typename ## _t *next;

/*
**	find or create a cfg record
*/
#define CFG_FIND_OR_CREATE(typename) \
	{ \
		for (cfg = _cfg_ ## typename;cfg;cfg = cfg->next) \
			if (cfg->idx == idx) \
				break; \
		if (!cfg) { \
			/* \
			**	create a new record \
			*/ \
			if ((cfg = (struct _cfg_ ## typename ## _t *) malloc(sizeof(*cfg)))) { \
				memset(cfg,0,sizeof(*cfg)); \
				cfg->idx = idx; \
				cfg->next = _cfg_ ## typename; \
				_cfg_ ## typename = cfg; \
			} \
		} \
	}

#define CFG_LOOP(typename) \
		for (cfg = _cfg_ ## typename;cfg;cfg = cfg->next)

#define CFG_1ST_RECORD(typename) \
		_cfg_ ## typename

/*
**	set values in a configuration record
*/
#define CFG_SET_KEY_VALUE_INTEGER(s,k) \
		if (!strcmp(key,# k)) cfg->k = atoi(value);
#define CFG_SET_KEY_VALUE_STRING(s,k) \
		if (!strcmp(key,# k)) cfg->k = strdup(value);

/*
**	dump values of a configuration record
*/
#define CFG_DUMP_KEY_VALUE_INTEGER(s,k) \
		logmsg(LOG_INFO,# s "[%d] " # k "=%ld\n",cfg->idx,(long) cfg->k);
#define CFG_DUMP_KEY_VALUE_STRING(s,k) \
		logmsg(LOG_INFO,# s "[%d] " # k "=%s\n",cfg->idx,(long) cfg->k);

int configuration_read(const char *file);

#endif

