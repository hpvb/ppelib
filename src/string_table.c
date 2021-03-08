/* Copyright 2021 Hein-Pieter van Braam-Stewart
 *
 * This file is part of ppelib (Portable Portable Executable LIBrary)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <ctype.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "platform.h"
#include "ppe_error.h"

#include "string_table_private.h"

#include "ppelib_internal.h"

const char *string_table_get(string_table_t *string_table, size_t offset) {
	if (offset > string_table->highest_offset) {
		ppelib_set_error("Offset out of range");
		return NULL;
	}

	return string_table->strings + offset;
}

void string_table_free(string_table_t *string_table) {
	if (!string_table) {
		return;
	}

	free(string_table->strings);
}

void parse_string_table(const uint8_t *buffer, size_t size, size_t offset, string_table_t *string_table) {
	if (offset >= size) {
		ppelib_set_error("String table offset past size");
		return;
	}

	string_table->size = 0;
	string_table->numb_strings = 0;
	string_table->highest_offset = 0;

	if (offset + 4 > size) {
		ppelib_set_error("Failed to read string table\n");
		return;
	}

	size_t string_table_size = read_uint32_t(buffer + offset);
	printf("String table size: %zi\n", string_table_size);
	if (string_table_size < 5) {
		return;
	}

	if (string_table_size > size || offset + string_table_size > size) {
		ppelib_set_error("Not enough space for string table\n");
		return;
	}

	const uint8_t *strings = buffer + offset + 4;
	string_table->size = string_table_size - 4;

	string_table->strings = malloc(string_table->size);
	if (!string_table->strings) {
		ppelib_set_error("Failed to allocate string table\n");
		return;
	}

	size_t last_found = 0;
	for (size_t i = 0; i < string_table->size; ++i) {
		if (strings[i] == '0') {
			++string_table->numb_strings;
			string_table->highest_offset = last_found;
			last_found = i + 1;
		}
	}

	memcpy(string_table->strings, buffer + offset, string_table->size);
}
