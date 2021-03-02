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
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include "ppelib-internal.h"

thread_local size_t t_max_size;
thread_local uint32_t t_rscs_base;
thread_local uint8_t t_parse_error_handled;

typedef struct string_table_string {
	uint32_t offset;
	uint16_t bytes;

	const wchar_t *string;
} string_table_string_t;

typedef struct string_table {
	size_t size;
	size_t bytes;

	uint32_t base_offset;

	string_table_string_t *strings;
} string_table_t;

size_t put_string(const wchar_t *string, uint8_t *buffer);
void free_resource_directory_table(ppelib_resource_table_t *table);

void string_table_serialize(string_table_t *table, uint8_t *buffer) {
	for (size_t i = 0; i < table->size; ++i) {
		size_t offset = table->base_offset + table->strings[i].offset;

		const wchar_t *string = table->strings[i].string;
		size_t size = wcslen(string);

		printf("Writing string '%ls' at offset %zi\n", string, offset);
		if (size > UINT16_MAX) {
			ppelib_set_error("String too long");
			return;
		}

		write_uint16_t(buffer + offset, (uint16_t) size);

		if (sizeof(wchar_t) == 2) {
			memcpy(buffer + offset + 2, string, size * 2);
		} else {
			for (uint16_t i = 0; i < size; ++i) {
				memcpy(buffer + offset + 2 + (i * 2), string + i, 2);
			}
		}
	}
}

string_table_string_t* string_table_find(string_table_t *table, wchar_t *string) {
	for (size_t i = 0; i < table->size; ++i) {
		if (wcscmp(string, table->strings[i].string) == 0) {
			return &table->strings[i];
		}
	}

	return NULL;
}

void string_table_put(string_table_t *table, wchar_t *string) {
	if (string_table_find(table, string)) {
		return;
	}

	size_t s_size = (wcslen(string) * 2) + 2;

	if (s_size > UINT16_MAX) {
		ppelib_set_error("String size too long");
		return;
	}

	table->size++;
	table->bytes += s_size;

	table->strings = realloc(table->strings, sizeof(string_table_string_t) * table->size);
	table->strings[table->size - 1].string = string;
	table->strings[table->size - 1].bytes = (uint16_t) s_size;

	if (table->size > 1) {
		uint32_t prev_offset = table->strings[table->size - 2].offset;
		uint32_t prev_length = table->strings[table->size - 2].bytes;
		table->strings[table->size - 1].offset = prev_offset + prev_length;
	} else {
		table->strings[table->size - 1].offset = 0;
	}

	printf("RVA: %u string: '%ls' s_size : %zi\n", table->strings[table->size - 1].offset, string, s_size);
}

void string_table_free(string_table_t *table) {
	free(table->strings);
}

size_t table_length(const ppelib_resource_table_t *resource_table, size_t in_size) {
	size_t subdirs_size = 0;
	for (uint32_t i = 0; i < resource_table->subdirectories_number; ++i) {
		subdirs_size += table_length(resource_table->subdirectories[i], in_size);
	}

	size_t entries_size = (resource_table->subdirectories_number + resource_table->data_entries_number) * 8;

	return in_size + subdirs_size + 16 + entries_size;
}

size_t table_data_entries_number(const ppelib_resource_table_t *resource_table, size_t in_size) {
	size_t subdirs_size = 0;
	for (uint32_t i = 0; i < resource_table->subdirectories_number; ++i) {
		subdirs_size += table_data_entries_number(resource_table->subdirectories[i], in_size);
	}

	return in_size + subdirs_size + resource_table->data_entries_number;
}

