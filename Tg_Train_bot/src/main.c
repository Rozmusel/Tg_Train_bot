#include <stdio.h>

#include "bot.h"


void callback(message_t message);


int main(void) {
    BOT* bot = bot_create();

    if (bot == NULL) return -1;

    bot_start(bot, callback);

    bot_delete(bot);

    return 0;
}


void callback(message_t message) {
    printf("%s | %s\n", message.user.username, message.text);
}
