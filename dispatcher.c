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


struct session {
	int64_t chat_id;
	time_t timestamp;
	struct bot* bot_ptr;
	struct session* next;
};


volatile struct session session_list = {.chat_id = 0, .bot_ptr = NULL, .next = NULL};


_Bool is_response_ok(const struct json_object *response) {
	struct json_object *ok;

	if (!json_object_object_get_ex(response, "ok", &ok)
			&& json_object_get_type(ok) != json_type_boolean)
		
		return 0;

	return json_object_get_boolean(ok);
}

// deprecated
uint32_t append_new_session(int64_t chat_id) {
	volatile struct session *tmp_session = &session_list;

	uint32_t length;
	for (length = 0; tmp_session->next != NULL; length++)
		tmp_session = tmp_session->next;

	
	tmp_session->next = malloc(sizeof(struct session));

	if (tmp_session->next == NULL) {
		fputs("append_new_session: not enough memory (malloc returned NULL)\n", stderr);
		return 0;
	}

	tmp_session->next->chat_id = chat_id;
	tmp_session->next->bot_ptr = new_bot(chat_id);
	tmp_session->next->next = NULL;

	return ++length;
}


void populate_session(volatile struct session *session_ptr, int64_t chat_id) {
	if (session_ptr != NULL) {
		session_ptr->chat_id = chat_id;
		session_ptr->bot_ptr = new_bot(chat_id);
		session_ptr->next = NULL;
	}

	else
		fputs("populate_session: not enough memory (malloc returned NULL)\n", stderr);
}


volatile struct session *get_session_ptr(int64_t chat_id) {
	volatile struct session *session_ptr = &session_list;
	int i = 0;

	while (session_ptr->chat_id != chat_id) {
		if (session_ptr->next != NULL)
			session_ptr = session_ptr->next;

		else {
			if (i > 0) {
				session_ptr->next = malloc(sizeof(struct session));
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

	init_engine(TOKEN);

	for (;;) {

		if (!is_first_run)
			response = tg_get_updates(20, last_update_id+1);
		else
			response = tg_get_updates(0, 0);

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

				volatile struct session *session_ptr = get_session_ptr(current_chat_id);
				if (session_ptr && !is_first_run) {
					session_ptr->timestamp = time(NULL);
					// session_ptr->bot_ptr->update(session_ptr->bot_ptr, update);

					struct update_arg uarg = {
						.bot_ptr = session_ptr->bot_ptr,
						.update = update
					};

					pthread_t thread_id;
					pthread_create(&thread_id, NULL, update_bot, (void *)&uarg);
					pthread_detach(thread_id);
				}
			}
			is_first_run = 0;
		}
	}
}
