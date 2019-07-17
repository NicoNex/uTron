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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "network.h"


static size_t write_memory_callback(void *contents, size_t size, size_t nmemb, void *userp) {
	size_t realsize = size * nmemb;
	struct memory_buffer *mem = (struct memory_buffer *)userp;

	char *ptr = realloc(mem->memory, mem->size + realsize + 1);

	if (ptr == NULL) {
		fputs("not enough memory (realloc returned NULL)", stderr);
		return 0;
	}

	mem->memory = ptr;
	memcpy(&(mem->memory[mem->size]), contents, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;

	return realsize;
}


struct memory_buffer send_get_request(const char *url) {
	CURL *curl;
	CURLcode result;

	struct memory_buffer buffer;
	buffer.memory = malloc(1);
	buffer.size = 0;

	curl_global_init(CURL_GLOBAL_ALL);

	curl = curl_easy_init();

	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_memory_callback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&buffer);
		result = curl_easy_perform(curl);

		if (result != CURLE_OK)
			fprintf(stderr, "request failed: %s\n", curl_easy_strerror(result));

		curl_easy_cleanup(curl);
		curl_global_cleanup();
	}

	else
		fputs("failed to initialize curl session\n", stderr);

	return buffer;
}


struct memory_buffer send_post_request(const char *url, const char *filepath, const char *filetype) {
	CURL *curl;
	CURLcode result;

	curl_mime *form = NULL;
	curl_mimepart *field = NULL;
	struct curl_slist *headerlist = NULL;
	static const char tmp[] = "Expect:";

	struct memory_buffer buffer;
	buffer.memory = malloc(1);
	buffer.size = 0;

	curl_global_init(CURL_GLOBAL_ALL);

	curl = curl_easy_init();

	if (curl) {
		form = curl_mime_init(curl);

		field = curl_mime_addpart(form);
		curl_mime_name(field, filetype);
		curl_mime_filedata(field, filepath);

		field = curl_mime_addpart(form);
		curl_mime_name(field, "submit");
		curl_mime_data(field, "send", CURL_ZERO_TERMINATED);

		headerlist = curl_slist_append(headerlist, tmp);

		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_MIMEPOST, form);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_memory_callback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&buffer);

		result = curl_easy_perform(curl);

		if (result != CURLE_OK)
			fprintf(stderr, "request failed: %s\n", curl_easy_strerror(result));

		curl_easy_cleanup(curl);
		curl_mime_free(form);
		curl_slist_free_all(headerlist);
	}

	return buffer;
}
