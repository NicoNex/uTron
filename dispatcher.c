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
#include <unistd.h>
#include <pthread.h>

#include "bot.h"
#include "engine.h"
#include "dispatcher.h"


#define CACHE_LEN 20
#define HASH(id) id % CACHE_LEN


struct session {
	int64_t chat_id;
	time_t timestamp;
	struct bot *bot_ptr;

	struct session *next;
};


struct session *session_list = NULL;
struct session *session_cache[CACHE_LEN] = {NULL};
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


static void update_cache(int64_t chat_id, struct session *session_ptr) {
	session_cache[HASH(chat_id)] = session_ptr;
}


static _Bool is_response_ok(const struct json_object *response) {
	struct json_object *ok;

	if (!json_object_object_get_ex(response, "ok", &ok)
			&& json_object_get_type(ok) != json_type_boolean)
		
		return 0;

	return json_object_get_boolean(ok);
}


static void push_session(int64_t chat_id) {
	struct session *tmp = session_list; 
	session_list = malloc(sizeof(struct session));
	session_list->chat_id = chat_id;
	session_list->bot_ptr = new_bot(chat_id);
	session_list->next = tmp;
}


static struct session *get_session_ptr(const int64_t chat_id) {
	struct session *tmp = session_cache[HASH(chat_id)];

	if (tmp && tmp->chat_id == chat_id)
		return tmp;

	tmp = session_list;

	if (!tmp) {
		push_session(chat_id);
		tmp = session_list;
		goto end;
	}

	while (tmp->chat_id != chat_id) {
		if (!tmp->next) {
			push_session(chat_id);
			tmp = session_list;
			break;
		}

		tmp = tmp->next;
	}

	end:
		update_cache(chat_id, tmp);
		return tmp;
}


uint session_list_len(void) {
	uint len = 0;
	struct session *tmp = session_list;

	for (; tmp; ++len)
		tmp = tmp->next;

	// printf("list len: %lu\n", len);
	return len;
}


static void *garbage_collector() {
	struct session *tmp;
	time_t current_time;
	int64_t idtmp;


	for (;;) {
		pthread_mutex_lock(&mutex);
		tmp = session_list;
		current_time = time(NULL);
		struct session *prev = NULL;

		while (tmp) {
			if (current_time - tmp->timestamp >= 3600) {
				prev ? (prev->next = tmp->next) : (session_list = tmp->next);
				free(tmp->bot_ptr);
				free(tmp);
			}

			prev = tmp;
			tmp = tmp->next;
		}
		pthread_mutex_unlock(&mutex);
		sleep(60);
	}
}


void run_dispatcher(const char *token) {
	_Bool is_first_run = 0;
	
	int last_update_id = 0;
	int updates_length;
	
	struct json_object *response;
	struct json_object *updates;

	pthread_mutex_init(&mutex, NULL);
	init_engine(token);

	pthread_t gcid;
	pthread_create(&gcid, NULL, garbage_collector, NULL);
	pthread_detach(gcid);

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

				if (!(json_object_object_get_ex(update, "message", &message)
						&& json_object_object_get_ex(message, "chat", &chat)
						&& json_object_object_get_ex(chat, "id", &chat_id)
						&& json_object_object_get_ex(update, "update_id", &update_id))) {

					continue;
				}

				last_update_id = json_object_get_int(update_id);
				current_chat_id = json_object_get_int64(chat_id);

				pthread_mutex_lock(&mutex);
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
				pthread_mutex_unlock(&mutex);
			}
			is_first_run = 0;
		}
	}
}
