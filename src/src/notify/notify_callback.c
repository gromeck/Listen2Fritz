/*
**	notify_callback.c
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
#include "notify.h"
#include "notify_callback.h"
#include "../phonenumber/phonenumber.h"
#if defined(HAVE_LIBCURL)
#include <curl/curl.h>
#endif

#define CALLBACK_URL	"http://%s/cgi-bin/webcm"
#define CALLBACK_POST_PASSWORD	"login:command/password=%s"
#define CALLBACK_POST_DIAL		"telcfg:command/Dial=%s%s&telcfg:settings/DialPort=%s"

static CFG_NOTIFY_CALLBACK_T *_cfg_notify_callback = NULL;

/*
**	configure this module
*/
int l2f_NOTIFY_CALLBACK_configure(const int idx,const char *key,const char *value)
{
#if defined(HAVE_L2F_NOTIFY_CALLBACK)
	CFG_NOTIFY_CALLBACK_T *cfg;

	CFG_FIND_OR_CREATE(notify_callback);

	CFG_SET_KEY_VALUE_STRING(notify_callback,host)
	else CFG_SET_KEY_VALUE_STRING(notify_callback,password)
	else CFG_SET_KEY_VALUE_STRING(notify_callback,caller_number)
	else CFG_SET_KEY_VALUE_STRING(notify_callback,called_number)
	else CFG_SET_KEY_VALUE_STRING(notify_callback,dial_prefix)
	else CFG_SET_KEY_VALUE_STRING(notify_callback,dial_port)
	else return 0;
#else
	logmsg(LOG_CRIT,"no notification support via callback enabled -- skipped\n");
#endif
	return 1;
}

/*
**	dump configuration
*/
void l2f_NOTIFY_CALLBACK_configdump(void)
{
	CFG_NOTIFY_CALLBACK_T *cfg;

	CFG_LOOP(notify_callback) {
		CFG_DUMP_KEY_VALUE_STRING(notify_callback,host);
		CFG_DUMP_KEY_VALUE_STRING(notify_callback,password);
		CFG_DUMP_KEY_VALUE_STRING(notify_callback,caller_number);
		CFG_DUMP_KEY_VALUE_STRING(notify_callback,called_number);
		CFG_DUMP_KEY_VALUE_STRING(notify_callback,dial_prefix);
		CFG_DUMP_KEY_VALUE_STRING(notify_callback,dial_port);
	}
}

