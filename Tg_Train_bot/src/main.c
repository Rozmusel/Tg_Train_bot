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
    if (/*��������� ������������� ������� List?*/) {
        update_list_id(&list_id); // ������������� ������� ���������� ������
        send_list(list_id); // ��������� � ���������� ��������� ����� ������������� ������� bot.h
        return 0;
    }
    if (/*��������� ���������� � �����?*/) {// ���� ������������ ����� �������� ��� ��� ���������� ����������, �� ��������� ����� ���������� � ������ ��� ������
        if (list_id[0] == list_id[1] == 0) {
            /*����� ��������� ������� bot.h ����� ��������� ��������� "������ �� ������"*/
            return 0;
        }
    }
    /*����� ��������� ������� bot.h ����� ��������� ��������� "������� �� ����������"*/
}
