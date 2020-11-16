/*
**	fritzboxfon.h
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
#ifndef __FRITZBOXFON_H__
#define __FRITZBOXFON_H__	1

#include "../listen2fritz.h"
#include "../util/util.h"
#include "../phonenumber/phonenumber.h"

/*
**	default connection values
*/
#define FRITZBOXFON_DEFAULT_HOST	"fritzbox"
#define FRITZBOXFON_DEFAULT_PORT	1012

/*
**	configuration structure
*/
typedef struct _fritzboxfon_cfg {
	CFG_COMMON_PART(fritzbox);
	char *host;			// DNS or IP address of the Fritz!Box
	int port;			// port to connect to
} CFG_FRITZBOXFON_T;

/*
**	configuration parameters
*/
#define L2F_CFG_FRITZBOXFON_HOST			(cfg->host)
#define L2F_CFG_FRITZBOXFON_PORT			(cfg->port)

/*
**	prototypes
*/
void l2f_FRITZBOXFON_close(void);
int l2f_FRITZBOXFON_configure(const char *section,const int idx,const char *key,const char *value);
void l2f_FRITZBOXFON_configdump(void);
int l2f_FRITZBOXFON_process(L2F_CALLBACK_T *callbacks[]);

#endif

/**/
