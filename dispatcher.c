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
	time_t timestamp;
	struct bot_t* bot;
	struct session_t* next;
};


volatile struct session_t sessions = {.chat_id = 0, .bot = NULL, .next = NULL};


_Bool is_response_ok(const struct json_object *response) {
	struct json_object *ok;

	if (!json_object_object_get_ex(response, "ok", &ok)
			&& json_object_get_type(ok) != json_type_boolean)
		
		return 0;

	return json_object_get_boolean(ok);
}

// deprecated
uint32_t append_new_session(int64_t chat_id) {
	volatile struct session_t *tmp_session = &sessions;

	uint32_t length;
	for (length = 0; tmp_session->next != NULL; length++)
		tmp_session = tmp_session->next;

	
	tmp_session->next = malloc(sizeof(struct session_t));

	if (tmp_session->next == NULL) {
		fputs("append_new_session: not enough memory (malloc returned NULL)\n", stderr);
		return 0;
	}

	tmp_session->next->chat_id = chat_id;
	tmp_session->next->bot = new_bot(chat_id);
	tmp_session->next->next = NULL;

	return ++length;
}


void populate_session(volatile struct session_t *session_ptr, int64_t chat_id) {
	if (session_ptr != NULL) {
		session_ptr->chat_id = chat_id;
		session_ptr->bot = new_bot(chat_id);
		session_ptr->next = NULL;
	}

	else
		fputs("populate_session: not enough memory (malloc returned NULL)\n", stderr);
}


volatile struct session_t *get_session_ptr(int64_t chat_id) {
	volatile struct session_t *session_ptr = &sessions;
	int i = 0;

	while (session_ptr->chat_id != chat_id) {
		if (session_ptr->next != NULL)
			session_ptr = session_ptr->next;

		else {
			if (i > 0) {
				session_ptr->next = malloc(sizeof(struct session_t));
				populate_session(session_ptr->next, chat_id);
				return session_ptr->next;
			}

			else {
				populate_session(session_ptr, chat_id);
				return session_ptr;
			}
		}
		i++;
	}

	return session_ptr;
}


void run_dispatcher() {
	_Bool is_first_run = 0;
	
	int last_update_id = 0;
	int updates_length;
	
	struct json_object *response;
	struct json_object *updates;
	struct engine_t engine;

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

				volatile struct session_t *session_ptr = get_session_ptr(current_chat_id);
				if (session_ptr && !is_first_run) {
					session_ptr->timestamp = time(NULL);
					// session_ptr->bot->update(session_ptr->bot, update);

					struct update_arg_t uarg = {
						.bot = session_ptr->bot,
						.update = update
					};

					printf("%lu\n", &uarg);

					pthread_t thread_id;
					pthread_create(&thread_id, NULL, update_bot, (void *)&uarg);
					pthread_detach(thread_id);
				}
			}
			is_first_run = 0;
		}
	}
}