size_t write_tables(const ppelib_resource_table_t *resource_table, uint8_t *buffer, size_t offset,
		string_table_t *string_table, size_t *data_entries_offset, size_t *data_offset, size_t rscs_base) {

	printf("write_tables\n");

	//size_t size = string_table_offset;
	size_t next_entry = 0;

	size_t number_of_name_entries = 0;
	size_t number_of_id_entries = 0;

	for (size_t i = 0; i < resource_table->subdirectories_number; ++i) {
		if (resource_table->subdirectories[i]->name) {
			number_of_name_entries++;
		} else {
			number_of_id_entries++;
		}
	}

	for (size_t i = 0; i < resource_table->data_entries_number; ++i) {
		if (resource_table->data_entries[i]->name) {
			number_of_name_entries++;
		} else {
			number_of_id_entries++;
		}
	}

	if (number_of_name_entries > UINT16_MAX) {
		ppelib_set_error("Too many name entries");
		return 0;
	}

	if (number_of_id_entries > UINT16_MAX) {
		ppelib_set_error("Too many id entries");
		return 0;
	}

	uint8_t *table = buffer + offset;

	write_uint32_t(table + 0, resource_table->characteristics);
	write_uint32_t(table + 4, resource_table->time_date_stamp);
	write_uint16_t(table + 8, resource_table->major_version);
	write_uint16_t(table + 10, resource_table->minor_version);
	write_uint16_t(table + 12, (uint16_t) number_of_name_entries);
	write_uint16_t(table + 14, (uint16_t) number_of_id_entries);

	offset += 16;

	next_entry = offset + ((number_of_name_entries + number_of_id_entries) * 8);

	printf("Going to write %zi subdirs\n", resource_table->subdirectories_number);
	for (size_t i = 0; i < resource_table->subdirectories_number; ++i) {

		if (next_entry > UINT32_MAX) {
			ppelib_set_error("Sub-directory offset out of range");
			return 0;
		}

		const ppelib_resource_table_t *t = resource_table->subdirectories[i];
		uint8_t *entry = buffer + offset;

		uint32_t name_offset_or_id = 0;
		uint32_t entry_offset = (uint32_t) next_entry;

		printf("table_length: %zi\n", table_length(t, 0));
		ppelib_print_resource_table(t);
		printf("\n");

		next_entry += table_length(t, 0);

		if (t->name) {
			string_table_string_t *string = string_table_find(string_table, t->name);
			name_offset_or_id = (string_table->base_offset + string->offset) ^ HIGH_BIT32;
		} else {
			name_offset_or_id = t->resource_type;
		}

		write_uint32_t(entry + 0, name_offset_or_id);
		write_uint32_t(entry + 4, entry_offset ^ HIGH_BIT32);

		write_tables(t, buffer, entry_offset, string_table, data_entries_offset, data_offset, rscs_base);
		offset += 8;
	}

	for (uint32_t i = 0; i < resource_table->data_entries_number; ++i) {
		printf("Writing data entry\n");

		if (*data_entries_offset > UINT32_MAX) {
			ppelib_set_error("Data entry offset out of range");
			return 0;
		}

		const ppelib_resource_data_t *d = resource_table->data_entries[i];
		uint8_t *entry = buffer + offset;

		uint32_t name_offset_or_id = 0;
		uint32_t entry_offset = (uint32_t) *data_entries_offset;

		if (d->name) {
			string_table_string_t *string = string_table_find(string_table, d->name);
			name_offset_or_id = (string_table->base_offset + string->offset) ^ HIGH_BIT32;
		} else {
			name_offset_or_id = d->resource_type;
		}

		write_uint32_t(entry + 0, name_offset_or_id);
		write_uint32_t(entry + 4, entry_offset);

		write_uint32_t(buffer + entry_offset + 0, (uint32_t) rscs_base + (uint32_t) *data_offset);
		write_uint32_t(buffer + entry_offset + 4, d->size);
		write_uint32_t(buffer + entry_offset + 8, d->codepage);
		write_uint32_t(buffer + entry_offset + 12, d->reserved);

		memcpy(buffer + *data_offset, d->data, d->size);

		*data_entries_offset = entry_offset + 16;
		*data_offset = TO_NEAREST(*data_offset + d->size, 8);

		offset += 8;
		return 0;
	}

	return 0;
}

void fill_string_table(const ppelib_resource_table_t *resource_table, string_table_t *string_table) {
	if (resource_table->name) {
		string_table_put(string_table, resource_table->name);
	}

	for (size_t i = 0; i < resource_table->subdirectories_number; ++i) {
		fill_string_table(resource_table->subdirectories[i], string_table);
	}

	for (size_t i = 0; i < resource_table->data_entries_number; ++i) {
		if (resource_table->data_entries[i]->name) {
			string_table_put(string_table, resource_table->data_entries[i]->name);
		}
	}
}

