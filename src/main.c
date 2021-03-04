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
#include <ppe_error.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ppelib/ppelib-constants.h>

#include "main.h"

EXPORT_SYM ppelib_file_t* ppelib_create() {
	ppelib_reset_error();

	ppelib_file_t *pe = calloc(sizeof(ppelib_file_t), 1);
	if (!pe) {
		ppelib_set_error("Failed to allocate PE structure");
	}

	return pe;
}

EXPORT_SYM void ppelib_destroy(ppelib_file_t *pe) {
	if (!pe) {
		return;
	}

	free(pe->trailing_data);

	free(pe);
}

EXPORT_SYM ppelib_file_t* ppelib_create_from_buffer(const uint8_t *buffer, size_t size) {
	ppelib_reset_error();

	if (size < PE_SIGNATURE_OFFSET + sizeof(uint32_t)) {
		ppelib_set_error("Not a PE file (file too small)");
		return NULL;
	}

	uint32_t header_offset = read_uint32_t(buffer + PE_SIGNATURE_OFFSET);
	if (size < header_offset + sizeof(uint32_t)) {
		ppelib_set_error("Not a PE file (file too small for PE signature)");
		return NULL;
	}

	uint32_t signature = read_uint32_t(buffer + header_offset);
	if (signature != PE_SIGNATURE) {
		ppelib_set_error("Not a PE file (PE00 signature missing)");
		return NULL;
	}

	ppelib_file_t *pe = ppelib_create();
	if (ppelib_error_peek()) {
		ppelib_destroy(pe);
		return NULL;
	}

	pe->pe_header_offset = header_offset;
	pe->coff_header_offset = header_offset + 4;

	if (size < pe->coff_header_offset + COFF_HEADER_SIZE) {
		ppelib_set_error("Not a PE file (file too small for COFF header)");
		ppelib_destroy(pe);
		return NULL;
	}

	ppelib_header_deserialize(buffer, pe->coff_header_offset, size, &pe->header);
	if (ppelib_error_peek()) {
		ppelib_destroy(pe);
		return NULL;
	}

	return pe;
}

EXPORT_SYM ppelib_file_t* ppelib_create_from_file(const char *filename) {
	ppelib_reset_error();
	size_t file_size;
	uint8_t *file_contents;

	FILE *f = fopen(filename, "rb");

	if (!f) {
		ppelib_set_error("Failed to open file");
		return NULL;
	}

	fseek(f, 0, SEEK_END);
	long ftell_size = ftell(f);
	rewind(f);

	if (ftell_size < 0) {
		fclose(f);
		ppelib_set_error("Unable to read file length");
		return NULL;
	}

	file_size = (size_t) ftell_size;

	if (!file_size) {
		fclose(f);
		ppelib_set_error("Empty file");
		return NULL;
	}

	file_contents = malloc(file_size);
	if (!file_size) {
		fclose(f);
		ppelib_set_error("Failed to allocate file data");
		return NULL;
	}

	size_t retsize = fread(file_contents, 1, file_size, f);
	if (retsize != file_size) {
		fclose(f);
		ppelib_set_error("Failed to read file data");
		return NULL;
	}

	fclose(f);

	ppelib_file_t *retval = ppelib_create_from_buffer(file_contents, file_size);
	free(file_contents);

	return retval;
}
