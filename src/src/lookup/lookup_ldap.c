/*
**	lookup_ldap.c
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
#include "lookup_ldap.h"
#include "../phonenumber/phonenumber.h"
#include <string.h>

static CFG_LOOKUP_LDAP_T *_cfg_lookup_ldap = NULL;

/*
**	configure this module
*/
int l2f_LOOKUP_LDAP_configure(const int idx,const char *key,const char *value)
{
#if defined(HAVE_L2F_LOOKUP_LDAP)
	CFG_LOOKUP_LDAP_T *cfg;

	CFG_FIND_OR_CREATE(lookup_ldap);

	CFG_SET_KEY_VALUE_STRING(lookup_ldap,host)
	else CFG_SET_KEY_VALUE_INTEGER(lookup_ldap,port)
	else CFG_SET_KEY_VALUE_STRING(lookup_ldap,basedn)
	else return 0;
#else
	logmsg(LOG_CRIT,"no ldap support enabled -- skipped\n");
#endif
	return 1;
}

/*
**	dump the configuration
*/
void l2f_LOOKUP_LDAP_configdump(void)
{
	CFG_LOOKUP_LDAP_T *cfg;

	CFG_LOOP(lookup_ldap) {
		CFG_DUMP_KEY_VALUE_STRING(lookup_ldap,host);
		CFG_DUMP_KEY_VALUE_INTEGER(lookup_ldap,port);
		CFG_DUMP_KEY_VALUE_STRING(lookup_ldap,basedn);
	}
}

/*
**	process this module
*/
const char *l2f_LOOKUP_LDAP_process(const char *number)
{
#if defined(HAVE_L2F_LOOKUP_LDAP)
	CFG_LOOKUP_LDAP_T *cfg;
	LDAP *ld;
	LDAPMessage *result;
	LDAPMessage *entry;
	struct berval cred;
	int err;
	struct timeval timeout = { 3, 0};
	char *attrs[] = { "cn", "telephoneNumber", "mobile", "homePhone", NULL };
	int n,matchdigits;
	char *normnumber = l2f_PHONENUMBER_normalize(number);
#define FILTER_MAX_DIGITS 4
	char filterpattern[FILTER_MAX_DIGITS * 2 + 2 + 1];
	char filter[BUFSIZ];
	char ldap_url[BUFSIZ];
	static char *name = NULL;

	if (!CFG_1ST_RECORD(lookup_ldap)) {
		logmsg(LOG_DEBUG,"lookup via LDAP not configured -- skipping\n");
		return NULL;
	}
	logmsg(LOG_DEBUG,"looking up via LDAP ...\n");

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

	/*
	**	loop over the configuration records
	*/
	CFG_LOOP(lookup_ldap) {
		/*
		**	check if this module is configured
		*/
		if (!cfg->host ||
			!cfg->port ||
			!cfg->basedn) {
			logmsg(LOG_CRIT,"not configured -- skipped\n");
			return NULL;
		}

		/*
		**	connect the host
		*/
		sprintf(ldap_url,"ldap://%s:%d/",cfg->host,cfg->port);
		if (!(err = ldap_initialize(&ld,ldap_url)) < 0) {
			logmsg(LOG_CRIT,"ldap_initialize(%s) failed: %d (%s)\n",ldap_url,errno,strerror(errno));
			return NULL;
		}

		/*
		**	do a simple bind
		*/
		cred.bv_val = "";
		cred.bv_len = 0;
		if ((err = ldap_sasl_bind_s(ld,"",LDAP_SASL_SIMPLE,&cred,NULL,NULL,NULL)) < 0) {
			logmsg(LOG_CRIT,"ldap_simple_bind_s() failed: %d (%s)\n",err,ldap_err2string(err));
			ldap_unbind_ext_s(ld,NULL,NULL);
			return NULL;
		}

		/*
		**	setup the filter pattern
		**
		**	We will filter for numbers which contain "*A*B*C*D*"
		**	where ABCD are the last 4 digits of the requested
		**	phone number. This should reduce the result dramatically.
		*/
		strcpy(filterpattern,"*");
		matchdigits = (strlen(normnumber) > FILTER_MAX_DIGITS) ? FILTER_MAX_DIGITS : strlen(normnumber);
		for (n = strlen(normnumber) - matchdigits;normnumber[n];n++)
			sprintf(&filterpattern[strlen(filterpattern)],"%c*",normnumber[n]);

		/*
		**	setup the filter
		**
		**	NOTE: we assume that the attributes in attrs[1..] are
		**	      phone numbers
		*/
		strcpy(filter,"(|");
		for (n = 1;attrs[n];n++)
			sprintf(&filter[strlen(filter)],"(%s=%s)",attrs[n],filterpattern);
		strcat(filter,")");

		/*
		**	do the search
		*/
		logmsg(LOG_DEBUG,"doing ldap lookup with filter '%s' ...\n",filter);
		if ((err = ldap_search_ext_s(ld,cfg->basedn,LDAP_SCOPE_ONELEVEL,
				filter,attrs,0,NULL,NULL,&timeout,0,&result)) < 0) {
			logmsg(LOG_CRIT,"ldap_simple_bind_s() failed: %d (%s)\n",err,ldap_err2string(err));
			ldap_unbind_ext_s(ld,NULL,NULL);
			return NULL;
		}

		/*
		**	loop over the result
		*/
		if ((entry = ldap_first_entry(ld,result))) {
			do {
				/*
				**	lookup the cn
				*/
				struct berval **cn;
				struct berval **phone;
				
				if ((cn = ldap_get_values_len(ld,entry,"cn"))) {
					/*
					**	we got a CN, so scan the values of the requested
					**	attributes
					**
					**	NOTE: we assume that the attributes in attrs[1..] are
					**	      phone numbers
					*/
					int n;

					logmsg(LOG_DEBUG,"retrieving numbers for %s\n",cn[0]->bv_val);
					for (n = 1;!name[0] && attrs[n];n++) {
						if ((phone = ldap_get_values_len(ld,entry,attrs[n]))) {
							/*
							**	check if the number matches
							*/
							logmsg(LOG_DEBUG,"%s has %s %s\n",cn[0]->bv_val,attrs[n],phone[0]->bv_val);
							if (phone[0] && l2f_PHONENUMBER_match(number,phone[0]->bv_val)) {
								/*
								**	phone number matches
								*/
								strcpy(name,cn[0]->bv_val);
								logmsg(LOG_DEBUG,"%s has %s %s which matches %s!\n",cn[0]->bv_val,attrs[n],phone[0]->bv_val,number);
							}
							ldap_value_free_len(phone);
						}
					}
					ldap_value_free_len(cn);
				}
			} while (!*name && (entry = ldap_next_entry(ld,entry)));
		}

		/*
		**	close the connection again
		*/
		ldap_msgfree(result);
		ldap_unbind_ext_s(ld,NULL,NULL);

		logmsg(LOG_DEBUG,"looked up: %s=%s\n",normnumber,(*name) ? name : "NULL");
		if (*name)
			break;
	}
	logmsg(LOG_DEBUG,"complete\n");
	return (*name) ? name : NULL;
#else
	return NULL;
#endif
}/**/
