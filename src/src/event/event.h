/*
**	listen2fritz.h
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
#ifndef __EVENT_H__
#define __EVENT_H__	1

#include "../listen2fritz.h"
#include "../util/util.h"

char *l2f_EVENT_type2string(const int type);
void l2f_EVENT_dump(const char *title,const L2F_EVENT_T *l2fevent);
char *l2f_EVENT_format(const char *format,const L2F_EVENT_T *l2fevent);
int l2f_EVENT_configure(const char *section,const int idx,const char *key,const char *value);
void l2f_EVENT_configdump(void);

#endif

/**/
