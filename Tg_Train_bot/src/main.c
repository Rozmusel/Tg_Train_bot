#include <stdio.h>
#include "settings.h"
int main(void) {
    char response[MAX_MSG_LEN];
    char key[MAX_MSG_LEN] = "List";
    char output[MAX_MSG_LEN] = "Succes";
    while (1) {  // ����������� ���� ��� ������ ����.
        get_updates(&response);  // ������� � ���������� ���������.
        if (is_key(response, key, 4)) {
            send_message(CHAT_ID, output);
        }
        memset(response, '\0', MAX_MSG_LEN);
        Sleep(1000);  // �������� ����� ��������� ��������.
    }
    return 0;
}