size_t serialize_resource_table(const ppelib_resource_table_t *resource_table, uint8_t *buffer, size_t rscs_base) {
	ppelib_reset_error();

	size_t size = 0;

	size_t string_table_offset = table_length(resource_table, 0);

	string_table_t string_table = { 0 };
	if (string_table_offset > UINT32_MAX) {
		ppelib_set_error("String table too large");
		return 0;
	}

	string_table.base_offset = (uint32_t) string_table_offset;

	size_t data_entries = table_data_entries_number(resource_table, 0);
	fill_string_table(resource_table, &string_table);

	size_t data_entries_offset = TO_NEAREST(string_table_offset + string_table.bytes, 4);
	size_t data_offset = data_entries_offset + (data_entries * 16);

	if (buffer) {
		string_table_serialize(&string_table, buffer);
		write_tables(resource_table, buffer, 0, &string_table, &data_entries_offset, &data_offset, rscs_base);
	}

	string_table_free(&string_table);
	return size;
}

wchar_t* get_string(uint8_t *buffer, size_t offset) {
	if (offset + 2 > t_max_size) {
		ppelib_set_error("Section too small for string");
		t_parse_error_handled = 0;
		return NULL;
	}

	uint16_t size = read_uint16_t(buffer + offset + 0);

	if (offset + 2u + (size * 2u) > t_max_size) {
		ppelib_set_error("Section too small for string");
		t_parse_error_handled = 0;
		return NULL;
	}

	wchar_t *string = calloc((size + 1u) * sizeof(wchar_t), 1);
	if (!string) {
		ppelib_set_error("Failed to allocate string");
		t_parse_error_handled = 0;
		return NULL;
	}

	if (sizeof(wchar_t) == 2) {
		memcpy(string, buffer + 2u, size * 2u);
	} else {
		for (uint16_t i = 0; i < size; ++i) {
			memcpy(string + i, buffer + offset + 2 + (i * 2), 2);
		}
	}

	return string;
}

size_t parse_data_entry(ppelib_resource_data_t *data_entry, uint8_t *buffer, size_t offset) {
	uint32_t data_rva = read_uint32_t(buffer + offset + 0) - t_rscs_base;
	data_entry->size = read_uint32_t(buffer + offset + 4);
	data_entry->codepage = read_uint32_t(buffer + offset + 8);
	data_entry->reserved = read_uint32_t(buffer + offset + 12);

	if (data_rva > t_max_size || data_entry->size > t_max_size || data_rva + data_entry->size > t_max_size) {
		ppelib_set_error("Section too small for resource data entry data");
		t_parse_error_handled = 0;
		return 0;
	}

	data_entry->data = malloc(data_entry->size);
	if (!data_entry->data) {
		ppelib_set_error("Failed to allocate resource data");
		t_parse_error_handled = 0;
		return 0;
	}

	memcpy(data_entry->data, buffer + data_rva, data_entry->size);

	return data_rva + data_entry->size;
}

