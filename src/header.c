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
#include <string.h>
#include <stddef.h>
#include <stdlib.h>

#include "generated/header_private.h"

#include "ppe_error.h"
#include "platform.h"
#include "main.h"

EXPORT_SYM header_t* ppelib_header_get(ppelib_file_t *pe) {
	ppelib_reset_error();

	return &pe->header;
}

EXPORT_SYM header_t* ppelib_header_copy(header_t *header) {
	ppelib_reset_error();

	header_t *retval = malloc(sizeof(header_t));
	if (!retval) {
		ppelib_set_error("Could not allocate header copy");
		return NULL;
	}

	memcpy(retval, header, sizeof(header_t));
	retval->pe = NULL;
	return retval;
}

EXPORT_SYM void ppelib_header_free_copy(header_t *header) {
	ppelib_reset_error();

	free(header);
}

EXPORT_SYM uint32_t ppelib_header_compare(header_t *header1, header_t *header2) {
	ppelib_reset_error();

	header_t *header1_copy = ppelib_header_copy(header1);
	header_t *header2_copy = ppelib_header_copy(header2);

	header1_copy->modified = 0;
	header1_copy->pe = NULL;
	header2_copy->modified = 0;
	header2_copy->pe = NULL;

	uint32_t retval = memcmp(header1_copy, header2_copy, sizeof(header_t)) != 0;
	free(header1_copy);
	free(header2_copy);

	return retval;
}

EXPORT_SYM uint32_t ppelib_header_compare_non_volitile(header_t *header1, header_t *header2) {
	ppelib_reset_error();

	header_t *header1_copy = ppelib_header_copy(header1);
	header_t *header2_copy = ppelib_header_copy(header2);

	header1_copy->modified = 0;
	header1_copy->pe = NULL;
	header2_copy->modified = 0;
	header2_copy->pe = NULL;

	header1_copy->base_of_data = header2_copy->base_of_data;
	header1_copy->base_of_code = header2_copy->base_of_code;
	header1_copy->size_of_code = header2_copy->size_of_code;
	header1_copy->size_of_image = header2_copy->size_of_image;
	header1_copy->size_of_uninitialized_data = header2_copy->size_of_uninitialized_data;
	header1_copy->size_of_initialized_data = header2_copy->size_of_initialized_data;

	uint32_t retval = memcmp(header1_copy, header2_copy, sizeof(header_t)) != 0;
	free(header1_copy);
	free(header2_copy);

	return retval;
}

EXPORT_SYM void ppelib_set_header(ppelib_file_t *pe, header_t *header) {
	ppelib_reset_error();

	if (header->magic != PE32_MAGIC && header->magic != PE32PLUS_MAGIC) {
		ppelib_set_error("Unknown magic");
		return;
	}

	if (header->number_of_sections != pe->header.number_of_sections) {
		ppelib_set_error("number_of_sections mismatch");
		return;
	}

	if (header->number_of_rva_and_sizes != pe->header.number_of_rva_and_sizes) {
		ppelib_set_error("number_of_rva_and_sizes mismatch");
	}

	if (header->size_of_headers != pe->header.size_of_headers) {
		ppelib_set_error("size_of_headers mismatch");
	}

	memcpy(&pe->header, header, sizeof(header_t));
}
