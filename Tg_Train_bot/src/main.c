#include <stdio.h>

#include "bot.h"
#include "exercises.h"


void callback(message_t message);


int main(void) {
    BOT* bot = bot_create();

    if (bot == NULL) return -1;

    bot_start(bot, callback);

    bot_delete(bot);

    return 0;
}


void callback(message_t message) {
    //printf("%s | %s\n", message.user.username, message.text);
    int list_id[2] = { 0, 0 };
    if (/*The message corresponds to the List command?*/) {
        update_list_id(&list_id);// The position of the next list is assigned
        send_list(list_id); // Generates and sends a message using functions bot.h
        return 0;
    }
    if (/*Does the message start with a number?*/) {// If the user wants to change the weight or the number of repetitions, he enters the exercise number and the data for replacement
        if (list_id[0] == list_id[1] == 0) {
            /*Here I use the bot.h function to send the message "List not opened"*/
            return 0;
        }
    }
    /*Here I use the bot.h function to send a message "Command not recognized"*/
}
