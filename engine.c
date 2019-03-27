/*
 * uTron
 * Copyright (C) 2019  Nicolò Santamaria
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdarg.h>
#include "engine.h"
#include "network.h"


#define API_URL "https://api.telegram.org/"
#define API_REQUEST_LEN 4096


enum request_type {
	GET, 
	POST
};


static char *base_url;


void init_engine(const char *token) {
	size_t base_url_size;

	base_url_size = strlen(API_URL) + strlen(token) + 5;
	base_url = malloc(base_url_size);
	snprintf(base_url, base_url_size, "%sbot%s/", API_URL, token);
}


struct json_object *make_request(const char *url, int type, ...) {
	struct json_object *response;
	struct memory_buffer mb;

	switch (type) {
		case GET:
			mb = send_get_request(url);
			break;

		case POST: {
			char *argtmp[2];

			va_list argptr;
			va_start(argptr, type);

			for (int i = 0; i < 2; i++)
				argtmp[i] = va_arg(argptr, char *);

			va_end(argptr);
			mb = send_post_request(url, argtmp[0], argtmp[1]);
			break;
		}
	}

	response = json_tokener_parse(mb.memory);
	free(mb.memory);

	return response;
}


struct json_object *tg_get_updates(int timeout, int offset) {
	struct json_object *response;
	char url[API_REQUEST_LEN];

	snprintf(url, API_REQUEST_LEN, "%sgetUpdates?timeout=%d", base_url, timeout);

	if (offset > 0) {
		char str_off[32];
		snprintf(str_off, 32, "&offset=%d", offset);
		strcat(url, str_off);
	}

	response = make_request(url, GET);

	return response;
}


struct json_object *tg_send_message(char *text, int64_t chat_id) {
	struct json_object *response;
	char url[API_REQUEST_LEN];

	snprintf(url, API_REQUEST_LEN, "%ssendMessage?text=%s&chat_id=%ld&parse_mode=markdown", base_url, text, chat_id);
	response = make_request(url, GET);

	return response;
}


struct json_object *tg_send_document(char *filepath, char *caption, int64_t chat_id) {
	struct json_object *response;
	char url[API_REQUEST_LEN];
	
	snprintf(url, API_REQUEST_LEN, "%ssendDocument?chat_id=%ld&caption=%s&parse_mode=markdown", base_url, chat_id, caption);
	response = make_request(url, POST, filepath, "document");

	return response;
}
