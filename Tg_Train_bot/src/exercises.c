#include <stdio.h>
#include <windows.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>

#include "settings.h"

#define STRSIZE 128	// Maximum length of accepted string
#define NUMSIZE 8	// Maximum length of the number
#define EXRSIZE 16

FILE* List = NULL;
FILE* List_EDIT = NULL;

int get_next_block(int* list_id);	// The number of the next block is written to the passed array.
int get_next_list(int* list_id);	// The number of the next list is written to the passed array.
int read_list(wchar_t exr_list[], int list_id[]);
int get_exercises(wchar_t exercises[][STRSIZE], int* list_id);
int history_update(wchar_t exercises[][STRSIZE]);
int edit_check();
int is_key_str(wchar_t str[], wchar_t exercises[][STRSIZE]);
int num_read(wchar_t str[], int cursor);

void block_finder(int block);	// Finds and hovers the cursor over the desired block
void list_finder(int* list_id);

int get_list_id(int* list_id) {
	int result;

	result = get_next_block(list_id);
	if (result != 1) return result;

	result = get_next_list(list_id);
	if (result != 1) return result;

	return 1;
}

int get_list(int* list_id, wchar_t exr_list[]) {
	int result = read_list(exr_list, list_id);
	if (result != 1) return result;

	wchar_t exercises[EXERSISES][STRSIZE] = { { '\0' } };
	result = get_exercises(exercises, list_id);
	if (result != 1) return result;

	result = history_update(exercises);
	if (result != 1) return result;

	return 1;
}

int get_next_block(int* list_id) {
	errno_t read_res = 0;
	read_res = _wfopen_s(&List, L"List.txt", L"r+");
	if (read_res != 0) return -1;

	int num;
	int block = 0;
	int cursor = 0;
	int counter = 0;
	wchar_t str[STRSIZE] = { 0 };

	while (!feof(List)) {
		fgetws(str, STRSIZE, List);
		if (str[cursor] != '#') continue;
		counter++;
		if (counter >= 9) return 0;

		while (str[cursor] != '>') {
			(cursor)++;
		}
		cursor++;

		num = (int)str[cursor] - 48;
		if (num == 0) {
			block = counter;
			break;
		}

		cursor = 0;
	}

	*list_id = block;

	fclose(List);

	return 1;
}

