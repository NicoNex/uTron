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

#include "bot.h"
#include "btree.h"
#include "engine.h"
#include "dispatcher.h"

static _Bool is_response_ok(const struct json_object *response) {
	struct json_object *ok;
	if (!json_object_object_get_ex(response, "ok", &ok)
			&& json_object_get_type(ok) != json_type_boolean)
		return 0;

	return json_object_get_boolean(ok);
}

void run_dispatcher(const char *token) {
	btree sessions = NULL;
	_Bool is_first_run = 0;
	int last_update_id = 0;
	int updates_length;
	struct json_object *response;
	struct json_object *updates;

	init_engine(token);

	for (;;) {
		response = is_first_run ? tg_get_updates(0, 0) : tg_get_updates(20, last_update_id+1);
		if (is_response_ok(response)) {
			if (!json_object_object_get_ex(response, "result", &updates))
				continue;

			updates_length = json_object_array_length(updates);

			#pragma omp parallel for schedule(static)
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

				struct node *session = get_node(sessions, current_chat_id);
				if (!session){
					session = new_node(current_chat_id, new_bot(current_chat_id));
					add_node(&sessions, session);
				}

				if (!is_first_run)
					update_bot(session->bot, update);
			}
			json_object_put(response);
			is_first_run = 0;
		}
	}
}
