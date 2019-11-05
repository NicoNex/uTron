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

#ifndef ENGINE_H_
#define ENGINE_H_

#include <stdint.h>
#include <json_object.h>

// The availabe options for the tg_send_message_opts function.
enum send_message_options {
	PARSE_MARKDOWN = 1 << 0,
	PARSE_HTML = 1 << 1,
	DISABLE_WEB_PAGE_PREVIEW = 1 << 2,
	DISABLE_NOTIFICATION = 1 << 3
};

// Use this function to get all updates from Telegram.
struct json_object *tg_get_updates(int timeout, int offset);

// This function sends text messages to a specified chat id.
struct json_object *tg_send_message(char *text, int64_t chat_id);

// Similar to tg_send_message but you can specify some additional options defined in
// send_message_options.
struct json_object *tg_send_message_opts(char *text, int64_t chat_id, int options);

// Use this function to send a document to the specified chat id.
struct json_object *tg_send_document(char *filepath, char *caption, int64_t chat_id);

void init_engine(const char *token);


#endif // ENGINE_H_
