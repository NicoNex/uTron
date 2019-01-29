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


void run_dispatcher() {
	_Bool is_first_run = 0;
	
	int last_update_id = 0;
	int updates_length;
	
	struct json_object *response;
	struct json_object updates;
	struct engine_t engine = new_engine(TOKEN);

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
			}
		}
	}
}