size_t parse_directory_table(ppelib_resource_table_t *resource_table, uint8_t *buffer, size_t offset, size_t depth) {
	depth++;

	if (depth > 10) {
		ppelib_set_error("Parse depth (10) exceeded");
		t_parse_error_handled = 0;
		return 0;
	}

	size_t size = 0;

	uint8_t *table = buffer + offset;

	resource_table->characteristics = read_uint32_t(table + 0);
	resource_table->time_date_stamp = read_uint32_t(table + 4);
	resource_table->major_version = read_uint16_t(table + 8);
	resource_table->minor_version = read_uint16_t(table + 10);

	uint16_t number_of_name_entries = read_uint16_t(table + 12);
	uint16_t number_of_id_entries = read_uint16_t(table + 14);

	size_t min_space = ((uint32_t) (number_of_name_entries + number_of_id_entries)) * 8u;

	if (offset + 16 + min_space > t_max_size) {
		ppelib_set_error("Section too small for resource table (no space for directory contents)");
		t_parse_error_handled = 0;
		return 0;
	}

	uint8_t *entries = table + 16;
	for (uint16_t i = 0; i < number_of_name_entries + number_of_id_entries; ++i) {

		uint32_t name_offset_or_id = read_uint32_t(entries + 0);
		uint32_t entry_offset = read_uint32_t(entries + 4);
		entries += 8;

		wchar_t *name = NULL;
		if (CHECK_BIT(name_offset_or_id, HIGH_BIT32)) {
			name = get_string(buffer, name_offset_or_id ^ HIGH_BIT32);
			if (ppelib_error_peek()) {
				return 0;
			}
		}

		if (CHECK_BIT(entry_offset, HIGH_BIT32)) {
			entry_offset = entry_offset ^ HIGH_BIT32;

			if (offset + 16 + entry_offset + 16 > t_max_size) {
				free(name);
				ppelib_set_error("Section too small for sub-directory");
				t_parse_error_handled = 0;
				return 0;
			}

			resource_table->subdirectories_number++;
			size_t subdirs = resource_table->subdirectories_number;

			void *oldptr = resource_table->subdirectories;
			resource_table->subdirectories = realloc(resource_table->subdirectories, sizeof(void*) * subdirs);
			if (!resource_table->subdirectories) {
				resource_table->subdirectories = oldptr;
				resource_table->subdirectories_number--;
				ppelib_set_error("Failed to allocate resource sub-directory entry");
				return 0;
			}

			resource_table->subdirectories[subdirs - 1] = calloc(sizeof(ppelib_resource_table_t), 1);
			ppelib_resource_table_t *subdir = resource_table->subdirectories[subdirs - 1];

			if (!subdir) {
				ppelib_set_error("Failed to allocate resource sub-directory entry");
				t_parse_error_handled = 0;
				return 0;
			}

			if (name) {
				subdir->name = name;
			} else {
				subdir->resource_type = name_offset_or_id;
			}

			size_t subdir_size = parse_directory_table(subdir, buffer, entry_offset, depth);
			if (ppelib_error_peek()) {
				if (!t_parse_error_handled) {
					t_parse_error_handled = 1;

					free(name);
					resource_table->subdirectories_number--;
					subdirs = resource_table->subdirectories_number;

					free_resource_directory_table(resource_table->subdirectories[subdirs]);
					free(resource_table->subdirectories[subdirs]);

					resource_table->subdirectories = realloc(resource_table->subdirectories, sizeof(void*) * subdirs);
				}
				return 0;
			}

			size = MAX(size, subdir_size);

		} else {
			if (offset + 16 + entry_offset + 16 > t_max_size) {
				free(name);
				ppelib_set_error("Section too small for data entry");
				t_parse_error_handled = 0;
				return 0;
			}

			resource_table->data_entries_number++;
			size_t datas = resource_table->data_entries_number;

			void *oldptr = resource_table->data_entries;
			resource_table->data_entries = realloc(resource_table->data_entries, sizeof(void*) * datas);

			if (!resource_table->data_entries) {
				resource_table->data_entries = oldptr;
				resource_table->data_entries_number--;
				ppelib_set_error("Failed to allocate resource data entry");
				return 0;
			}

			resource_table->data_entries[datas - 1] = calloc(sizeof(ppelib_resource_data_t), 1);
			ppelib_resource_data_t *data_entry = resource_table->data_entries[datas - 1];
			if (!data_entry) {
				ppelib_set_error("Failed to allocate resource data entry");
				return 0;
			}

			if (name) {
				data_entry->name = name;
			} else {
				data_entry->resource_type = name_offset_or_id;
			}

			size_t data_size = parse_data_entry(data_entry, buffer, entry_offset);
			if (ppelib_error_peek()) {
				if (!t_parse_error_handled) {
					t_parse_error_handled = 1;

					free(name);
					resource_table->data_entries_number--;
					free(resource_table->data_entries[resource_table->data_entries_number]);
					resource_table->data_entries = realloc(resource_table->data_entries,
							sizeof(void*) * resource_table->data_entries_number);

					return 0;
				}
			}

			size = MAX(size, data_size);
		}
	}

	return size;
}

