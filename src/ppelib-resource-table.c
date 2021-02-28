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

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <ppelib/ppelib-section.h>
#include <ppelib/ppelib-resource-table.h>

#include "main.h"
#include "ppelib-error.h"
#include "export.h"
#include "utils.h"

_Thread_local size_t max_size;
_Thread_local size_t rscs_base;

size_t serialize_resource_table(const ppelib_resource_table_t *resource_table, uint8_t *buffer) {
	ppelib_reset_error();

}

wchar_t* get_string(uint8_t *buffer, size_t offset) {
	if (offset + 2 > max_size) {
		ppelib_set_error("Section too small for string");
		return NULL;
	}

	uint16_t size = read_uint16_t(buffer + offset + 0);

	if (offset + size + 2 > max_size) {
		ppelib_set_error("Section too small for string");
		return NULL;
	}

	wchar_t *string = calloc((size + 1) * sizeof(wchar_t), 1);
	if (!string) {
		ppelib_set_error("Failed to allocate string");
		return NULL;
	}

	if (sizeof(wchar_t) == 2) {
		memcpy(string, buffer + 2, size * 2);
	} else {
		for (uint16_t i = 0; i < size; ++i) {
			string[i] = *(uint16_t*) (buffer + offset + 2 + (i * 2));
		}
	}

	return string;
}

size_t parse_data_entry(ppelib_resource_data_t *data_entry, uint8_t *buffer, size_t offset) {
	uint32_t data_rva = read_uint32_t(buffer + offset + 0) - rscs_base;
	data_entry->size = read_uint32_t(buffer + offset + 4);
	data_entry->codepage = read_uint32_t(buffer + offset + 8);
	data_entry->reserved = read_uint32_t(buffer + offset + 12);

	if (data_rva > max_size || data_entry->size > max_size || data_rva + data_entry->size > max_size) {
		ppelib_set_error("Section too small for resource data entry");
		return 0;
	}

	data_entry->data = malloc(data_entry->size);
	if (!data_entry->data) {
		ppelib_set_error("Failed to allocate resource data");
		return 0;
	}

	memcpy(data_entry->data, buffer + data_rva, data_entry->size);

	return data_rva + data_entry->size;
}

size_t parse_directory_table(ppelib_resource_table_t *resource_table, uint8_t *buffer, size_t offset, size_t depth) {
	depth++;

	if (depth > 10) {
		ppelib_set_error("Parse depth (10) exceeded");
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

	size_t min_space = (number_of_name_entries + number_of_id_entries) * 8;

	if (offset + 16 + min_space > max_size) {
		ppelib_set_error("Section too small for resource table (no space for directory contents)");
		return 0;
	}

	uint8_t *entries = table + 16;
	for (uint16_t i = 0; i < number_of_name_entries + number_of_id_entries; ++i) {

		uint32_t name_offset_or_id = read_uint32_t(entries + 0);
		uint32_t entry_offset = read_uint32_t(entries + 4);
		entries += 8;

		wchar_t *name = NULL;
		if (CHECK_BIT(name_offset_or_id, 1 << 31)) {
			name = get_string(buffer, name_offset_or_id ^ (1 << 31));
			if (ppelib_error_peek()) {
				return 0;
			}
		}

		if (CHECK_BIT(entry_offset, 1 << 31)) {
			entry_offset = entry_offset ^ (1 << 31);

			if (offset + 16 + entry_offset + 16 > max_size) {
				ppelib_set_error("Section too small for sub-directory");
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
				return 0;
			}

			if (name) {
				subdir->name = name;
			} else {
				subdir->resource_type = name_offset_or_id;
			}

			size_t subdir_size = parse_directory_table(subdir, buffer, entry_offset, depth);
			if (ppelib_error_peek()) {
				return 0;
			}

			size = MAX(size, subdir_size);

		} else {
			if (offset + 16 + entry_offset + 16 > max_size) {
				ppelib_set_error("Section too small for data entry");
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
				return 0;
			}

			size = MAX(size, data_size);
		}
	}

	return size;
}

size_t parse_resource_table(ppelib_file_t *pe) {
	ppelib_reset_error();

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

	max_size = data_size;
	rscs_base = pe->data_directories[DIR_RESOURCE_TABLE].orig_rva;

	return parse_directory_table(&pe->resource_table, data_table, 0, 0);
}

void free_resource_directory_table(ppelib_resource_table_t *table) {
	for (size_t i = 0; i < table->subdirectories_number; ++i) {
		free_resource_directory_table(table->subdirectories[i]);
		free(table->subdirectories[i]->name);
		free(table->subdirectories[i]);
	}

	free(table->subdirectories);
	table->subdirectories_number = 0;

	for (size_t i = 0; i < table->data_entries_number; ++i) {
		free(table->data_entries[i]->data);
		free(table->data_entries[i]->name);
		free(table->data_entries[i]);
	}

	free(table->data_entries);
	table->data_entries_number = 0;
}

void free_resource_directory(ppelib_file_t *pe) {

	free_resource_directory_table(&pe->resource_table);
}

void print_resource_directory_data(const ppelib_resource_data_t *data, uint16_t indent) {
	for (uint16_t i = 0; i < indent; ++i) {
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

void print_resource_table(const ppelib_resource_table_t *table, uint16_t indent) {
	for (uint16_t i = 0; i < indent; ++i) {
		printf(" ");
	}

	if (table->root) {
		printf("RootDir: subdirs(%li) data_entries(%li)\n", table->subdirectories_number, table->data_entries_number);
	} else {
		if (table->name) {
			printf("Dir: Name(%ls) subdirs(%li) data_entries(%li)\n", table->name, table->subdirectories_number,
					table->data_entries_number);
		} else {
			printf("Dir: Type(0x%08X: (%s)) subdirs(%li) data_entries(%li)\n", table->resource_type,
					map_lookup(table->resource_type, ppelib_resource_types_map), table->subdirectories_number,
					table->data_entries_number);
		}
	}

	for (size_t i = 0; i < table->subdirectories_number; ++i) {
		print_resource_table(table->subdirectories[i], indent + 2);
	}

	for (size_t i = 0; i < table->data_entries_number; ++i) {
		print_resource_directory_data(table->data_entries[i], indent + 2);
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
