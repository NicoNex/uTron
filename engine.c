/**
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


#define API_URL "https://api.telegram.org/"
#define API_REQUEST_LEN 4096


struct json_object* _send_message(struct engine_t *this, char *text, int64_t chat_id) {
	struct json_object *response;
	char *url = malloc(API_REQUEST_LEN);

	snprintf(url, API_REQUEST_LEN, "%ssendMessage?text=%s&chat_id=%ld&parse_mode=markdown", this->base_url, text, chat_id);
	struct memory_buffer_t mb = send_get_request(url);
	free(url);

	response = json_tokener_parse(mb.memory);
	free(mb.memory);

	return response;
}


struct json_object* _get_updates(struct engine_t *this, int timeout, int offset) {
	struct json_object *response;
	char *url = malloc(API_REQUEST_LEN);

	snprintf(url, API_REQUEST_LEN, "%sgetUpdates?timeout=%d", this->base_url, timeout);

	if (offset > 0)
		snprintf(url, API_REQUEST_LEN, "%s&offset=%d", url, offset);

	struct memory_buffer_t mb = send_get_request(url);
	free(url);

	response = json_tokener_parse(mb.memory);
	free(mb.memory);

	return response;
}


struct engine_t new_engine(const char *token) {
	struct engine_t engine;
	size_t base_url_size;

	engine.token = token;
	base_url_size = sizeof(API_URL) + strlen(token) + 4;
	engine.base_url = malloc(base_url_size);
	snprintf(engine.base_url, base_url_size, "%sbot%s/", API_URL, token);

	engine.send_message = _send_message;
	engine.get_updates = _get_updates;

	return engine;
}
