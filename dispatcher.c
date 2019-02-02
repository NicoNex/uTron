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


volatile struct session_t *get_session(int64_t chat_id) {
	volatile struct session_t *tmp_session = &sessions;

	while (tmp_session->chat_id != chat_id) {
		if (tmp_session->next != NULL)
			tmp_session = tmp_session->next;

		else
			return NULL;
	}

	return tmp_session;
}


uint32_t append_new_session(int64_t chat_id, struct bot_t *bot) {
	volatile struct session_t *tmp_session = &sessions;

	uint32_t length;
	for (length = 0; tmp_session->next != NULL; length++)
		tmp_session = tmp_session->next;

	
	tmp_session->next = malloc(sizeof(struct session_t));
	tmp_session->next->chat_id = chat_id;
	tmp_session->next->bot = bot;
	tmp_session->next->next = NULL;

	return ++length;
}


void run_dispatcher() {
	_Bool is_first_run = 0;
	
	int last_update_id = 0;
	int updates_length;
	
	struct json_object *response;
	struct json_object *updates;
	struct engine_t engine;

	sessions.chat_id = 0;
	sessions.bot = NULL;
	sessions.next = NULL;

	engine = new_engine(TOKEN);

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

				if (!json_object_object_get_ex(update, "message", &message))
					continue; // may generate unexpected behaviour

				if (!json_object_object_get_ex(message, "chat", &chat))
					continue; // may generate unexpected behaviour

				if (!json_object_object_get_ex(chat, "id", &chat_id))
					continue; // may generate unexpected behaviour

				if (!json_object_object_get_ex(update, "update_id", &update_id))
					continue; // may generate unexpected behaviour

				last_update_id = json_object_get_int(update_id);
				current_chat_id = json_object_get_int64(chat_id);

				volatile struct session_t *session_tmp = get_session(current_chat_id);
				if (session_tmp && !is_first_run) {
					session_tmp->bot->update(session_tmp->bot, update);

					// struct update_arg_t uarg = {
					// 	.bot = session_tmp->bot,
					// 	.update = update
					// };

					// printf("%lu\n", &uarg);

					// pthread_t thread_id;
					// pthread_create(&thread_id, NULL, update_bot, (void *)&uarg);
					// pthread_detach(thread_id);
				}

				else {
					if (is_first_run) {
						struct bot_t bot_tmp = new_bot(current_chat_id);
						
						sessions.chat_id = current_chat_id;
						sessions.bot = &bot_tmp;
						sessions.next = NULL;
					}

					else {
						struct bot_t bot_tmp = new_bot(current_chat_id);
						append_new_session(current_chat_id, &bot_tmp);
					}
				}
			}
			is_first_run = 0;
		}
	}
}
