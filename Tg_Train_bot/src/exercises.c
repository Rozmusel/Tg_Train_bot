#include <stdio.h>
#include <windows.h>
#include <locale.h>

#define STRSIZE 128	// Maximum length of accepted string
#define NUMSIZE 16	// Maximum length of the number to be read
#define EXRSIZE 16
#define EXERSISES 8
FILE* List = NULL;

void get_next_type(int* list_id);	// Returns the block number taking into account the usage history
int get_next_list(int* list_id);	// Returns the list number in the block with the least used exercises
void block_finder(int block);	// Finds and hovers the cursor over the desired block
void list_finder(int* list_id);

void get_list_id(int* list_id) {
	int elem = 0;
	get_next_type(list_id);
	elem = get_next_list(list_id);
}
void get_list(int* list_id, char exr_list[]) {
	memset(exr_list, '\0', 512);
	errno_t read_res = 0;	// Storing the result of opening a file
	read_res = fopen_s(&List, "List.txt", "r+");
	if (read_res != 0) return;
	int cursor = 0;
	int ecounter = 0;
	int scounter = 0;
	int lcounter = 0;
	char exercises[EXERSISES][STRSIZE];
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
	fclose(List);
}

void get_next_type(int* list_id) {
	errno_t read_res = 0;	// Storing the result of opening a file
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

int get_next_list(int* list_id) {
	errno_t read_res = 0;	// Storing the result of opening a file
	read_res = fopen_s(&List, "List.txt", "r+");
	if (read_res != 0) return -1;
	block_finder(*list_id);

	int cursor = 0;
	char str[STRSIZE] = { 0 };
	int priority[EXRSIZE] = { 0 };
	int counter = 0;
	int imin = 1;

	while (!feof(List)) {
		cursor = 0;
		fgets(str, STRSIZE, List);
		if (str[cursor] == '#') break;
		if (str[cursor] == '*') {
			counter++;
			fgets(str, STRSIZE, List);
		}
		while (str[cursor] != '>' && str[cursor] != '/') {
			cursor++;
		}
		if (str[cursor] == '/') continue;
		cursor++;
		priority[counter] += (int)str[cursor] - 48;
	}
	for (int i = 2; i <= counter; i++) {
		if (priority[i] < priority[imin]) imin = i;
	}
	fclose(List);
	list_id++;
	*list_id = imin;
	return counter;
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