/*
**	process this module
*/
int l2f_NOTIFY_CALLBACK_process(const L2F_EVENT_T *l2f_event)
{
#if defined(HAVE_L2F_NOTIFY_CALLBACK)
	CURL *curl;
	char url[BUFSIZ];
	char postdata[BUFSIZ];
	CFG_NOTIFY_CALLBACK_T *cfg;

	if (!CFG_1ST_RECORD(notify_callback)) {
		logmsg(LOG_DEBUG,"notification via callback not configured -- skipping\n");
		return 0;
	}
	logmsg(LOG_DEBUG,"notification via callback ...\n");

	/*
	**	filter only incoming rings
	*/
	if (l2f_event->type != L2F_EVENT_TYPE_RING) {
		logmsg(LOG_DEBUG,"event doesn't match module policy -- event ignoring\n");
		return 0;
	}

	/*
	**	loop over the configuration records
	*/
	CFG_LOOP(notify_callback) {
		logmsg(LOG_DEBUG,"[%d] host=%s\n",cfg->idx,cfg->host);

		/*
		**	check if this module is configured
		*/
		if (!cfg->host) {
			logmsg(LOG_CRIT,"[%d] not configured -- skipped\n",cfg->idx);
			continue;
		}

		/*
		**	check if the caller number matches
		*/
		if (cfg->caller_number && !l2f_PHONENUMBER_match(l2f_event->caller_number,cfg->caller_number)) {
			logmsg(LOG_CRIT,"[%d] configured caller_number (%s) doesn't match the caller number (%s) -- skipped\n",
					cfg->idx,cfg->caller_number,l2f_event->caller_number);
			continue;
		}

		/*
		**	check if the called number matches
		*/
		if (cfg->called_number && !l2f_PHONENUMBER_match(l2f_event->called_number,cfg->called_number)) {
			logmsg(LOG_CRIT,"[%d] configured called_number (%s) doesn't match the called number (%s) -- skipped\n",
					cfg->idx,cfg->called_number,l2f_event->called_number);
			continue;
		}

		/*
		**	do the request via curl
		*/
		if ((curl = curl_easy_init())) {
			/*
			**	setup the message to display on our dreambox
			*/
			CURLSHcode curl_errno;

			/*
			**	send the password
			*/
			if (cfg->password) {
				logmsg(LOG_DEBUG,"[%d] sending password to login\n",cfg->idx);
				sprintf(url,CALLBACK_URL,cfg->host);
				sprintf(postdata,CALLBACK_POST_PASSWORD,cfg->password);

				if ((curl_errno = curl_easy_setopt(curl,CURLOPT_URL,url))) {
					logmsg(LOG_CRIT,"[%d] cur_easy_setopt() failed: %d %s\n",
							cfg->idx,curl_errno,curl_share_strerror(curl_errno));
				}
				if ((curl_errno = curl_easy_setopt(curl,CURLOPT_POST,1))) {
					logmsg(LOG_CRIT,"[%d] cur_easy_setopt() failed: %d %s\n",
							cfg->idx,curl_errno,curl_share_strerror(curl_errno));
				}
				if ((curl_errno = curl_easy_setopt(curl,CURLOPT_POSTFIELDS,postdata))) {
					logmsg(LOG_CRIT,"[%d] cur_easy_setopt() failed: %d %s\n",
							cfg->idx,curl_errno,curl_share_strerror(curl_errno));
				}
				if ((curl_errno = curl_easy_perform(curl))) {
					logmsg(LOG_CRIT,"[%d] curl_easy_perform() failed: %d %s\n",
							cfg->idx,curl_errno,curl_share_strerror(curl_errno));
				}
			}

			/*
			**	send the dial request
			*/
			logmsg(LOG_DEBUG,"[%d] sending dial request: prefix+number=%s%s  dialport=%s\n",
					cfg->idx,
					(cfg->dial_prefix) ? cfg->dial_prefix : "",
					l2f_event->caller_number,
					cfg->dial_port);
			sprintf(url,CALLBACK_URL,cfg->host);
			sprintf(postdata,CALLBACK_POST_DIAL,
					(cfg->dial_prefix) ? cfg->dial_prefix : "",
					l2f_event->caller_number,
					cfg->dial_port);

			if ((curl_errno = curl_easy_setopt(curl,CURLOPT_URL,url))) {
				logmsg(LOG_CRIT,"[%d] cur_easy_setopt() failed: %d %s\n",
						cfg->idx,curl_errno,curl_share_strerror(curl_errno));
			}
			if ((curl_errno = curl_easy_setopt(curl,CURLOPT_POST,1))) {
				logmsg(LOG_CRIT,"[%d] cur_easy_setopt() failed: %d %s\n",
						cfg->idx,curl_errno,curl_share_strerror(curl_errno));
			}
			if ((curl_errno = curl_easy_setopt(curl,CURLOPT_POSTFIELDS,postdata))) {
				logmsg(LOG_CRIT,"[%d] cur_easy_setopt() failed: %d %s\n",
						cfg->idx,curl_errno,curl_share_strerror(curl_errno));
			}
			if ((curl_errno = curl_easy_perform(curl))) {
				logmsg(LOG_CRIT,"[%d] curl_easy_perform() failed: %d %s\n",
						cfg->idx,curl_errno,curl_share_strerror(curl_errno));
			}

			/*
			**	send the dial request
			*/
			curl_easy_cleanup(curl);
		}
		else {
			logmsg(LOG_CRIT,"[%d] cur_easy_init() failed\n",cfg->idx);
			continue;
		}
	}
	logmsg(LOG_DEBUG,"complete\n");
#endif
	return 0;
}/**/
