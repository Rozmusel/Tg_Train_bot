#pragma once


#ifndef _BOT_H_
#define _BOT_H_


#include <stdint.h>

#include <curl/curl.h>


typedef struct {
	CURL* curl;
	char* token;
} BOT;


typedef struct {
	uint64_t id;
	char* first_name;
	char* last_name;
	char* username;
} user_t;


typedef struct {
	user_t user;
	char* text;
} message_t;


typedef struct {
	uint64_t update_id;
	message_t message;
} update_t;


BOT* bot_create();
void bot_delete(BOT* bot);

void bot_start(BOT* bot, void (*callback)(message_t));
uint64_t bot_get_updates(BOT* bot, update_t* updates);


#endif // !_BOT_H_
