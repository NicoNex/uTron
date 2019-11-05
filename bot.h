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

#ifndef BOT_H_
#define BOT_H_

#include <json_object.h>

// This struct must be implemented by the user.
// This is the struct containing the context of the bot.
struct bot;

// This function must be implemented by the user.
// It is the function the dispatcher calls when a new user writes to the bot.
struct bot *new_bot(int64_t chat_id);

// This function must be implemented by the user.
// It gets called every time the dispatcher receives an update
// that belongs to the relative 'bot' instance.
void update_bot(struct bot *bot, struct json_object *update);

#endif // BOT_H_
