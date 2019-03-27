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


#define HASH_CODE(c) c % CACHE_LEN


struct session {
	int64_t chat_id;
	time_t timestamp;
	struct bot *bot_ptr;

	struct session *prev;
	struct session *next;
};


struct session session_list = {.chat_id = 0, .bot_ptr = NULL, .next = NULL, .prev = NULL};
struct session *session_cache[CACHE_LEN] = {NULL};
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


static int hash_code(int64_t chat_id) {
	return chat_id % CACHE_LEN;
}


static void update_cache(int64_t chat_id, struct session *session_ptr) {
	session_cache[hash_code(chat_id)] = session_ptr;
}


static _Bool is_response_ok(const struct json_object *response) {
	struct json_object *ok;

	if (!json_object_object_get_ex(response, "ok", &ok)
			&& json_object_get_type(ok) != json_type_boolean)
		
		return 0;

	return json_object_get_boolean(ok);
}


static int populate_session(struct session *session_ptr, int64_t chat_id) {
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


static struct session *get_session_ptr(int64_t chat_id) {
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
				session_ptr->next->prev = session_ptr;
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


void print_session_list_len(void) {
	int i = 0;
	struct session *session_ptr = &session_list;

	if (session_ptr->chat_id)
		i++;

	for (; session_ptr->next; i++) {
		session_ptr = session_ptr->next;
		puts("sas");
	}

	printf("list len: %d\n", i);
}


static void *garbage_collector() {
	struct session *current_ptr;
	time_t current_time;
	int64_t idtmp;

	for (;;) {
		print_session_list_len(); // DEBUG
		pthread_mutex_lock(&mutex);
		current_ptr = &session_list;
		current_time = time(NULL);

		while (current_ptr) {
			if (current_time - current_ptr->timestamp >= /*3600*/ 5) {
				idtmp = current_ptr->chat_id;

				struct session *ssntmp = session_cache[hash_code(idtmp)];				
				if (ssntmp && ssntmp->chat_id == idtmp)
					session_cache[hash_code(idtmp)] = NULL;


				if (current_ptr->prev) {
					current_ptr->prev->next = current_ptr->next;
					free(current_ptr->bot_ptr);
					free(current_ptr);
				}

				else {
					if (current_ptr->next) {
						struct session *tmp_ptr;
						free(current_ptr->bot_ptr);

						tmp_ptr = current_ptr->next;
						current_ptr->chat_id = current_ptr->next->chat_id;
						current_ptr->bot_ptr = current_ptr->next->bot_ptr;
						current_ptr->timestamp = current_ptr->next->timestamp;
						current_ptr->next = current_ptr->next->next;
						free(tmp_ptr);
					}

					else {
						session_list.chat_id = 0;
						session_list.bot_ptr = NULL;
						session_list.next = NULL;
						session_list.prev = NULL;
					}
				}
			}

			current_ptr = current_ptr->next;
		}

		pthread_mutex_unlock(&mutex);
		print_session_list_len();
		sleep(5);
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

	// pthread_t gcid;
	// pthread_create(&gcid, NULL, garbage_collector, NULL);
	// pthread_detach(gcid);

	for (;;) {
		print_session_list_len();

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

				// if (!json_object_object_get_ex(update, "message", &message)
				// 		&& !json_object_object_get_ex(message, "chat", &chat)
				// 		&& !json_object_object_get_ex(chat, "id", &chat_id)
				// 		&& !json_object_object_get_ex(update, "update_id", &update_id)) {

				// 	continue;
				// }


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
