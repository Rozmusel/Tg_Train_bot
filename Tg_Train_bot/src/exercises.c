#include <stdio.h>
#include <windows.h>
#include <locale.h>

#include "settings.h"

#define STRSIZE 128	// Maximum length of accepted string
#define NUMSIZE 16	// Maximum length of the number to be read
#define EXRSIZE 16
FILE* List = NULL;

void get_next_block(int* list_id);	// The number of the next block is written to the passed array.
void get_next_list(int* list_id);	// The number of the next list is written to the passed array.
void block_finder(int block);	// Finds and hovers the cursor over the desired block
void list_finder(int* list_id);
int get_min();

void get_list_id(int* list_id) {
	get_next_block(list_id);
	get_next_list(list_id);
}
void get_list(int* list_id, char exr_list[]) {
	errno_t read_res = 0;
	read_res = fopen_s(&List, "List.txt", "r+");
	if (read_res != 0) return;
	int cursor = 0;	// Cursor for reading
	int ecounter = 0;	// Column count in exercise list to record
	int scounter = 0;	// Counting the line in the exercise list for recording
	int lcounter = 0;	// Counting a line in a common sheet for recording
	char exercises[EXERSISES][STRSIZE];
	memset(exercises, '\0', sizeof(exercises));
	char str[STRSIZE] = { 0 };
	block_finder(*list_id);
	list_id++;
	list_finder(list_id);
	while (!feof(List)) {
		fgets(str, STRSIZE, List);
		if (str[cursor] == '*' || str[cursor] == '#') break;
		while (str[cursor] != '>') {
			exr_list[lcounter] = str[cursor];
			lcounter++;
			exercises[ecounter][scounter] = str[cursor];
			scounter++;
			cursor++;
		}
		exr_list[lcounter] = '\n';
		lcounter++;
		ecounter++;
		scounter = 0;
		cursor = 0;
		fgets(str, STRSIZE, List);
		while (str[cursor] != '\n') {
			exr_list[lcounter] = str[cursor];
			lcounter++;
			cursor++;
		}
		cursor = 0;
		exr_list[lcounter] = '\n';
		lcounter++;
	}
	int min;
	min = get_min();
	int flag = 0;
	char buf[STRSIZE];
	memset(buf, '\0', STRSIZE);
	fseek(List, 0, SEEK_SET);
	while (!feof(List)) {
		fgets(str, STRSIZE, List);
		if (str[cursor] == '*' || str[cursor] == '#') continue;
		while (str[cursor] != '>') {
			buf[cursor] = str[cursor];
			cursor++;
		}
		cursor++;
		for (int j = 0; strcmp("", exercises[j]) != 0; j++) {
			if (strcmp(buf, exercises[j]) == 0) {
				int num = (int)str[cursor] - 47 - min;
				char cnum = (char)(num + 48);
				fseek(List, -3, SEEK_CUR);
				putc(cnum, List);
				flag = 1;
			}
		}
		if (flag == 0) {
			int num = (int)str[cursor] - 48 - min;
			char cnum = (char)(num + 48);
			fseek(List, -3, SEEK_CUR);
			putc(cnum, List);
		}
		fseek(List, 3, SEEK_CUR);
		fgets(str, STRSIZE, List);
		cursor = 0;
		flag = 0;
		memset(buf, '\0', STRSIZE);
	}
	fclose(List);
}

void get_next_block(int* list_id) {
	errno_t read_res = 0;
	read_res = fopen_s(&List, "List.txt", "r+");
	if (read_res != 0) return;
	int day = 0;
	int counter = 0;
	int cursor = 0;
	char str[STRSIZE] = { 0 };

	while (!feof(List)) {
		fgets(str, STRSIZE, List);
		if (str[cursor] != '#') continue;
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
		}
		fseek(List, 3, SEEK_CUR);
		cursor = 0;
	}
	fclose(List);
	*list_id = day;
}

void get_next_list(int* list_id) {
	errno_t read_res = 0;
	read_res = fopen_s(&List, "List.txt", "r+");
	if (read_res != 0) return;
	block_finder(*list_id);

	int cursor = 0;
	char str[STRSIZE] = { 0 };
	float priority[EXRSIZE] = { 0 };
	int counter = 0;
	int imin = 0;
	int exercises = 0;
	fgets(str, STRSIZE, List);

	while (!feof(List)) {
		cursor = 0;
		fgets(str, STRSIZE, List);
		if (str[cursor] == '#') break;
		if (str[cursor] == '*') {
			fgets(str, STRSIZE, List);
			priority[counter] /= exercises;
			counter++;
			exercises = 0;
		}
		if (isdigit(str[cursor])) continue;
		while (str[cursor] != '>') {
			cursor++;
		}
		cursor++;
		exercises++;
		priority[counter] += (float)str[cursor] - 48;
		
	}
	priority[counter] /= exercises;
	for (int i = 1; i <= counter; i++) {
		if (priority[i] < priority[imin]) imin = i;
	}
	fclose(List);
	list_id++;
	*list_id = imin + 1;
}

void block_finder(int block) {
	char str[STRSIZE] = { 0 };
	int cursor = 0;
	int counter = 0;
	while (counter < block) {
		fgets(str, STRSIZE, List);
		if (str[cursor] == '#') counter++;
	}
	return;
}

void list_finder(int* list_id) {
	char str[STRSIZE] = { 0 };
	int cursor = 0;
	int counter = 0;
	while (counter < *list_id) {
		fgets(str, STRSIZE, List);
		if (str[cursor] == '*') counter++;
	}
	return;
}

int get_min() {
	fseek(List, 0, SEEK_SET);
	char str[STRSIZE] = { 0 };
	int cursor = 0;
	int buf;
	while (!feof(List)) {
		fgets(str, STRSIZE, List);
		if (str[cursor] == '*' || str[cursor] == '#' || isdigit(str[cursor])) continue;
		while (str[cursor] != '>') {
			cursor++;
		}
		cursor++;
		buf = (float)str[cursor] - 48;
		if (buf == 0) return 0;
		cursor = 0;
	}
	return 1;
}