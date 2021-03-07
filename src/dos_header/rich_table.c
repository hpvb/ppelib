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

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "main.h"
#include "platform.h"
#include "ppe_error.h"

#include "ppelib_internal.h"

#include "rich_table.h"

#define CHECK_TABLE_INDEX if(table_index>=table->size){ppelib_set_error("Index out of range");return 0;}

EXPORT_SYM size_t ppelib_rich_table_get_size(const rich_table_t *table) {
	ppelib_reset_error();

	return table->size;
}

EXPORT_SYM uint16_t ppelib_rich_table_get_id(const rich_table_t *table, size_t table_index) {
	ppelib_reset_error();

	CHECK_TABLE_INDEX;

	return table->entries[table_index].id;
}

EXPORT_SYM uint16_t ppelib_rich_table_get_build_number(const rich_table_t *table, size_t table_index) {
	ppelib_reset_error();

	CHECK_TABLE_INDEX;

	return table->entries[table_index].build_number;
}

EXPORT_SYM uint32_t ppelib_rich_table_get_use_count(const rich_table_t *table, size_t table_index) {
	ppelib_reset_error();

	CHECK_TABLE_INDEX;

	return table->entries[table_index].use_count;
}

EXPORT_SYM void ppelib_rich_table_printf(FILE *stream, const rich_table_t *table) {
	ppelib_reset_error();

	for (size_t i = 0; i < table->size; ++i) {
		rich_table_entry_t *entry = &table->entries[i];
		fprintf(stream, "ID: 0x%04X, build_number: %u, use_count: %i\n", entry->id, entry->build_number,
				entry->use_count);
	}
}

EXPORT_SYM void ppelib_rich_table_print(const rich_table_t *table) {
	ppelib_rich_table_printf(stdout, table);
}

size_t find_rich_signature(uint8_t *buffer, size_t size) {
	if (size < 4) {
		goto out;
	}

	for (size_t i = 0; i < size - 3; ++i) {
		uint32_t header = read_uint32_t(buffer + i);
		if (header == RICH_MARKER) {
			return i;
		}
	}

	out: return size + 1;
}

uint8_t parse_rich_table(uint8_t *buffer, size_t size, rich_table_t *rich_table) {
	size_t footer_offset = find_rich_signature(buffer, size);

	if (footer_offset > size) {
		return 1;
	}

	if (footer_offset + 8 > size) {
		return 1;
	}

	if (footer_offset < 4) {
		return 1;
	}

	if (footer_offset > INT32_MAX) {
		return 1;
	}

	uint32_t key = read_uint32_t(buffer + footer_offset + 4);
	size_t rich_table_size_padded = 0;
	char dans_marker_found = 0;

	for (uint32_t i = (uint32_t) footer_offset - 4; i >= 4; i -= 4) {
		uint32_t value = read_uint32_t(buffer + i) ^ key;
		if (value == DANS_MARKER) {
			dans_marker_found = 1;
			break;
		}
		++rich_table_size_padded;
	}

	if (!dans_marker_found) {
		return 1;
	}

	size_t value_offset = footer_offset - (rich_table_size_padded * 4);
	size_t rich_table_size = rich_table_size_padded;

	for (size_t i = 0; i < rich_table_size_padded; ++i) {
		uint32_t value = read_uint32_t(buffer + value_offset + 0) ^ key;

		if (!value) {
			rich_table_size--;
		} else {
			break;
		}

		value_offset += 4;
	}

	rich_table_size /= 2;

	rich_table->entries = malloc(sizeof(rich_table_entry_t) * rich_table_size);
	if (!rich_table->entries) {
		return 1;
	}

	value_offset = footer_offset - (rich_table_size * 8);
	for (size_t i = 0; i < rich_table_size; ++i) {
		uint32_t id_value = read_uint32_t(buffer + value_offset + 0) ^ key;

		rich_table->entries[i].id = (uint16_t) ((id_value & 0xffff0000) >> 16);
		rich_table->entries[i].build_number = id_value & 0x0000ffff;
		rich_table->entries[i].use_count = read_uint32_t(buffer + value_offset + 4) ^ key;

		value_offset += 8;
	}

	rich_table->size = rich_table_size;
	return 0;
}
