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

#include "ppelib-internal.h"

EXPORT_SYM uint32_t ppelib_has_signature(ppelib_file_t *pe) {
	ppelib_reset_error();

	if (pe->header.number_of_rva_and_sizes < DIR_CERTIFICATE_TABLE) {
		return 0;
	}

	if (pe->header.data_directories[DIR_CERTIFICATE_TABLE].size) {
		return 1;
	}

	return 0;
}

EXPORT_SYM void ppelib_signature_remove(ppelib_file_t *pe) {
	ppelib_reset_error();

	if (!ppelib_has_signature(pe)) {
		return;
	}

	if (pe->header.data_directories[DIR_CERTIFICATE_TABLE].virtual_address > pe->end_of_sections) {
		size_t offset = pe->header.data_directories[DIR_CERTIFICATE_TABLE].virtual_address - pe->end_of_sections;
		size_t size = pe->header.data_directories[DIR_CERTIFICATE_TABLE].size;

		if (buffer_excise(&pe->overlay, pe->overlay_size, offset, offset + size)) {
			ppelib_set_error("Failed to resize trailing data");
			return;
		}

		pe->overlay_size -= size;
	}

	ppelib_free_certificate_table(&pe->certificate_table);

	memset(&pe->data_directories[DIR_CERTIFICATE_TABLE], 0, sizeof(ppelib_data_directory_t));
	memset(&pe->header.data_directories[DIR_CERTIFICATE_TABLE], 0, sizeof(ppelib_header_data_directory_t));

	ppelib_recalculate(pe);
}
