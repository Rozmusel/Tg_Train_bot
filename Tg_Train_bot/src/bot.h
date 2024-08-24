#pragma once


#ifndef _BOT_H_
#define _BOT_H_


#include <stdint.h>

#include <curl/curl.h>


typedef enum {
	NoParseMode = 0,
	MarkdownV2 = 1,
	HTML = 2,
	Markdown = 3
} parse_mode_t;


typedef struct {
	CURL* curl;
	char* token;
	uint64_t last_update_id;
} BOT;


typedef struct {
	uint64_t id;
	char* first_name;
	char* last_name;
	char* username;
} user_t;


typedef struct {
	uint64_t id;
	char* type;
} chat_t;


typedef struct {
	user_t user;
	chat_t chat;
	char* text;
} message_t;


typedef struct {
	uint64_t update_id;
	message_t message;
} update_t;


BOT* bot_create();
void bot_delete(BOT* bot);

void bot_start(BOT* bot, void (*callback)(BOT*, message_t));
uint64_t bot_get_updates(BOT* bot, update_t* updates);

errno_t bot_send_message(BOT* bot, uint64_t chat_id, char* text, parse_mode_t parse_mode);


#endif // !_BOT_H_
