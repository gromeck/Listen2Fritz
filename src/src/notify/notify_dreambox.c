/*
**	notify_dreambox.c
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
#include "notify_dreambox.h"
#include "../phonenumber/phonenumber.h"
#if defined(HAVE_LIBCURL)
#include <curl/curl.h>
#endif

static CFG_NOTIFY_DREAMBOX_T *_cfg_notify_dreambox = NULL;

/*
**	configure this module
*/
int l2f_NOTIFY_DREAMBOX_configure(const int idx,const char *key,const char *value)
{
#if defined(HAVE_L2F_NOTIFY_DREAMBOX)
	CFG_NOTIFY_DREAMBOX_T *cfg;

	CFG_FIND_OR_CREATE(notify_dreambox);

	CFG_SET_KEY_VALUE_STRING(notify_dreambox,url)
	else CFG_SET_KEY_VALUE_STRING(notify_dreambox,text)
	else CFG_SET_KEY_VALUE_STRING(notify_dreambox,caller_number)
	else CFG_SET_KEY_VALUE_STRING(notify_dreambox,called_number)
	else return 0;
#else
	logmsg(LOG_CRIT,"no notification support via dreambox enabled -- skipped\n");
#endif
	return 1;
}

/*
**	dump configuration
*/
void l2f_NOTIFY_DREAMBOX_configdump(void)
{
	CFG_NOTIFY_DREAMBOX_T *cfg;

	CFG_LOOP(notify_dreambox) {
		CFG_DUMP_KEY_VALUE_STRING(notify_dreambox,url);
		CFG_DUMP_KEY_VALUE_STRING(notify_dreambox,text);
		CFG_DUMP_KEY_VALUE_STRING(notify_dreambox,caller_number);
		CFG_DUMP_KEY_VALUE_STRING(notify_dreambox,called_number);
	}
}

/*
**	process this module
*/
int l2f_NOTIFY_DREAMBOX_process(const L2F_EVENT_T *l2f_event)
{
#if defined(HAVE_L2F_NOTIFY_DREAMBOX)
	CURL *curl;
	char buffer[BUFSIZ],*escaped_buffer = NULL;
	char url[BUFSIZ];
	CFG_NOTIFY_DREAMBOX_T *cfg;

	if (!CFG_1ST_RECORD(notify_dreambox)) {
		logmsg(LOG_DEBUG,"notification via dreambox not configured -- skipping\n");
		return 0;
	}
	logmsg(LOG_DEBUG,"notification via dreambox ...\n");

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
	CFG_LOOP(notify_dreambox) {
		logmsg(LOG_DEBUG,"[%d] url=%s\n",cfg->idx,cfg->url);

		/*
		**	check if this module is configured
		*/
		if (!cfg->url || !cfg->text) {
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

			strcpy(buffer,l2f_EVENT_format(cfg->text,l2f_event));
#if 0
			escaped_buffer = curl_easy_escape(curl,buffer,0);
#else
			escaped_buffer = curl_escape(buffer,0);
#endif
			sprintf(url,cfg->url,escaped_buffer);
			curl_free(escaped_buffer);
			if ((curl_errno = curl_easy_setopt(curl,CURLOPT_URL,url))) {
				logmsg(LOG_CRIT,"[%d] cur_easy_setopt() failed: %d %s\n",cfg->idx,curl_errno,curl_share_strerror(curl_errno));
			}
			if ((curl_errno = curl_easy_setopt(curl,CURLOPT_TIMEOUT,2))) {
				logmsg(LOG_CRIT,"[%d] cur_easy_setopt() failed: %d %s\n",cfg->idx,curl_errno,curl_share_strerror(curl_errno));
			}
			if ((curl_errno = curl_easy_perform(curl))) {
				logmsg(LOG_CRIT,"[%d] curl_easy_perform() failed: %d %s\n",cfg->idx,curl_errno,curl_share_strerror(curl_errno));
			}
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
