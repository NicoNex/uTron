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


#include <time.h>
#include <pthread.h>
#include "engine.h"
#include "bot.h"
#include "dispatcher.h"


#define CACHE_LEN 20


struct session {
	int64_t chat_id;
	time_t timestamp;
	struct bot* bot_ptr;
	struct session* next;
};


struct session session_list = {.chat_id = 0, .bot_ptr = NULL, .next = NULL};
struct session *session_cache[CACHE_LEN] = {NULL};


int hash_code(int64_t chat_id) {
	return chat_id % CACHE_LEN;
}


void update_cache(int64_t chat_id, struct session *session_ptr) {
	session_cache[hash_code(chat_id)] = session_ptr;
}


_Bool is_response_ok(const struct json_object *response) {
	struct json_object *ok;

	if (!json_object_object_get_ex(response, "ok", &ok)
			&& json_object_get_type(ok) != json_type_boolean)
		
		return 0;

	return json_object_get_boolean(ok);
}


int populate_session(struct session *session_ptr, int64_t chat_id) {
	if (session_ptr != NULL) {
		session_ptr->chat_id = chat_id;
		session_ptr->bot_ptr = new_bot(chat_id);
		session_ptr->next = NULL;
		return 0;
	}

	else {
		fputs("populate_session: not enough memory (malloc returned NULL)\n", stderr);
		return 1;
	}
}


struct session *get_session_ptr(int64_t chat_id) {
	struct session *session_ptr = session_cache[hash_code(chat_id)];

	if (session_ptr != NULL && session_ptr->chat_id == chat_id)
		return session_ptr;

	session_ptr = &session_list;
	
	for (int i = 0; session_ptr->chat_id != chat_id; i++) {
		if (session_ptr->next != NULL)
			session_ptr = session_ptr->next;

		else {
			if (i > 0) {
				session_ptr->next = malloc(sizeof(struct session));
				populate_session(session_ptr->next, chat_id);
				session_ptr = session_ptr->next;
			}

			else
				populate_session(session_ptr, chat_id);

			break;
		}
	}

	update_cache(chat_id, session_ptr);
	return session_ptr;
}


void run_dispatcher(const char *token) {
	_Bool is_first_run = 0;
	
	int last_update_id = 0;
	int updates_length;
	
	struct json_object *response;
	struct json_object *updates;

	init_engine(token);

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
					continue;

				if (!json_object_object_get_ex(message, "chat", &chat))
					continue;

				if (!json_object_object_get_ex(chat, "id", &chat_id))
					continue;

				if (!json_object_object_get_ex(update, "update_id", &update_id))
					continue;

				last_update_id = json_object_get_int(update_id);
				current_chat_id = json_object_get_int64(chat_id);

				struct session *session_ptr = get_session_ptr(current_chat_id);
				if (session_ptr && !is_first_run) {
					session_ptr->timestamp = time(NULL);

					struct bot_update_arg uarg = {
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
