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


    char mes[4096];
    sprintf_s(mes, sizeof(mes), "Hi\nI am telegram bot\nYour message is ||%s||", message.text);
    bot_send_message(bot, message.chat.id, mes, MarkdownV2);
    int list_id[2] = { 1, 0 };
    if (strcmp(message.text, "List") == 0) {
        update_list_id(list_id);
        printf("Block:%d\tNumber:%d\n", list_id[0], list_id[1]);
    }
}
