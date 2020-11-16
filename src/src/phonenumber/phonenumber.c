/*
**	phonenumber.c
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
#include "phonenumber.h"
#include <string.h>
#include <ctype.h>

static CFG_PHONENUMBER_T _cfg_phonenumber;

/*
**	normalize a phone number
*/
char *l2f_PHONENUMBER_normalize(const char *number)
{
#define ROTATION	10
	static char _buffer[ROTATION][100];
	static int rotation = 0;
	char *buffer = _buffer[rotation++ % ROTATION];
#undef ROTATION
	char buffhelp[BUFSIZ];
	const char *s;
	char *t;

	/*
	**	copy only the preceeding + and digits
	*/
	for (s = number,t = buffer;*s;s++)
		if ((s == number && *s == '+') || isdigit(*s))
			*t++ = *s;
	*t = '\0';

	if (buffer[0] && buffer[0] != '+') {
		/*
		**	add my city prefix if the number doesn't start with '0'
		*/
		if (buffer[0] != '0' && L2F_CFG_PHONENUMBER_PREFIX_CITY && *L2F_CFG_PHONENUMBER_PREFIX_CITY) {
			strcpy(buffhelp,L2F_CFG_PHONENUMBER_PREFIX_CITY);
			strcat(buffhelp,buffer);
			strcpy(buffer,buffhelp);
		}

		/*
		**	replace the leading '0' by the state prefix
		*/
		if (buffer[1] == '0') {	
			/*
			**	looks like an international number starting with a 00
			*/
			buffhelp[0] = '+';
			strcpy(&buffhelp[1],&buffer[2]);
			strcpy(buffer,buffhelp);
		}
		else if (L2F_CFG_PHONENUMBER_PREFIX_STATE && *L2F_CFG_PHONENUMBER_PREFIX_STATE) {
			/*
			**	national number
			*/
			strcpy(buffhelp,L2F_CFG_PHONENUMBER_PREFIX_STATE);
			strcat(buffhelp,&buffer[1]);
			strcpy(buffer,buffhelp);
		}
	}

	return buffer;
}

/*
**	match two phonenumbers
*/
int l2f_PHONENUMBER_match(const char *numberA,const char *numberB)
{
	return !strcmp(l2f_PHONENUMBER_normalize(numberA),l2f_PHONENUMBER_normalize(numberB));
}

/*
**	return the default for unknown numbers
*/
const char *l2f_PHONENUMBER_nonumber(void)
{
	return L2F_CFG_PHONENUMBER_NONUMBER;
}

/*
**	configure this module
*/
int l2f_PHONENUMBER_configure(const char *section,int idx,const char *key,const char *value)
{
	CFG_PHONENUMBER_T *cfg = &_cfg_phonenumber;

	if (!strcmp(section,"phonenumber")) {
		CFG_SET_KEY_VALUE_STRING(phonenumber,prefix_city)
		else CFG_SET_KEY_VALUE_STRING(phonenumber,prefix_state)
		else CFG_SET_KEY_VALUE_STRING(phonenumber,nonumber)
		else return 0;
		return 1;
	}
	return 0;
}


/*
**	dump the configuration
*/
void l2f_PHONENUMBER_configdump(void)
{
	CFG_PHONENUMBER_T *cfg = &_cfg_phonenumber;

	CFG_DUMP_KEY_VALUE_STRING(phonenumber,prefix_city);
	CFG_DUMP_KEY_VALUE_STRING(phonenumber,prefix_state);
	CFG_DUMP_KEY_VALUE_STRING(phonenumber,nonumber);
}/**/
