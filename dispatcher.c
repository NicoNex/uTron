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


#include <time.h>
#include "dispatcher.h"
#include "../bot.h"


struct session_t {
	int64_t chat_id;
	struct bot_t* bot;
	struct session_t* next;
};


volatile struct session_t sessions;


_Bool is_response_ok(const struct json_object *response) {
	struct json_object ok;

	if (!json_object_object_get_ex(response, "ok", ok))
		return 0;

	if (json_object_get_type(ok) != json_type_boolean)
		return 0;

	return json_object_get_boolean(ok);
}


_Bool get_session(int64_t chat_id, session_t *session) {
	struct session_t *tmp_session = &sessions;

	while (tmp_session != NULL) {
		if (tmp_session->chat_id == chat_id) {
			session = &tmp_session;
			return 1;
		}

		else {
			tmp_session = tmp_session->next;
			continue;
		}
	}

	return 0;
}


void append_new_session (int64_t chat_id, bot_t *bot) {
	struct session_t *tmp_session = &sessions;

	while (tmp_session != NULL)
		tmp_session = tmp_session->next;

	tmp_session->next = malloc(sizeof(session_t));

	tmp_session->next = {
		.chat_id = chat_id,
		.bot = bot,
		.next = NULL
	};
}


void run_dispatcher() {
	_Bool is_first_run = 0;
	
	int last_update_id = 0;
	int updates_length;
	
	struct json_object *response;
	struct json_object updates;
	struct engine_t engine = new_engine(TOKEN);
	struct session_t *session_tmp;

	sessions = {
		.chat_id = 0,
		.bot = NULL,
		.next = NULL
	};

	for (;;) {

		if (!is_first_run)
			response = engine.get_updates(&engine, last_update_id+1, 120);
		else
			response = engine.get_updates(&engine, 0, 0);

		if (is_response_ok(response)) {

			if (!json_object_object_get_ex(response, "result", &updates))
				continue;

			updates_length = json_object_array_length(updates);

			for (int i = 0; i < updates_length; i++) {
				struct json_object *update, *chat_id, *update_id;

				update = json_object_array_get_idx(updates, i);

				if (!json_object_object_get_ex(update, "chat_id", &chat_id))
					continue;

				// check whether json_object_get_int(chat_id) is in the list `sessions` 

				if (get_session(json_object_get_int(chat_id), session_tmp)) {
					// launch this in another thread
					session_tmp->bot->update(update);
					continue;
				}

				else {
					// add code to create a new session here
				}
			}
		}
	}
}