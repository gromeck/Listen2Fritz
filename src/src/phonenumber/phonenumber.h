/*
**	phonenumber.h
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
#ifndef __PHONENUMBER_H__
#define __PHONENUMBER_H__	1

#include "../listen2fritz.h"
#include "../util/util.h"

/*
**	configuration structure
*/
typedef struct _cfg_phonenumber_t {
	CFG_COMMON_PART(phonenumber);
	char *prefix_city;	// city prefix
	char *prefix_state;	// state prefix
	char *nonumber;		// default for empty numbers
} CFG_PHONENUMBER_T;

/*
**	configuration parameters
*/
#define L2F_CFG_PHONENUMBER_PREFIX_CITY		(_cfg_phonenumber.prefix_city)
#define L2F_CFG_PHONENUMBER_PREFIX_STATE	(_cfg_phonenumber.prefix_state)
#define L2F_CFG_PHONENUMBER_NONUMBER		(_cfg_phonenumber.nonumber)

/*
**	prototypes
*/
char *l2f_PHONENUMBER_normalize(const char *number);
int l2f_PHONENUMBER_match(const char *numberA,const char *numberB);
const char *l2f_PHONENUMBER_nonumber(void);
int l2f_PHONENUMBER_configure(const char *section,const int idx,const char *key,const char *value);
void l2f_PHONENUMBER_configdump(void);

#endif

/**/
