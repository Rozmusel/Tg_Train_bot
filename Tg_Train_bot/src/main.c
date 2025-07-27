#include <stdio.h>
#include <stdarg.h>

#include "bot.h"
#include "exercises.h"

#pragma execution_character_set("utf-8")    // for sending strings in russian

void callback(BOT* bot, message_t message);

int main(void) {
	system("chcp 65001"); // Set console code page to UTF-8

    BOT* bot = bot_create();

    if (bot == NULL) return -1;

    bot_start(bot, callback);

    bot_delete(bot);

    return 0;
}


void callback(BOT* bot, message_t message) {
    const char* butt[] = { "create", "read", "update" };
    if (message.text != NULL) {
        printf("%s | %s\n", message.user.username, message.text);
        bot_send_message_with_keyboard(bot, message.chat.id, message.text, butt, 3);
    } else if (message.callback_data != NULL) {
        printf("%s | %s\n", message.user.username, message.callback_data);
        bot_send_message(bot, message.chat.id, message.callback_data, HTML);
    } else {
        printf("%s | Unknown message type\n", message.user.username);
        return;
	}    
}
