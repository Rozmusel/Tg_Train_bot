#include <stdio.h>
#include <windows.h>
#include <locale.h>

#define STRSIZE 128	// Максимальная длина принимаемой строки
#define NUMSIZE 16	// Максимальная длина считываемого числа

FILE* List = NULL;

int day_type();	// Возращает номер блока с учётом истории использования
void block_finder(int block);	// Находит и наводит курсор на нужный блок
int main() {
	setlocale(LC_ALL, "rus");

	errno_t read_res = NULL;	// Хранение результата открытия файла
	read_res = fopen_s(&List, "List.txt", "r+");
	if (read_res != 0) return -1;

	int cursor = 0;
	typedef enum {
		LEGS = 1,
		CHEST = 2,
		BACK = 3
	} mode;
	switch (day_type(&cursor)) {
	case LEGS:
		block_finder(LEGS);

		break;
	case CHEST:
		block_finder(CHEST);
		break;
	case BACK:
		block_finder(BACK);
		break;
	}
}
int day_type() {
	int day = 0;
	int counter = 0;
	int cursor = 0;
	char str[STRSIZE] = { 0 };

	while (!feof(List)) {
		fgets(str, STRSIZE, List);
		if (str[cursor] != '*') continue;
		(cursor)++;
		if (str[cursor] != 'b') continue;
		counter++;
		while (str[cursor] != '>') {
			(cursor)++;
		}
		(cursor)++;
		switch ((int)str[cursor] - 48) {
		case 0:
			day = counter;
			fseek(List, -3, SEEK_CUR);
			putc('1', List);
			break;
		case 1:
			fseek(List, -3, SEEK_CUR);
			putc('2', List);
			break;
		case 2:
			fseek(List, -3, SEEK_CUR);
			putc('0', List);
			break;
		default:
			printf("A FATAL ERROR HAS OCCURRED\nUNKNOWN HISTORY PARAMETER VALUE");
			exit;
		}
		fseek(List, 3, SEEK_CUR);
		cursor = 0;
	}
	return day;
}

void block_finder(int block) {
	char str[STRSIZE] = { 0 };
	int cursor = 0;
	int counter = 0;
	while (!feof(List) && counter < block) {
		fgets(str, STRSIZE, List);
		if (str[cursor] != '*') continue;
		(cursor)++;
		if (str[cursor] != 'b') continue;
		counter++;
	}
	return 0;
}