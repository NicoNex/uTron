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

#ifndef NETWORK_H_
#define NETWORK_H_

// This struct contains the data returned by curl.
struct memory_buffer {
	char *memory;
	size_t size;
};

// This function makes a HTTP request to the specified url with the GET method.
struct memory_buffer send_get_request(const char *url);

// This function makes a HTTP request including files to the specified url with POST method.
struct memory_buffer send_post_request(const char *url, const char *filepath, const char *filetype);

#endif // NETWORK_H_
