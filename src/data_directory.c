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

#include "main.h"
#include "platform.h"
#include "ppe_error.h"
#include "data_directory_private.h"

EXPORT_SYM uint32_t ppelib_data_directory_get_id(const data_directory_t *data_directory) {
	ppelib_reset_error();

	return data_directory->id;
}

EXPORT_SYM const section_t* ppelib_data_directory_get_section(const data_directory_t *data_directory) {
	ppelib_reset_error();

	return data_directory->section;
}

EXPORT_SYM size_t ppelib_data_directory_get_offset(const data_directory_t *data_directory) {
	ppelib_reset_error();

	return data_directory->offset;
}

EXPORT_SYM size_t ppelib_data_directory_get_size(const data_directory_t *data_directory) {
	ppelib_reset_error();

	return data_directory->size;
}

EXPORT_SYM uint32_t ppelib_data_directory_get_rva(const data_directory_t *data_directory) {
	ppelib_reset_error();

	section_t *section = data_directory->section;
	uint32_t rva = 0;

	if (section) {
		rva = data_directory->section->virtual_address + (uint32_t) data_directory->offset;
	} else {
		rva = (uint32_t) data_directory->offset;
	}

	return rva;
}

EXPORT_SYM const data_directory_t* ppelib_data_directory_get(ppelib_file_t *pe, uint32_t data_directory_index) {
	ppelib_reset_error();

	if (data_directory_index > pe->header.number_of_rva_and_sizes) {
		ppelib_set_error("Data directory index out of range");
		return NULL ;
	}

	return &pe->data_directories[data_directory_index];
}

EXPORT_SYM void ppelib_data_directory_fprint(FILE *stream, const data_directory_t *data_directory) {
	ppelib_reset_error();

	fprintf(stream, "Type: %s\n", map_lookup(data_directory->id, ppelib_data_directories_map));
	if (data_directory->section) {
		fprintf(stream, "Section: %s\n", ppelib_section_get_name(data_directory->section));
	} else if (data_directory->size) {
		fprintf(stream, "Section: After section data\n");
	} else {
		fprintf(stream, "Section: Empty\n");
	}

	fprintf(stream, "Offset: %zi\n", data_directory->offset);
	fprintf(stream, "Size: %zi\n", data_directory->size);
}

EXPORT_SYM void ppelib_data_directory_print(const data_directory_t *data_directory) {
	ppelib_data_directory_fprint(stdout, data_directory);
}
