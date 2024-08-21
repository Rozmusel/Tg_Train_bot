#include "bot.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <curl/curl.h>
#include <json-c/json.h>


typedef struct {
	char* data;
	size_t size;
} response_t;


static size_t write_callback(char* data, size_t size, size_t nmemb, void* clientp) {
	size_t realsize = size * nmemb;
	response_t* resp = (response_t*)clientp;
	size_t new_resp_size = resp->size + realsize + 1;

	char* ptr = realloc(resp->data, new_resp_size);
	if (ptr == NULL) {
		printf("%s\n", "ERROR: Error during memory reallocation for telegram response");
		return 0;
	}

	resp->data = ptr;
	memcpy_s(&(resp->data[resp->size]), new_resp_size, data, realsize);
	resp->size += realsize;
	resp->data[resp->size] = '\0';

	return realsize;
}


void get_method_url(char* url, size_t url_size, char* token, char* method) {
	sprintf_s(url, url_size, "https://api.telegram.org/bot%s/%s?", token, method);
}


errno_t add_url_param_str(char* url, size_t url_size, char* key, char* value) {
	char data[1024];

	sprintf_s(data, 1024, "%s=%s&", key, value);

	return strcat_s(url, url_size, data);
}


errno_t add_url_param_uint(char* url, size_t url_size, char* key, uint64_t value) {
	char val[1024];

	sprintf_s(val, 1024, "%lld", value);

	return add_url_param_str(url, url_size, key, val);
}


user_t parse_user(json_object* json_user) {
	const char* first_name = json_object_get_string(json_object_object_get(json_user, "first_name"));
	const char* last_name = json_object_get_string(json_object_object_get(json_user, "last_name"));
	const char* username = json_object_get_string(json_object_object_get(json_user, "username"));

	user_t user = {
		json_object_get_uint64(json_object_object_get(json_user, "id")),
		NULL,
		NULL,
		NULL
	};

	if (first_name != NULL) {
		size_t first_name_size = strlen(first_name) + 1;
		user.first_name = malloc(first_name_size);
		if (user.first_name != NULL) memcpy_s(user.first_name, first_name_size, first_name, first_name_size);
		else printf("%s\n", "ERROR: Error during memory allocation for the user.first_name");
	}

	if (last_name != NULL) {
		size_t last_name_size = strlen(last_name) + 1;
		user.last_name = malloc(last_name_size);
		if (user.last_name != NULL) memcpy_s(user.last_name, last_name_size, last_name, last_name_size);
		else printf("%s\n", "ERROR: Error during memory allocation for the user.last_name");
	}

	if (username != NULL) {
		size_t username_size = strlen(username) + 1;
		user.username = malloc(username_size);
		if (user.username != NULL) memcpy_s(user.username, username_size, username, username_size);
		else printf("%s\n", "ERROR: Error during memory allocation for the user.username");
	}

	return user;
}


chat_t parse_chat(json_object* json_chat) {
	const char* type = json_object_get_string(json_object_object_get(json_chat, "type"));

	chat_t chat = {
		json_object_get_uint64(json_object_object_get(json_chat, "id")),
		NULL,
	};

	if (type != NULL) {
		size_t type_size = strlen(type) + 1;
		chat.type = malloc(type_size);
		if (chat.type != NULL) memcpy_s(chat.type, type_size, type, type_size);
		else printf("%s\n", "ERROR: Error during memory allocation for the chat.type");
	}

	return chat;
}


