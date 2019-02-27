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

#include "engine.h"
#include "network.h"


#define API_URL "https://api.telegram.org/"
#define API_REQUEST_LEN 4096


static char *base_url;


struct json_object *tg_send_message(char *text, int64_t chat_id) {
	struct json_object *response;
	char *url = malloc(API_REQUEST_LEN);

	snprintf(url, API_REQUEST_LEN, "%ssendMessage?text=%s&chat_id=%ld&parse_mode=markdown", base_url, text, chat_id);
	struct memory_buffer mb = send_get_request(url);
	free(url);

	response = json_tokener_parse(mb.memory);
	free(mb.memory);

	return response;
}


struct json_object *tg_get_updates(int timeout, int offset) {
	struct json_object *response;
	char *url = malloc(API_REQUEST_LEN);

	snprintf(url, API_REQUEST_LEN, "%sgetUpdates?timeout=%d", base_url, timeout);

	if (offset > 0) {
		char *str_off = malloc(32);
		snprintf(str_off, 32, "&offset=%d", offset);
		strcat(url, str_off);
		free(str_off);
	}

	struct memory_buffer mb = send_get_request(url);
	free(url);

	response = json_tokener_parse(mb.memory);
	free(mb.memory);

	return response;
}


void init_engine(const char *token) {
	size_t base_url_size;

	base_url_size = strlen(API_URL) + strlen(token) + 5;
	base_url = malloc(base_url_size);
	snprintf(base_url, base_url_size, "%sbot%s/", API_URL, token);
}