size_t parse_resource_table(ppelib_file_t *pe) {
	ppelib_reset_error();
	t_parse_error_handled = 0;

	if (pe->header.number_of_rva_and_sizes < DIR_RESOURCE_TABLE) {
		ppelib_set_error("No resource table found (too few directory entries).");
	}

	ppelib_section_t *section = pe->data_directories[DIR_RESOURCE_TABLE].section;
	size_t table_offset = pe->data_directories[DIR_RESOURCE_TABLE].offset;
	size_t table_size = pe->data_directories[DIR_RESOURCE_TABLE].size;

	if (!table_size) {
		ppelib_set_error("No resource table found. (no size)");
		return 0;
	}

	if (!section) {
		ppelib_set_error("Resource table not in section");
		return 0;
	}

	size_t data_size = MIN(section->virtual_size, section->size_of_raw_data);

	if (table_offset + table_size > data_size) {
		ppelib_set_error("Section too small for table. (offset + size too large)");
		return 0;
	}

	memset(&pe->resource_table, 0, sizeof(ppelib_resource_table_t));
	pe->resource_table.root = 1;
	uint8_t *data_table = section->contents + table_offset;

	if (table_offset + 16 > data_size) {
		ppelib_set_error("Section too small for table. (No room for directory table)");
		return 0;
	}

	t_max_size = data_size;
	if (pe->data_directories[DIR_RESOURCE_TABLE].orig_rva > UINT32_MAX) {
		ppelib_set_error("Data directory offset out of range");
		return 0;
	}

	t_rscs_base = (uint32_t) pe->data_directories[DIR_RESOURCE_TABLE].orig_rva;

	return parse_directory_table(&pe->resource_table, data_table, 0, 0);
}

void free_resource_directory_table(ppelib_resource_table_t *table) {
	for (size_t i = 0; i < table->data_entries_number; ++i) {
		free(table->data_entries[i]->data);
		free(table->data_entries[i]->name);
		free(table->data_entries[i]);
	}

	free(table->data_entries);
	table->data_entries_number = 0;

	for (size_t i = 0; i < table->subdirectories_number; ++i) {
		free_resource_directory_table(table->subdirectories[i]);
		free(table->subdirectories[i]->name);
		free(table->subdirectories[i]);
	}

	free(table->subdirectories);
	table->subdirectories_number = 0;
}

void free_resource_directory(ppelib_file_t *pe) {

	free_resource_directory_table(&pe->resource_table);
}

void print_resource_directory_data(const ppelib_resource_data_t *data, uint32_t indent) {
	for (uint32_t i = 0; i < indent; ++i) {
		printf(" ");
	}

	if (data->name) {
		printf("Data: Name(%ls) Size(%i) Codepage(0x%08X (%s))\n", data->name, data->size, data->codepage,
				map_lookup(data->codepage, ppelib_charsets_types_map));
	} else {
		printf("Data: Type(0x%08X: (%s)) Size(%i) Codepage(0x%08X (%s))\n", data->resource_type,
				map_lookup(data->resource_type, ppelib_resource_types_map), data->size, data->codepage,
				map_lookup(data->codepage, ppelib_charsets_types_map));
	}
}

void print_resource_table(const ppelib_resource_table_t *table, uint32_t indent) {
	for (uint32_t i = 0; i < indent; ++i) {
		printf(" ");
	}

	if (table->name) {
		printf("Dir: Name(%ls) subdirs(%u) data_entries(%u)\n", table->name, (uint16_t) table->subdirectories_number,
				(uint16_t) table->data_entries_number);
	} else {
		printf("Dir: Type(0x%08X: (%s)) subdirs(%u) data_entries(%u)\n", table->resource_type,
				map_lookup(table->resource_type, ppelib_resource_types_map), (uint16_t) table->subdirectories_number,
				(uint16_t) table->data_entries_number);
	}

	for (size_t i = 0; i < table->subdirectories_number; ++i) {
		print_resource_table(table->subdirectories[i], indent + 2u);
	}

	for (size_t i = 0; i < table->data_entries_number; ++i) {
		print_resource_directory_data(table->data_entries[i], indent + 2u);
	}
}

EXPORT_SYM ppelib_resource_table_t* ppelib_get_resource_table(ppelib_file_t *pe) {
	ppelib_reset_error();

	return &pe->resource_table;
}

EXPORT_SYM void ppelib_print_resource_table(const ppelib_resource_table_t *resource_table) {
	ppelib_reset_error();

	print_resource_table(resource_table, 0);
}
