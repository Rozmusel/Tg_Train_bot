#include <stdio.h>

#include "bot.h"
#include "exercises.h"


void callback(BOT* bot, message_t message);


int main(void) {
    BOT* bot = bot_create();

    if (bot == NULL) return -1;

    bot_start(bot, callback);

    bot_delete(bot);

    return 0;
}


void callback(BOT* bot, message_t message) {
    printf("%s | %s\n", message.user.username, message.text);
    // if (bot_send_message(bot, message.chat.id, "Hello\\!\nI'm ||magic|| bot\\!", MarkdownV2) != 0) printf("ERROR\n"); // ok
    // if (bot_send_message(bot, message.chat.id, "Hello!\nI'm ||magic|| bot!", MarkdownV2) != 0) printf("ERROR\n"); // error
    int list_id[2];
    char exr_list[512];
    if (strcmp(message.text, "List") == 0) {
        get_list_id(list_id);
        get_list(list_id, exr_list);
        bot_send_message(bot, message.chat.id, exr_list, NoParseMode);
    }
}
