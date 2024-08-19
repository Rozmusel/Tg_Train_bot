#include "bot.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#include <curl/curl.h>
#include <json-c/json.h>


typedef struct {
	char* data;
	size_t size;
} response_t;


// ? memory leak
static size_t write_callback(char* data, size_t size, size_t nmemb, void* clientp) {
	size_t realsize = size * nmemb;
	response_t* resp = (response_t*)clientp;

	char* ptr = realloc(resp->data, resp->size + realsize + 1);
	if (!ptr) {
		printf("%s\n", "ERROR: Error during memory reallocation for telegram response");
		//free(resp->data);
		return 0;
	}
	//free(resp->data);

	resp->data = ptr;
	memcpy(&(resp->data[resp->size]), data, realsize);
	resp->size += realsize;
	resp->data[resp->size] = 0;

	return realsize;
}


char* get_method_url(char* token, char* method) {
	size_t url_size = strlen("https://api.telegram.org/bot") + strlen(token) + strlen(method) + 1;
	char* url = malloc(url_size);
	if (url == NULL) {
		printf("%s\n", "ERROR: Error during memory allocation to create a method url");
		return;
	}

	strcpy_s(url, url_size, "https://api.telegram.org/bot");
	strcat_s(url, url_size, token);
	strcat_s(url, url_size, method);

	return url;
}


user_t parse_user(json_object* json_user) {
	char* first_name = json_object_get_string(json_object_object_get(json_user, "first_name"));
	char* last_name = json_object_get_string(json_object_object_get(json_user, "last_name"));
	char* username = json_object_get_string(json_object_object_get(json_user, "username"));

	user_t user = {
		json_object_get_uint64(json_object_object_get(json_user, "id")),
		"",
		"",
		""
	};

	if (first_name != NULL) {
		size_t first_name_size = strlen(first_name) + 1;
		user.first_name = malloc(first_name_size);
		if (user.first_name != NULL) memcpy(user.first_name, first_name, first_name_size);
	}

	if (last_name != NULL) {
		size_t last_name_size = strlen(last_name) + 1;
		user.last_name = malloc(last_name_size);
		if (user.last_name != NULL) memcpy(user.last_name, last_name, last_name_size);
	}

	if (username != NULL) {
		size_t username_size = strlen(username) + 1;
		user.username = malloc(username_size);
		if (user.username != NULL) memcpy(user.username, username, username_size);
	}

	return user;
}


message_t parse_message(json_object* json_message) {
	user_t user = parse_user(json_object_object_get(json_message, "from"));

	char* text = json_object_get_string(json_object_object_get(json_message, "text"));

	message_t message = {
		user,
		""
	};

	if (text != NULL) {
		size_t text_size = strlen(text) + 1;
		message.text = malloc(text_size);
		if (message.text != NULL) memcpy(message.text, text, text_size);
	}

	return message;
}


update_t parse_update(json_object* json_update) {
	message_t message = parse_message(json_object_object_get(json_update, "message"));

	update_t update = {
		json_object_get_uint64(json_object_object_get(json_update, "update_id")),
		message
	};

	return update;
}


BOT* bot_create() {
	BOT* bot = malloc(sizeof(BOT));
	if (bot == NULL) {
		printf("%s\n", "ERROR: Error during memory allocation to create a bot structure");
		return NULL;
	}

	bot->curl = curl_easy_init();
	if (bot->curl == NULL) {
		printf("%s\n", "ERROR: Error during initialization of the curl structure");
		return NULL;
	}

	if (curl_easy_setopt(bot->curl, CURLOPT_WRITEFUNCTION, write_callback) != CURLE_OK) {
		printf("%s\n", "ERROR: Error during setting the curl flag CURLOPT_WRITEFUNCTION");
		return NULL;
	}

	size_t size = 0;
	if ((_dupenv_s(&bot->token, &size, "BOT_TOKEN")) || (size == 0)) {
		printf("%s\n", "ERROR: Error while reading the BOT_TOKEN environment variable");
		return NULL;
	}

	return bot;
}


void bot_delete(BOT* bot) {
	curl_easy_cleanup(bot->curl);
	free(bot->token);
}


void bot_start(BOT* bot, void (*callback)(message_t)) {
	while (1) {
		update_t updates[100] = { NULL };
		uint64_t updates_count = bot_get_updates(bot, updates);
		
		for (uint64_t i = 0; i < updates_count; ++i) {
			(*callback)(updates[i].message);
		}

		Sleep(1000);
	}
}


uint64_t bot_get_updates(BOT* bot, update_t* updates) {
	response_t buffer = { NULL };

	if (curl_easy_setopt(bot->curl, CURLOPT_WRITEDATA, (void*)&buffer) != CURLE_OK) {
		printf("%s\n", "ERROR: Error during setting the curl flag CURLOPT_WRITEDATA");
		return 0;
	}

	char* url = get_method_url(bot->token, "/getUpdates");
	if (curl_easy_setopt(bot->curl, CURLOPT_URL, url) != CURLE_OK) {
		printf("%s\n", "ERROR: Error during setting the curl flag CURLOPT_URL");
		free(url);
		return 0;
	}

	if (curl_easy_perform(bot->curl) != CURLE_OK) {
		printf("%s\n", "ERROR: Error during https request execution");
		free(url);
		return 0;
	}
	free(url);
	
	json_object* obj = json_tokener_parse(buffer.data);
	// free(buffer.data); // ? mem leak
	
	if (json_object_get_boolean(json_object_object_get(obj, "ok")) == 0) {
		printf("%s\n", "ERROR: The response from the telegram server is false");
		json_object_put(obj);
		return 0;
	}

	uint64_t updates_count = 0;
	json_object* json_updates = json_object_object_get(obj, "result");
	while (1) {
		json_object* json_update = json_object_array_get_idx(json_updates, updates_count++);
		if (json_update == NULL) break;

		updates[updates_count - 1] = parse_update(json_update);
	}

	json_object_put(obj);

	return --updates_count;
}