int get_next_list(int* list_id) {
	errno_t read_res = 0;
	read_res = _wfopen_s(&List, L"List.txt", L"r+");
	if (read_res != 0) return -1;

	block_finder(*list_id);

	int cursor = 0;
	wchar_t str[STRSIZE] = { 0 };
	float priority[EXRSIZE] = { 0 };
	int counter = 0;
	int imin = 0;
	int exercises = 0;

	fgetws(str, STRSIZE, List);

	while (!feof(List)) {
		cursor = 0;
		fgetws(str, STRSIZE, List);

		if (str[cursor] == '#') break;
		if (isdigit(str[cursor])) continue;

		if (str[cursor] == '*') {
			fgetws(str, STRSIZE, List);
			priority[counter] /= exercises;
			counter++;
			exercises = 0;
		}

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
	return 1;
}

void block_finder(int block) {
	wchar_t str[STRSIZE] = { 0 };
	int cursor = 0;
	int counter = 0;
	while (counter < block) {
		fgetws(str, STRSIZE, List);
		if (str[cursor] == '#') counter++;
	}
	return;
}

void list_finder(int* list_id) {
	wchar_t str[STRSIZE] = { 0 };
	int cursor = 0;
	int counter = 0;
	while (counter < *list_id) {
		fgetws(str, STRSIZE, List);
		if (str[cursor] == '*') counter++;
	}
	return;
}


int edit_exr(wchar_t exr_list[], int list_id[], wchar_t* edit) {
	if (edit_check() == 0) return 0;
	errno_t read_res = 0;
	read_res = _wfopen_s(&List, L"List.txt", L"r+");
	if (read_res != 0) return -1;

	int exr = (int)edit[0] - 48;
	wchar_t str[STRSIZE] = { 0 };
	wchar_t new_str[STRSIZE] = { 0 };

	read_res = _wfopen_s(&List_EDIT, L"List_edit.txt", L"w");
	if (read_res != 0) return -1;
	int iblock = 0;
	int ilist = 0;
	int counter = 1;
	while (iblock < *list_id) {
		fgetws(str, STRSIZE, List);
		if (str[0] == '#') iblock++;
		fwprintf(List_EDIT, L"%s", str);
	}
	list_id++;
	while (ilist < *list_id) {
		fgetws(str, STRSIZE, List);
		if (str[0] == '*') ilist++;
		fwprintf(List_EDIT, L"%s", str);
	}
	fgetws(str, STRSIZE, List);
	fwprintf(List_EDIT, L"%s", str);
	fgetws(str, STRSIZE, List);
	while (counter < exr) {
		fwprintf(List_EDIT, L"%s", str);
		fgetws(str, STRSIZE, List);
		fwprintf(List_EDIT, L"%s", str);
		fgetws(str, STRSIZE, List);
		counter++;
	}
	for (int i = 0; i < STRSIZE; i++) {
		new_str[i] = edit[i + 3];
	}
	fputws(new_str, List_EDIT);
	fputwc('\n', List_EDIT);
	while(!feof(List)){
		fgetws(str, STRSIZE, List);
		fwprintf(List_EDIT, L"%s", str);
	}
	fclose(List);
	fclose(List_EDIT);
	remove("List.txt");
	if (rename("List_edit.txt", "List.txt") == 0) return -1;
	return 1;
}

int edit_check() {	// Здесь будет проверка на корректность введёных данных
	return 1;
}

int get_exercises(wchar_t exercises[][STRSIZE], int* list_id) {
	errno_t read_res = 0;
	read_res = _wfopen_s(&List, L"List.txt", L"r+");
	if (read_res != 0) return -1;


	wchar_t str[STRSIZE] = { 0 };
	int cursor = 0;
	int i = 0;
	int j = 0;

	block_finder(*list_id);
	list_id++;
	list_finder(list_id);

	while (!feof(List)) {
		fgetws(str, STRSIZE, List);
		if (str[cursor] == '*' || str[cursor] == '#') break;
		while (str[cursor] != '>') {
			exercises[i][j] = str[cursor];
			j++;
			cursor++;
		}
		fgetws(str, STRSIZE, List);

		i++;
		j = 0;
		cursor = 0;
	}

	fclose(List);

	return 1;
}

int read_list(wchar_t exr_list[], int list_id[]) {
	errno_t read_res = 0;
	read_res = _wfopen_s(&List, L"List.txt", L"r+");
	if (read_res != 0) return -1;

	int cursor = 0;	// Cursor for reading
	int counter = 0;
	wchar_t str[STRSIZE] = { 0 };

	block_finder(*list_id);
	list_id++;
	list_finder(list_id);
	
	while (!feof(List)) {
		fgetws(str, STRSIZE, List);
		if (str[cursor] == '*' || str[cursor] == '#') break;

		while (str[cursor] != '>') {	// Copy title
			exr_list[counter] = str[cursor];
			counter++;
			cursor++;
		}

		exr_list[counter] = '\n';
		counter++;
		cursor = 0;
		fgetws(str, STRSIZE, List);

		while (str[cursor] != '\n') {	// Copy sets
			exr_list[counter] = str[cursor];
			counter++;
			cursor++;
		}

		cursor = 0;
		exr_list[counter] = '\n';
		counter++;
	}

	fclose(List);
	return 1;
}

int history_update(wchar_t exercises[][STRSIZE]) {
	errno_t read_res = 0;
	read_res = _wfopen_s(&List, L"List.txt", L"r+");
	if (read_res != 0) return -1;

	read_res = _wfopen_s(&List_EDIT, L"List_edit.txt", L"w");
	if (read_res != 0) return -1;

	setlocale(LC_ALL, "rus");

	long max_pos = 0;
	wchar_t max = 0;
	int cursor = 0;
	wchar_t str[STRSIZE] = { 0 };
	int pos;

	while (!feof(List)) {
		fgetws(str, STRSIZE, List);

		if (str[cursor] == '#') {
			while (!isdigit(str[cursor])) {
				cursor++;
			}

			if (max < str[cursor]) {
				max = str[cursor];
				max_pos = ftell(List);
			}

			fseek(List, -3, SEEK_CUR);
			fputwc((int)str[cursor] + 1, List);	// block history update
			fseek(List, 3, SEEK_CUR);
			cursor = 0;
			continue;
		}
	}

	fseek(List, max_pos - 3, SEEK_SET);
	fputwc('0', List);

	fseek(List, 0, SEEK_SET);

	while (!feof(List)) {
		fgetws(str, STRSIZE, List);

		if (!(str[cursor] == '*' || str[cursor] == '#' || isdigit(str[cursor]))) {

			while (!isdigit(str[cursor])) {
				fputwc(str[cursor], List_EDIT);
				cursor++;
			}

			if (is_key_str(str, exercises)) {
				fputwc('0', List_EDIT);
				fputwc('\n', List_EDIT);

			} else {
				int num = num_read(str, cursor);
				num++;
				
				wchar_t snum[NUMSIZE];
				_itow_s(num, snum, NUMSIZE, 10);
				fputws(snum, List_EDIT);
				fputwc('\n', List_EDIT);
			}
			continue;
		}

		fputws(str, List_EDIT);
	}

	fclose(List);
	fclose(List_EDIT);
	remove("List.txt");
	if (rename("List_edit.txt", "List.txt") == 0) return -1;

	return 1;
}

int is_key_str(wchar_t str[], wchar_t exercises[][STRSIZE]) {
	int flag = 0;

	for (int i = 0; exercises[i][0] != '\0'; i++) {
		flag = 1;

		for (int j = 0; str[j] != '>'; j++) {
			if (exercises[i][j] != str[j]) {
				flag = 0;
				break;
			}
		}

		if (flag == 1) break;
	}

	return flag;
}

int num_read(wchar_t str[], int cursor) {
	wchar_t cnum[NUMSIZE] = { '\0' };
	
	for (int i = 0; str[cursor] != '\n'; i++) {
		cnum[i] = str[cursor];
		cursor++;
	}

	return _wtoi(cnum);
}