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
#include <pthread.h>
#include "engine.h"
#include "../bot.h"
#include "dispatcher.h"


struct session_t {
	int64_t chat_id;
	struct bot_t* bot;
	struct session_t* next;
};


volatile struct session_t sessions;


_Bool is_response_ok(const struct json_object *response) {
	struct json_object *ok;

	if (!json_object_object_get_ex(response, "ok", &ok)
			&& json_object_get_type(ok) != json_type_boolean)
		
		return 0;

	return json_object_get_boolean(ok);
}


struct session_t *get_session(int64_t chat_id) {
	puts("sus");
	struct session_t *tmp_session = &sessions;

	while (tmp_session != NULL) {
		puts("seees");
		printf("%ld %ld\n", chat_id, tmp_session->chat_id);
		if (tmp_session->chat_id == chat_id)
			return tmp_session;

		printf("%ld\n", tmp_session->chat_id);

		tmp_session = tmp_session->next;
	}

	return NULL;
}


void append_new_session(int64_t chat_id, struct bot_t *bot) {
	puts("fif");
	struct session_t *tmp_session = &sessions;

	while (tmp_session != NULL)
		tmp_session = tmp_session->next;

	tmp_session->next = malloc(sizeof(struct session_t));
	tmp_session->next->chat_id = chat_id;
	tmp_session->next->bot = bot;
	tmp_session->next->next = NULL;

	// tmp_session->next = &{
	// 	.chat_id = chat_id,
	// 	.bot = bot,
	// 	.next = NULL
	// };
}


void run_dispatcher() {
	_Bool is_first_run = 0;
	
	int last_update_id = 0;
	int updates_length;
	
	struct json_object *response;
	struct json_object *updates;
	struct engine_t engine = new_engine(TOKEN);
	struct session_t *session_tmp = malloc(sizeof(struct session_t));

	session_tmp->chat_id = 0;
	session_tmp->bot = NULL;
	session_tmp->next = NULL;

	// sessions = &{
	// 	.chat_id = 0,
	// 	.bot = NULL,
	// 	.next = NULL
	// };

	for (;;) {

		if (!is_first_run)
			response = engine.get_updates(&engine, 120, last_update_id+1);
		else
			response = engine.get_updates(&engine, 0, 0);

		if (is_response_ok(response)) {

			if (!json_object_object_get_ex(response, "result", &updates))
				continue;

			updates_length = json_object_array_length(updates);

			for (int i = 0; i < updates_length; i++) {
				int64_t current_chat_id;
				struct json_object *update, *message, *chat, *chat_id, *update_id;

				update = json_object_array_get_idx(updates, i);

				if (!json_object_object_get_ex(update, "message", &message)
						&& !json_object_object_get_ex(message, "chat", &chat)
						&& !json_object_object_get_ex(chat, "id", &chat_id))

					continue;

				// need to understand why json_object_get_int64(chat_id) is returning 0
				current_chat_id = json_object_get_int64(chat_id);

				session_tmp = get_session(current_chat_id);
				if (session_tmp) {
					puts("sas");
					// launch this in another thread
					session_tmp->bot->update(session_tmp->bot, &update);
					continue;
				}

				else {
					puts("fif");
					// add code to create a new session here
					struct bot_t bot_tmp = new_bot(current_chat_id);
					append_new_session(current_chat_id, &bot_tmp);
				}
			}
			is_first_run = 0;
		}
	}
}
