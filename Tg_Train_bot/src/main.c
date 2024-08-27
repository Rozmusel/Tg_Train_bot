#include <stdio.h>
#include <locale.h>

#include "bot.h"
#include "exercises.h"

#pragma execution_character_set("utf-8")    // for sending strings in russian

#define MAX_MSG_LEN 512
#define MAX_EXR_LEN 1024

void callback(BOT* bot, message_t message);


int main(void) {
    setlocale(LC_ALL, "rus");   // for working with isdigit

    BOT* bot = bot_create();

    if (bot == NULL) return -1;

    bot_start(bot, callback);

    bot_delete(bot);

    return 0;
}


void callback(BOT* bot, message_t message) {
    printf("%s | %s\n", message.user.username, message.text);
    int list_id[2]; // Block and exercise number
    char exr_list[MAX_EXR_LEN];
    memset(exr_list, '\0', MAX_EXR_LEN);
    if (strcmp(message.text, "/list") == 0 || strcmp(message.text, "Список") == 0) {
        get_list_id(list_id);
        get_list(list_id, exr_list);
        bot_send_message(bot, message.chat.id, exr_list, NoParseMode);
    }
    if (isdigit(message.text[0])) {
        if (exr_list[0] == '\0') {
            bot_send_message(bot, message.chat.id, "Сначала откройте список", NoParseMode);
            return;
        }
        /*if (edit_exr(exr_list, message.chat.id) == 0) {
            bot_send_message(
                bot,
                message.chat.id,
                "Введите данные в правильном формате\nДля получения справки отправьте команду /help или напишите \"Справка\"",
                NoParseMode
            );
            return;
        }*/
        bot_send_message(bot, message.chat.id, "Изменения применены", NoParseMode);
    }
    if (strcmp(message.text, "Справка") == 0 || strcmp(message.text, "/help") == 0) {
        bot_send_message(
            bot,
            message.chat.id,
            "Чтобы вызвать следующий список оправьте сообщение \"Список\" или команду /list",
            NoParseMode
        );
        bot_send_message(
           bot,
            message.chat.id,
            "Для изменения веса и подходов в упражнении воспользуютесь одним из следующих форматов:\nx. w/s w/s w/s ...\nx. s s s ...\nГде w - вес, а s - количество подходов. x - это номер упражнения",
            NoParseMode
        );
        return;
    }
}
