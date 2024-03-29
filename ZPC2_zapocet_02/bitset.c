﻿/* Vojtecn Netrh R21412 - may 2022 */

#include <stdio.h>
#include <stdlib.h>
#include "bitset.h"

#define MAX_NUM_DIGITS 20
#define MAX_LINE_LEN 500

#define edit_size(size) (((size) + ((size) % CHAR_BIT)) / CHAR_BIT)

void free_bitset(Bitset* set) {
	for (int i = 0; i < set->size; i++) {
		free(set->set[i]);
		set->set[i] = NULL;
	}
	free(set->size);
	free(set->relative_size);
}

Bitset* create_bitset(size_t size) {
	Bitset* new_set = (Bitset*)malloc(sizeof(Bitset));
	check_alloc(new_set);
	if (!new_set) {
		free_bitset(new_set);
		return NULL;
	}

	size_t edited_size = edit_size(size);

	new_set->set = (char*)malloc(edited_size * sizeof(char));
	if (!new_set->set) {
		free_bitset(new_set);
		return NULL;
	}

	for (int i = 0; i < edited_size; i++) {
		new_set->set[i] = 0;
	}

	new_set->size = size;
	new_set->relative_size = edited_size;
	return new_set;
}

Bitset* create_bitset_with_values(size_t size, const int* values, size_t array_size) {
	Bitset* new_set = create_bitset(size);
	if (!new_set) {
		free_bitset(new_set);
		return NULL;
	}

	int end_pos = 0;
	for (int j = 0; j < new_set->relative_size; j++) {
		int start_element = (j * 8);
		if (start_element != 0) {
			start_element--;
		}

		for (int i = end_pos; i < array_size; i++) {
			int mask = 1 << (values[i] - (8 * j));
			new_set->set[j] = (new_set->set[j] & ~mask) | (1 << (values[i] - (8 * j)));

			if (values[i + 1] > (8 * (j + 1) - 1)) {
				end_pos = i + 1;
				break;
			}
		}
	}

	return new_set;
}

Bitset* create_bitset_with_range(size_t size, int upto) {
	Bitset* new_set = create_bitset(size);
	if (!new_set) {
		free_bitset(new_set);
		return NULL;
	}

	int end_pos = 0;
	int sequence = 0;

	for (int j = 0; j < new_set->relative_size; j++) {
		int start_element = (j * 8);
		if (start_element != 0) {
			start_element--;
		}

		for (int i = end_pos; i < 8; i++) {
			int mask = 1 << (sequence - (8 * j));
			new_set->set[j] = (new_set->set[j] & ~mask) | (1 << (sequence - (8 * j)));

			if (sequence + 1 >= upto) {
				end_pos = i + 1;
				return new_set;
			}
			sequence++;
		}
	}

	return new_set;
}

void set_element_change(Bitset* bitset, int element, int binary_value) {
	int pos_in_array = element / CHAR_BIT;
	int relative_pos = element % CHAR_BIT;

	int mask = 1 << (relative_pos);
	bitset->set[pos_in_array] = (bitset->set[pos_in_array] & ~mask) | (binary_value << relative_pos);
}

void set_insert(Bitset* bitset, int element) {
	set_element_change(bitset, element, 1);
}

void set_remove(Bitset* bitset, int element) {
	set_element_change(bitset, element, 0);
}

int contains(Bitset* bitset, int element) {
	int pos_in_array = element / CHAR_BIT;
	int relative_pos = element % CHAR_BIT;

	if (((bitset->set[pos_in_array] >> relative_pos) & 1U) == 1) {
		return 1;
	}
	else {
		return 0;
	}
}

int set_expand(Bitset* set, size_t new_size, int start) {
	char* temp = (char*)realloc(set->set, new_size);
	if (!temp) {
		free(temp);
		return 1;
	}
	set->set = temp;

	for (int i = start; i < set->relative_size; i++) {
		set->set[i] = 0;
	}
	return 0;
}

char bitwise_and(char a, char b) {
	return (a & b);
}

char bitwise_or(char a, char b) {
	return (a | b);
}

char bitwise_substraction(char a, char b) {
	return ((~b & a) & (b ^ a));
}

int bitwise_subset_test(char a, char b) {
	/* A is a subset of B (alternatively we can use '(a & b) == a) */
	if (bitwise_or(a, b) == b) {
		return 1;
	}
	else {
		return 0;
	}
}

void form_operation(Bitset* left, Bitset* right, char (*bitwise_operation)(char, char)) {
	for (int i = 0; i < right->relative_size; i++) {
		left->set[i] = bitwise_operation(left->set[i], right->set[i]);
	}
}

void form_intersection(Bitset* left, Bitset* right) {
	if (left->size < right->size) {
		int start = right->relative_size - left->relative_size;

		left->size = right->size;
		left->relative_size = right->relative_size;

		int expand = set_expand(left, left->relative_size, start);
		if (expand == 1) {
			return;
		}
	}

	form_operation(left, right, bitwise_and);
}

Bitset* set_operation(Bitset* left, Bitset* right, size_t new_size, size_t new_edited_size, char (*bitwise_operation)(char, char)) {
	int* values = (int*)malloc(new_edited_size * sizeof(int));
	if (!values) {
		free(values);
		return NULL;
	}

	int arr_size = 0;
	for (int i = 0; i < new_edited_size; i++) {
		char temp_left = left->set[i];
		char temp_right = right->set[i];
		for (int j = 0; j < CHAR_BIT; j++) {
			if (bitwise_operation((temp_left & 0x01), (temp_right & 0x01))) {
				values[arr_size] = (i * 8) + j;
				arr_size++;
			}
			temp_left = temp_left >> 1;
			temp_right = temp_right >> 1;
		}
	}

	return create_bitset_with_values(new_size, values, arr_size);
}

