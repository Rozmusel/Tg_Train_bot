#include "bot.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <curl/curl.h>
#include <json-c/json.h>


char* parse_modes[] = { "NoParseMode", "MarkdownV2", "HTML", "Markdown" };


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
		printf("ERROR: Error during memory reallocation for telegram response\n");
		return 0;
	}

	resp->data = ptr;
	memcpy_s(&(resp->data[resp->size]), new_resp_size, data, realsize);
	resp->size += realsize;
	resp->data[resp->size] = '\0';

	return realsize;
}


static void get_method_url(char* url, size_t url_size, char* token, char* method) {
	sprintf_s(url, url_size, "https://api.telegram.org/bot%s/%s?", token, method);
}


static errno_t add_url_param_str(CURL* curl, char* url, size_t url_size, char* key, char* value) {
	char data[2048];
	
	char* val = curl_easy_escape(curl, value, strlen(value));
	if (val == NULL) {
		printf("ERROR: Error during escaping of special characters\n");
		return EBADMSG;
	}

	sprintf_s(data, 2048, "%s=%s&", key, val);
	curl_free(val);

	return strcat_s(url, url_size, data);
}


static errno_t add_url_param_uint(CURL* curl, char* url, size_t url_size, char* key, uint64_t value) {
	char val[1024];

	sprintf_s(val, 1024, "%lld", value);

	return add_url_param_str(curl, url, url_size, key, val);
}


static user_t parse_user(json_object* json_user) {
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
		else printf("ERROR: Error during memory allocation for the user.first_name\n");
	}

	if (last_name != NULL) {
		size_t last_name_size = strlen(last_name) + 1;
		user.last_name = malloc(last_name_size);
		if (user.last_name != NULL) memcpy_s(user.last_name, last_name_size, last_name, last_name_size);
		else printf("ERROR: Error during memory allocation for the user.last_name\n");
	}

	if (username != NULL) {
		size_t username_size = strlen(username) + 1;
		user.username = malloc(username_size);
		if (user.username != NULL) memcpy_s(user.username, username_size, username, username_size);
		else printf("ERROR: Error during memory allocation for the user.username\n");
	}

	return user;
}


static chat_t parse_chat(json_object* json_chat) {
	const char* type = json_object_get_string(json_object_object_get(json_chat, "type"));

	chat_t chat = {
		json_object_get_uint64(json_object_object_get(json_chat, "id")),
		NULL,
	};

	if (type != NULL) {
		size_t type_size = strlen(type) + 1;
		chat.type = malloc(type_size);
		if (chat.type != NULL) memcpy_s(chat.type, type_size, type, type_size);
		else printf("ERROR: Error during memory allocation for the chat.type\n");
	}

	return chat;
}


static message_t parse_message(json_object* json_message) {
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
		else printf("ERROR: Error during memory allocation for the message.text\n");
	}

	return message;
}


static update_t parse_update(json_object* json_update) {
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
		printf("ERROR: Error during memory allocation to create a bot structure\n");
		return NULL;
	}

	bot->curl = curl_easy_init();
	if (bot->curl == NULL) {
		printf("ERROR: Error during initialization of the curl structure\n");
		free(bot);
		return NULL;
	}

	bot->token = malloc(1024);

	if (curl_easy_setopt(bot->curl, CURLOPT_WRITEFUNCTION, write_callback) != CURLE_OK) {
		printf("ERROR: Error during setting the curl flag CURLOPT_WRITEFUNCTION\n");
		bot_delete(bot);
		return NULL;
	}

	size_t size = 0;
	if ((_dupenv_s(&bot->token, &size, "BOT_TOKEN")) || (size == 0)) {
		printf("ERROR: Error while reading the BOT_TOKEN environment variable\n");
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
		printf("ERROR: Error during setting the curl flag CURLOPT_WRITEDATA\n");
		return 0;
	}

	char url[4096];
	get_method_url(url, sizeof(url), bot->token, "getUpdates");

	if (add_url_param_uint(bot->curl, url, sizeof(url), "offset", bot->last_update_id + 1)) {
		printf("ERROR: Error while adding the 'offset' parameter to the request url\n");
		return 0;
	}

	if (add_url_param_uint(bot->curl, url, sizeof(url), "timeout", 1)) {
		printf("ERROR: Error while adding the 'timeout' parameter to the request url\n");
		return 0;
	}
	
	if (curl_easy_setopt(bot->curl, CURLOPT_URL, url) != CURLE_OK) {
		printf("ERROR: Error during setting the curl flag CURLOPT_URL\n");
		return 0;
	}

	if (curl_easy_perform(bot->curl) != CURLE_OK) {
		printf("ERROR: Error during https request execution\n");
		return 0;
	}
	
	json_object* obj = json_tokener_parse(buffer.data);
	
	if (json_object_get_boolean(json_object_object_get(obj, "ok")) == 0) {
		printf("ERROR: The response from the telegram server is false\n");
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


void bot_send_message(BOT* bot, uint64_t chat_id, char* text, parse_mode_t parse_mode) {
	response_t buffer = { NULL, 0 };
	if (curl_easy_setopt(bot->curl, CURLOPT_WRITEDATA, (void*)&buffer) != CURLE_OK) {
		printf("ERROR: Error during setting the curl flag CURLOPT_WRITEDATA\n");
		return;
	}

	char url[16384];
	get_method_url(url, sizeof(url), bot->token, "sendMessage");

	if (add_url_param_uint(bot->curl, url, sizeof(url), "chat_id", chat_id)) {
		printf("ERROR: Error while adding the 'chat_id' parameter to the request url\n");
		return;
	}

	if (add_url_param_str(bot->curl, url, sizeof(url), "text", text)) {
		printf("ERROR: Error while adding the 'text' parameter to the request url\n");
		return;
	}

	if (parse_mode) {
		if (add_url_param_str(bot->curl, url, sizeof(url), "parse_mode", parse_modes[parse_mode])) {
			printf("ERROR: Error while adding the 'parse_mode' parameter to the request url\n");
			return;
		}
	}

	if (curl_easy_setopt(bot->curl, CURLOPT_URL, url) != CURLE_OK) {
		printf("ERROR: Error during setting the curl flag CURLOPT_URL\n");
		return;
	}

	if (curl_easy_perform(bot->curl) != CURLE_OK) {
		printf("ERROR: Error during https request execution\n");
		return;
	}

	json_object* obj = json_tokener_parse(buffer.data);
	if (json_object_get_boolean(json_object_object_get(obj, "ok")) == 0) {
		printf("ERROR: The response from the telegram server is false\n");
		json_object_put(obj);
		return;
	}
	json_object_put(obj);
}
