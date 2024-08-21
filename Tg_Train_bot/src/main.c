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

    // int list_id[2] = { 0, 0 };
    // if (/*Ñîîáùåíèå ñîîòâåòñòâóåò êîìàíäå List?*/) {
        // update_list_id(&list_id);// Ïðèñâàèâàåòñÿ ïîçèöèÿ ñëåäóþùåãî ñïèñêà
        // send_list(list_id); // Ôîðìèðóåò è îòïðàâëÿåò ñîîáùåíèå ÷åðåç èñïîëüçîâàíèå ôóíêöèé bot.h
        // return 0;
    // }
    // if (/*Ñîîáùåíèå íà÷èíàåòñÿ ñ öèôðû?*/) {// Åñëè ïîëüçîâàòåëü õî÷åò ïîìåíÿòü âåñ èëè êîëè÷åñòâî ïîâòîðåíèé, îí âïèñûâàåò öèôðó óïðàæíåíèÿ è äàííûå äëÿ çàìåíû
        // if (list_id[0] == list_id[1] == 0) {
            /*Çäåñü èñïîëüçóþ ôóíêöèþ bot.h ÷òîáû îòïðàâèòü ñîîáùåíèå "Ñïèñîê íå îòêðûò"*/
            // return 0;
        // }
    // }
    /*Çäåñü èñïîëüçóþ ôóíêöèþ bot.h ÷òîáû îòïðàâèòü ñîîáùåíèå "Êîìàíäà íå ðàñïîçíàíà"*/
}