Bitset* set_intersection(Bitset* left, Bitset* right) {
	size_t new_size = min(left->size, right->size);
	size_t new_edited_size = edit_size(new_size);

	return set_operation(left, right, new_size, new_edited_size, bitwise_and);
}

void form_union(Bitset* left, Bitset* right) {
	form_operation(left, right, bitwise_or);
}

Bitset* set_union(Bitset* left, Bitset* right) {
	size_t new_size = max(left->size, right->size);
	size_t new_edited_size = edit_size(new_size);

	return set_operation(left, right, new_size, new_edited_size, bitwise_or);
}

void form_difference(Bitset* left, Bitset* right) {
	return form_operation(left, right, bitwise_substraction);
}

Bitset* set_difference(Bitset* left, Bitset* right) {
	size_t new_size = left->size;
	size_t new_edited_size = edit_size(new_size);

	return set_operation(left, right, new_size, new_edited_size, bitwise_substraction);
}

int is_part_empty(Bitset* set, size_t start) {
	for (size_t i = start; i < set->relative_size; i++) {
		if (set->set[i] != 0) {
			return 0;
		}
	}
	return 1;
}

int is_subset(Bitset* left, Bitset* right) {
	size_t i = 0;
	while (i < left->relative_size) {
		if (bitwise_subset_test(left->set[i], right->set[i]) == 0) {
			return 0;
		}
		if ((i + 1) > right->relative_size && left->relative_size > right->relative_size) {
			if (is_part_empty(left, i + 1)) {
				return 1;
			}
			else {
				return 0;
			}
		}
		i++;
	}

	return 1;
}

int digits_of_int(int number) {
	int count = 1;
	number = number / 10;

	while (number != 0)
	{
		number = number / 10;
		count++;
	}

	return count;
}

int error_print(int err_type) {
		switch (err_type) {
		case 1:
			printf("error %i: The file could not be opened", err_type);
			return 1;
		case 2:
			printf("error %i: Could not write values ​​to file", err_type);
			return 2;
		case 3:
			printf("error %i: The file could not be closed", err_type);
			return 3;
		default:
			printf("error 4: Unspecified problem");
			return 4;
	}
}

int save_bitsets_to_file(char* file, Bitset** bitsets, size_t bitsets_count) {
	FILE* new_file = fopen(file, "w");
	if (!new_file) {
		error_print(1);
	}

	for (int i = 0; i < bitsets_count; i++) {
		Bitset* current_bitset = bitsets[i];
		for (int j = 0; j < current_bitset->relative_size; j++) {
			char temp = current_bitset->set[j];
			for (int k = 0; k < CHAR_BIT; k++) {
				if ((temp & 0x01) == 1) {
					int num_to_write = (j * 8) + k;
					char* converted_num = (char*)malloc(sizeof(char) * digits_of_int(num_to_write));
					if (!converted_num) {
						error_print(2);
					}
					itoa(num_to_write, converted_num, 10);
					fputs(converted_num, new_file);
					fputc(' ', new_file);
				}
				temp = temp >> 1;
			}
		}
		fprintf(new_file, "\n");
	}

	if (fclose(new_file) == EOF) {
		error_print(3);
	}

	return 0;
}

int* convert_line_to_int(char* current_line, int* numbers) {
	int i = 0, pos = 0, j = 0;
	char temp[MAX_NUM_DIGITS];
	for (int i = 0; i < strlen(current_line); i++) {
		if (current_line[i] != ' ' && current_line[i] != '\n') {
			temp[j] = current_line[i];
			j++;
		}
		else {
			if (j == 1) {
				numbers[pos] = temp[j - 1] - '0';
			}
			else {
				numbers[pos] = atoi(temp);
			}
			pos++;
			j = 0;
		}
	}

	Help_nums* result = (Help_nums*)malloc(sizeof(Help_nums));
	if (!result) {
		free(result);
		return NULL;
	}
	result->numbers = numbers;
	result->pos = pos;

	return result;
}

void extend_arr(Bitset** arr, int new_size) {
	Bitset** temp = realloc(arr, new_size);
	if (!temp) {
		free(temp);
		return NULL;
	}
	arr = temp;
}

Bitset** load_bitsets(char* file) {
	FILE* read_file = fopen(file, "r");
	if (!read_file) {
		return NULL;
	}

	char* line = (char*)malloc(sizeof(char) * MAX_LINE_LEN);
	if (!line) {
		free(line);
		return NULL;
	}

	Bitset** result_arr = (Bitset*)malloc(sizeof(Bitset*) * 10);
	if (!result_arr) {
		free(result_arr);
		return NULL;
	}

	int line_count = 0;
	while (fgets(line, MAX_LINE_LEN, read_file) != NULL) {
		int* numbers = (int*)malloc(sizeof(int) * (strlen(line) / 2));
		if (!numbers) {
			free(numbers);
			return NULL;
		}

		Help_nums* temp = convert_line_to_int(line, numbers);
		if (temp->pos != 0) {
			result_arr[line_count] = create_bitset_with_values(numbers[temp->pos - 1] + 1, numbers, temp->pos);
			free(numbers);
		}

		line_count++;
		if (line_count > 10) {
			extend_arr(result_arr, line_count * 2);
		}
	}
	if (fclose(read_file) == EOF) {
		return NULL;
	}
	
	return result_arr;
}