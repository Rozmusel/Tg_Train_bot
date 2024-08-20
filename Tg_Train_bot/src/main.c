#include <stdio.h>

#include "bot.h"

int list_id[2] = {0, 0};

void callback(message_t message);


int main(void) {
    BOT* bot = bot_create();

    if (bot == NULL) return -1;

    printf("BOT TOKEN: %s\n", bot->token);

    bot_start(bot, callback);

    bot_delete(bot);

    return 0;
}


void callback(message_t message) {
    //printf("%s | %s\n", message.user.username, message.text);
    if (/*Сообщение соответствует команде List?*/) {
        update_list_id(&list_id); // Присваивается позиция следующего списка
        send_list(list_id); // Формирует и отправляет сообщение через использование функций bot.h
        return 0;
    }
    if (/*Сообщение начинается с цифры?*/) {// Если пользователь хочет поменять вес или количество повторений, он вписывает цифру упражнения и данные для замены
        if (list_id[0] == list_id[1] == 0) {
            /*Здесь использую функцию bot.h чтобы отправить сообщение "Список не открыт"*/
            return 0;
        }
    }
    /*Здесь использую функцию bot.h чтобы отправить сообщение "Команда не распознана"*/
}
