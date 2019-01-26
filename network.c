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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "network.h"


static size_t write_memory_callback(void *contents, size_t size, size_t nmemb, void *userp) {
	size_t realsize = size * nmemb;
	struct memory_buffer_t *mem = (struct memory_buffer_t *)userp;

	char *ptr = realloc(mem->memory, mem->size + realsize + 1);

	if (ptr == NULL) {
		puts("not enough memory (realloc returned NULL)");
		return 0;
	}

	mem->memory = ptr;
	memcpy(&(mem->memory[mem->size]), contents, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;

	return realsize;
}


struct memory_buffer_t send_get_request(const char *url) {
	CURL *curl_sessn;
	CURLcode result;

	struct memory_buffer_t buffer;
	buffer.memory = malloc(1);
	buffer.size = 0;

	curl_global_init(CURL_GLOBAL_ALL);
	
	curl_sessn = curl_easy_init();

	if (!curl_sessn) {
		fputs("failed to initialize curl session\n", stderr);
		goto END;
	}

	curl_easy_setopt(curl_sessn, CURLOPT_URL, url);
	curl_easy_setopt(curl_sessn, CURLOPT_WRITEFUNCTION, write_memory_callback);
	curl_easy_setopt(curl_sessn, CURLOPT_WRITEDATA, (void *)&buffer);
	result = curl_easy_perform(curl_sessn);

	if (result != CURLE_OK) {
		fprintf(stderr, "request failed: %s", curl_easy_strerror(result));
		goto END;
	}


END:
	curl_easy_cleanup(curl_sessn);
	curl_global_cleanup();
	return buffer;
}
