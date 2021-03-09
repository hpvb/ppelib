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
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "platform.h"
#include "ppe_error.h"
#include "ppelib_internal.h"

#include "generated/section_private.h"

#include "import_table.h"

EXPORT_SYM void ppelib_import_table_print(import_table_t *import_table) {
	for (size_t i = 0; i < import_table->size; ++i) {
		ppelib_import_directory_table_print(&import_table->import_directory_tables[i]);
	}
}

void import_table_free(import_table_t *import_table) {
	for (size_t i = 0; i < import_table->size; ++i) {
		free(import_table->import_directory_tables[i].dll_name);
	}
	free(import_table->import_directory_tables);
}

void parse_import_table(const section_t *section, size_t offset, import_table_t *import_table) {
	if (section->contents_size < IMPORT_DIRECTORY_TABLE_SIZE * 2) {
		ppelib_set_error("Not enough space for directory table");
		return;
	}

	size_t scan_offset = offset;
	import_table->import_directory_tables = NULL;

	while (section->contents_size - scan_offset >= IMPORT_DIRECTORY_TABLE_SIZE) {
		import_directory_table_t import_directory_table;

		ppelib_import_directory_table_deserialize(section->contents, section->contents_size, scan_offset, &import_directory_table);
		if (ppelib_error_peek()) {
			return;
		}

		if (ppelib_import_directory_table_is_null(&import_directory_table)) {
			break; // null buffer
		}

		size_t dll_name_offset = section_rva_to_offset(section, import_directory_table.name_rva);
		if (ppelib_error_peek()) {
			return;
		}

		size_t dll_name_max_size = section->contents_size - dll_name_offset;
		size_t dll_name_size = strnlen((const char *)section->contents + dll_name_offset, dll_name_max_size);
		if (dll_name_size == dll_name_max_size) {
			ppelib_set_error("DLL name outside of section");
			return;
		}
		import_directory_table.dll_name = strdup((const char *)section->contents + dll_name_offset);

		++import_table->size;
		size_t new_size = sizeof(import_directory_table_t) * import_table->size;

		import_table->import_directory_tables = realloc(import_table->import_directory_tables, new_size);
		if (!import_table->import_directory_tables) {
			ppelib_set_error("Allocating import table directory failed");
			return;
		}

		memcpy(&import_table->import_directory_tables[import_table->size - 1], &import_directory_table, sizeof(import_directory_table_t));
		scan_offset += IMPORT_DIRECTORY_TABLE_SIZE;
	}
}
