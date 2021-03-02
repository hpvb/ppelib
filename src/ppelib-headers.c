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

#include <stdlib.h>
#include <string.h>

#include "ppelib-internal.h"

EXPORT_SYM ppelib_header_t* ppelib_get_header(ppelib_file_t *pe) {
	ppelib_reset_error();

	ppelib_header_t *retval = malloc(sizeof(ppelib_header_t));
	if (!retval) {
		ppelib_set_error("Unable to allocate header");
		return NULL;
	}

	memcpy(retval, &pe->header, sizeof(ppelib_header_t));
	return retval;
}

EXPORT_SYM void ppelib_set_header(ppelib_file_t *pe, ppelib_header_t *header) {
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

	memcpy(&pe->header, header, sizeof(ppelib_header_t));
}

EXPORT_SYM void ppelib_free_header(ppelib_header_t *header) {
	free(header);
}

