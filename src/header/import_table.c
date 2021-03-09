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

EXPORT_SYM void ppelib_import_table_fprint(FILE *stream, import_table_t *import_table) {
	ppelib_reset_error();

	for (size_t i = 0; i < import_table->size; ++i) {
		fprintf(stream, "DLL name: %s, entries: %zi\n", import_table->entries[i].dll_name, import_table->entries[i].size);

		for (size_t l = 0; l < import_table->entries[i].size; ++l) {
			if (import_table->entries[i].names[l].name) {
				fprintf(stream, "  Name: %s, hint: 0x%04X\n", import_table->entries[i].names[l].name, import_table->entries[i].names[l].hint);
			} else {
				fprintf(stream, "  Ordinal: 0x%04X\n", import_table->entries[i].names[l].ordinal);
			}
		}
	}
}

EXPORT_SYM void ppelib_import_table_print(import_table_t *import_table) {
	ppelib_import_table_fprint(stdout, import_table);
}

void import_table_entry_free(import_table_entry_t *entry) {
	for (size_t i = 0; i < entry->size; ++i) {
		free(entry->names[i].name);
	}

	free(entry->dll_name);
	free(entry->names);
}

void import_table_free(import_table_t *import_table) {
	for (size_t i = 0; i < import_table->size; ++i) {
		import_table_entry_free(&import_table->entries[i]);
	}

	free(import_table->entries);
}

void parse_import_table(const section_t *section, size_t offset, import_table_t *import_table, uint16_t magic) {
	if (section->contents_size == IMPORT_DIRECTORY_TABLE_SIZE) {
		// Empty table
		return;
	}

	if (magic != PE32_MAGIC && magic != PE32PLUS_MAGIC) {
		ppelib_set_error("Unknown magic value");
		return;
	}

	size_t scan_offset = offset;
	import_table->entries = NULL;

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

		++import_table->size;
		size_t new_size = sizeof(import_table_entry_t) * import_table->size;

		import_table->entries = realloc(import_table->entries, new_size);
		if (!import_table->entries) {
			ppelib_set_error("Allocating import table directory failed");
			return;
		}

		import_table_entry_t *entry = &import_table->entries[import_table->size - 1];
		memset(entry, 0, sizeof(import_table_entry_t));

		size_t ilt_offset = section_rva_to_offset(section, import_directory_table.import_address_table_rva);

		size_t il_stride = 4;
		if (magic == PE32PLUS_MAGIC) {
			il_stride = 8;
		}

		while (1) {
			if (ilt_offset + il_stride > section->contents_size) {
				ppelib_set_error("Import Lookup Table outside of section");
				return;
			}

			uint64_t il;
			if (il_stride == 4) {
				il = read_uint32_t(section->contents + ilt_offset);
			} else {
				il = read_uint64_t(section->contents + ilt_offset);
			}

			if (!il) {
				break;
			}

			++entry->size;
			entry->names = realloc(entry->names, sizeof(import_table_name_t) * entry->size);
			if (!entry->names) {
				ppelib_set_error("Failed to allocate import entry name");
				goto out;
			}
			import_table_name_t *sym_name = &entry->names[entry->size - 1];
			memset(sym_name, 0, sizeof(import_table_name_t));

			uint8_t is_ordinal = 0;
			if (il_stride == 4) {
				if (CHECK_BIT(il, HIGH_BIT32)) {
					sym_name->ordinal = (uint16_t)il;
					is_ordinal = 1;
				}
			} else {
				if (CHECK_BIT(il, HIGH_BIT64)) {
					sym_name->ordinal = (uint16_t)il;
					is_ordinal = 1;
				}
			}

			if (!is_ordinal) {
				size_t hint_offset = section_rva_to_offset(section, (uint32_t)il);
				if (hint_offset + 2 > section->contents_size) {
					ppelib_set_error("Symbol hint outside of section");
					goto out;
				}
				sym_name->hint = read_uint16_t(section->contents + hint_offset);

				size_t sym_name_offset = hint_offset + 2;
				size_t sym_name_max_size = section->contents_size - sym_name_offset;
				size_t sym_name_size = strnlen((const char *)section->contents + sym_name_offset, sym_name_max_size);
				if (sym_name_size == sym_name_max_size) {
					ppelib_set_error("Symbol name outside of section");
					goto out;
				}

				sym_name->name = strdup((const char *)section->contents + sym_name_offset);
			}
			ilt_offset += il_stride;
		}

		size_t dll_name_max_size = section->contents_size - dll_name_offset;
		size_t dll_name_size = strnlen((const char *)section->contents + dll_name_offset, dll_name_max_size);
		if (dll_name_size == dll_name_max_size) {
			ppelib_set_error("DLL name outside of section");
			goto out;
		}

		entry->dll_name = strdup((const char *)section->contents + dll_name_offset);

		scan_offset += IMPORT_DIRECTORY_TABLE_SIZE;
	}

	return;
out:
	--import_table->size;
	import_table_entry_free(&import_table->entries[import_table->size]);
	import_table->entries = realloc(import_table->entries, sizeof(import_table_entry_t) * import_table->size);
	return;
}