message_t parse_message(json_object* json_message) {
	user_t user = parse_user(json_object_object_get(json_message, "from"));
	chat_t chat = parse_chat(json_object_object_get(json_message, "chat"));

	const char* text = json_object_get_string(json_object_object_get(json_message, "text"));

	message_t message = {
		user,
		chat,
		NULL
	};

	if (text != NULL) {
		size_t text_size = strlen(text) + 1;
		message.text = malloc(text_size);
		if (message.text != NULL) memcpy_s(message.text, text_size, text, text_size);
		else printf("%s\n", "ERROR: Error during memory allocation for the message.text");
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
		free(bot);
		return NULL;
	}

	bot->token = malloc(1024);

	if (curl_easy_setopt(bot->curl, CURLOPT_WRITEFUNCTION, write_callback) != CURLE_OK) {
		printf("%s\n", "ERROR: Error during setting the curl flag CURLOPT_WRITEFUNCTION");
		bot_delete(bot);
		return NULL;
	}

	size_t size = 0;
	if ((_dupenv_s(&bot->token, &size, "BOT_TOKEN")) || (size == 0)) {
		printf("%s\n", "ERROR: Error while reading the BOT_TOKEN environment variable");
		bot_delete(bot);
		return NULL;
	}

	bot->last_update_id = 0;

	return bot;
}


void bot_delete(BOT* bot) {
	curl_easy_cleanup(bot->curl);
	free(bot->token);
	free(bot);
}


void bot_start(BOT* bot, void (*callback)(BOT*, message_t)) {
	while (1) {
		update_t updates[100];
		uint64_t updates_count = bot_get_updates(bot, updates);
		
		for (uint64_t i = 0; i < updates_count; ++i) {
			update_t update = updates[i];
			(*callback)(bot, update.message);
			bot->last_update_id = max(bot->last_update_id, update.update_id);
		}

		Sleep(1000);
	}
}


uint64_t bot_get_updates(BOT* bot, update_t* updates) {
	response_t buffer = { NULL, 0 };

	if (curl_easy_setopt(bot->curl, CURLOPT_WRITEDATA, (void*)&buffer) != CURLE_OK) {
		printf("%s\n", "ERROR: Error during setting the curl flag CURLOPT_WRITEDATA");
		return 0;
	}

	char url[4096];
	get_method_url(url, sizeof(url), bot->token, "getUpdates");

	if (add_url_param_uint(url, sizeof(url), "offset", bot->last_update_id + 1)) {
		printf("%s\n", "ERROR: Error while adding the 'offset' parameter to the request url");
		return 0;
	}

	if (add_url_param_uint(url, sizeof(url), "timeout", 1)) {
		printf("%s\n", "ERROR: Error while adding the 'timeout' parameter to the request url");
		return 0;
	}
	
	if (curl_easy_setopt(bot->curl, CURLOPT_URL, url) != CURLE_OK) {
		printf("%s\n", "ERROR: Error during setting the curl flag CURLOPT_URL");
		return 0;
	}

	if (curl_easy_perform(bot->curl) != CURLE_OK) {
		printf("%s\n", "ERROR: Error during https request execution");
		return 0;
	}
	
	json_object* obj = json_tokener_parse(buffer.data);
	
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


void bot_send_message(BOT* bot, uint64_t chat_id, char* text) {
	response_t buffer = { NULL, 0 };
	if (curl_easy_setopt(bot->curl, CURLOPT_WRITEDATA, (void*)&buffer) != CURLE_OK) {
		printf("%s\n", "ERROR: Error during setting the curl flag CURLOPT_WRITEDATA");
		return;
	}

	char url[4096];
	get_method_url(url, sizeof(url), bot->token, "sendMessage");

	if (add_url_param_uint(url, sizeof(url), "chat_id", chat_id)) {
		printf("%s\n", "ERROR: Error while adding the 'chat_id' parameter to the request url");
		return;
	}

	if (add_url_param_str(url, sizeof(url), "text", text)) {
		printf("%s\n", "ERROR: Error while adding the 'text' parameter to the request url");
		return;
	}

	if (curl_easy_setopt(bot->curl, CURLOPT_URL, url) != CURLE_OK) {
		printf("%s\n", "ERROR: Error during setting the curl flag CURLOPT_URL");
		return;
	}

	if (curl_easy_perform(bot->curl) != CURLE_OK) {
		printf("%s\n", "ERROR: Error during https request execution");
		return;
	}

	json_object* obj = json_tokener_parse(buffer.data);
	if (json_object_get_boolean(json_object_object_get(obj, "ok")) == 0) {
		printf("%s\n", "ERROR: The response from the telegram server is false");
		json_object_put(obj);
		return;
	}
	json_object_put(obj);
}
