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
#ifndef __FRITZBOXFON_SIMULATE_H__
#define __FRITZBOXFON_SIMULATE_H__	1

#include "../listen2fritz.h"
#include "../util/util.h"

/*
**	default connection values
*/
#define FRITZBOXFON_SIMULATE_DEFAULT_HOST	"localhost"
#define FRITZBOXFON_SIMULATE_DEFAULT_PORT	2012

/*
**	prototypes
*/
int l2f_simulate_fritzboxfon_init(void);
int l2f_simulate_fritzboxfon_main(void);
int l2f_simulate_fritzboxfon_exit(pid_t pid);

#endif

/**/
