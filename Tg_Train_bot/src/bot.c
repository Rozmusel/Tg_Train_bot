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
	const size_t realsize = size * nmemb;
	response_t* resp = (response_t*)clientp;
	const size_t new_resp_size = resp->size + realsize + 1;

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
	char* val = curl_easy_escape(curl, value, strlen(value));
	if (val == NULL) {
		printf("ERROR: Error during escaping of special characters\n");
		return EBADMSG;
	}

	const size_t data_size = strlen(key) + strlen(val) + 2 + 1; // len of key + len of val + "=&" + '\0'
	char* data = malloc(data_size);
	if (data == NULL) {
		printf("ERROR: Error during memory allocation for the url parameter\n");
		return ENOMEM;
	}

	sprintf_s(data, data_size, "%s=%s&", key, val);
	curl_free(val);

	errno_t result = strcat_s(url, url_size, data);

	free(data);

	return result;
}


static errno_t add_url_param_uint(CURL* curl, char* url, size_t url_size, char* key, uint64_t value) {
	char val[21]; // max uint64_t - 18446744073709551615 (20 digits) and '\0'

	sprintf_s(val, sizeof(val), "%lld", value);

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
		const size_t first_name_size = strlen(first_name) + 1;
		user.first_name = malloc(first_name_size);
		if (user.first_name != NULL) memcpy_s(user.first_name, first_name_size, first_name, first_name_size);
		else printf("ERROR: Error during memory allocation for the user.first_name\n");
	}

	if (last_name != NULL) {
		const size_t last_name_size = strlen(last_name) + 1;
		user.last_name = malloc(last_name_size);
		if (user.last_name != NULL) memcpy_s(user.last_name, last_name_size, last_name, last_name_size);
		else printf("ERROR: Error during memory allocation for the user.last_name\n");
	}

	if (username != NULL) {
		const size_t username_size = strlen(username) + 1;
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
		const size_t type_size = strlen(type) + 1;
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
		const size_t text_size = strlen(text) + 1;
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

	CURLcode curl_code = curl_easy_setopt(bot->curl, CURLOPT_WRITEFUNCTION, write_callback);
	if (curl_code != CURLE_OK) {
		printf("ERROR: Error during setting the curl flag CURLOPT_WRITEFUNCTION (%s)\n", curl_easy_strerror(curl_code));
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
		update_t updates[100]; // max telegram bot api response - 100 update_t (information from telegram bot api specification)
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
	CURLcode curl_code = CURLE_OK;
	response_t buffer = { NULL, 0 };

	curl_code = curl_easy_setopt(bot->curl, CURLOPT_WRITEDATA, (void*)&buffer);
	if (curl_code != CURLE_OK) {
		printf("ERROR: Error during setting the curl flag CURLOPT_WRITEDATA (%s)\n", curl_easy_strerror(curl_code));
		return 0;
	}

	const uint64_t uint_param_count = 2;
	const uint64_t param_count = uint_param_count;
	const size_t url_size =
		strlen("https://api.telegram.org/bot/?") +
		strlen(bot->token) +
		strlen("getUpdates") +
		2 * param_count +
		strlen("offset") +
		strlen("timeout") +
		20 * uint_param_count
		+ 1;
	
	char* url = malloc(url_size);
	if (url == NULL) {
		printf("ERROR: Error during memory allocation for url\n");
		return 0;
	}

	get_method_url(url, url_size, bot->token, "getUpdates");

	if (add_url_param_uint(bot->curl, url, url_size, "offset", bot->last_update_id + 1)) {
		printf("ERROR: Error while adding the 'offset' parameter to the request url\n");
		free(url);
		return 0;
	}

	if (add_url_param_uint(bot->curl, url, url_size, "timeout", 1)) {
		printf("ERROR: Error while adding the 'timeout' parameter to the request url\n");
		free(url);
		return 0;
	}
	
	curl_code = curl_easy_setopt(bot->curl, CURLOPT_URL, url);
	if (curl_code != CURLE_OK) {
		printf("ERROR: Error during setting the curl flag CURLOPT_URL (%s)\n", curl_easy_strerror(curl_code));
		free(url);
		return 0;
	}

	curl_code = curl_easy_perform(bot->curl);
	if (curl_code != CURLE_OK) {
		printf("ERROR: Error during https request execution (%s)\n", curl_easy_strerror(curl_code));
		free(url);
		return 0;
	}
	free(url);
	
	json_object* obj = json_tokener_parse(buffer.data);
	
	if (json_object_get_boolean(json_object_object_get(obj, "ok")) == 0) {
		int32_t error_code = json_object_get_int(json_object_object_get(obj, "error_code"));
		const char* description = json_object_get_string(json_object_object_get(obj, "description"));
		printf("ERROR: The response from the telegram server is false (%i - %s)\n", error_code, description);
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
	CURLcode curl_code = CURLE_OK;
	response_t buffer = { NULL, 0 };

	curl_code = curl_easy_setopt(bot->curl, CURLOPT_WRITEDATA, (void*)&buffer);
	if (curl_code != CURLE_OK) {
		printf("ERROR: Error during setting the curl flag CURLOPT_WRITEDATA (%s)\n", curl_easy_strerror(curl_code));
		return;
	}

	const uint64_t uint_param_count = 1;
	const uint64_t str_param_count = 1;
	const uint64_t param_count = uint_param_count + str_param_count + (parse_mode != NoParseMode ? 1 : 0);
	const size_t url_size =
		strlen("https://api.telegram.org/bot/?") +
		strlen(bot->token) +
		strlen("sendMessage") +
		2 * param_count +
		strlen("chat_id") +
		strlen("text") +
		(parse_mode != NoParseMode ? strlen("parse_mode") : 0) +
		20 * uint_param_count +
		strlen(curl_easy_escape(bot->curl, text, strlen(text))) +
		(parse_mode != NoParseMode ? strlen(parse_modes[parse_mode]) : 0)
		+ 1;

	char* url = malloc(url_size);
	if (url == NULL) {
		printf("ERROR: Error during memory allocation for url\n");
		return;
	}

	get_method_url(url, url_size, bot->token, "sendMessage");

	if (add_url_param_uint(bot->curl, url, url_size, "chat_id", chat_id)) {
		printf("ERROR: Error while adding the 'chat_id' parameter to the request url\n");
		free(url);
		return;
	}

	if (add_url_param_str(bot->curl, url, url_size, "text", text)) {
		printf("ERROR: Error while adding the 'text' parameter to the request url\n");
		free(url);
		return;
	}

	if (parse_mode != NoParseMode) {
		if (add_url_param_str(bot->curl, url, url_size, "parse_mode", parse_modes[parse_mode])) {
			printf("ERROR: Error while adding the 'parse_mode' parameter to the request url\n");
			free(url);
			return;
		}
	}

	curl_code = curl_easy_setopt(bot->curl, CURLOPT_URL, url);
	if (curl_code != CURLE_OK) {
		printf("ERROR: Error during setting the curl flag CURLOPT_URL (%s)\n", curl_easy_strerror(curl_code));
		free(url);
		return;
	}

	curl_code = curl_easy_perform(bot->curl);
	if (curl_code != CURLE_OK) {
		printf("ERROR: Error during https request execution (%s)\n", curl_easy_strerror(curl_code));
		free(url);
		return;
	}
	free(url);

	json_object* obj = json_tokener_parse(buffer.data);
	if (json_object_get_boolean(json_object_object_get(obj, "ok")) == 0) {
		int32_t error_code = json_object_get_int(json_object_object_get(obj, "error_code"));
		const char* description = json_object_get_string(json_object_object_get(obj, "description"));
		printf("ERROR: The response from the telegram server is false (%i - %s)\n", error_code, description);
		json_object_put(obj);
		return;
	}
	json_object_put(obj);
}
