/*
**	lookup_script.h
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
#ifndef __LOOKUP_SCRIPT_H__
#define __LOOKUP_SCRIPT_H__	1

#include "lookup.h"

/*
**	configuration structure
*/
typedef struct _cfg_lookup_script_t {
	CFG_COMMON_PART(lookup_script);
	char *script;		// full path of the script to call
} CFG_LOOKUP_SCRIPT_T;

/*
**	configuration defaults
*/

/*
**	prototypes
*/
int l2f_LOOKUP_SCRIPT_configure(const int idx,const char *key,const char *value);
void l2f_LOOKUP_SCRIPT_configdump(void);
const char *l2f_LOOKUP_SCRIPT_process(const char *number);

#endif

/**/
