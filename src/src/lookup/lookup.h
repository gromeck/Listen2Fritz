/*
**	lookup.h
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
#ifndef __LOOKUP_H__
#define __LOOKUP_H__	1

#include "../listen2fritz.h"
#include "../util/util.h"
#include "lookup_ldap.h"
#include "lookup_mysql.h"
#include "lookup_script.h"

int l2f_LOOKUP_configure(const char *section,const int idx,const char *key,const char *value);
void l2f_LOOKUP_configdump(void);
int l2f_LOOKUP_process(L2F_EVENT_T *l2f_event);

#endif

/**/
