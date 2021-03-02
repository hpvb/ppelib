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

#include <stdio.h>
#include <string.h>

#include <ppelib/ppelib.h>
#include <ppelib/ppelib-low-level.h>

int main(int argc, char *argv[]) {
	if (argc != 2) {
		printf("Usage: %s <infile> <outfile>\n", argv[0]);
		return 1;
	}

	int retval = 0;

	ppelib_handle *pe = ppelib_create_from_file(argv[1]);
	if (ppelib_error()) {
		printf("PElib-error: %s\n", ppelib_error());
		return (1);
	}

	// Get original header
	ppelib_header_t *header = ppelib_get_header(pe);

	// Recalculate headers
	ppelib_recalculate(pe);

	// Get new headers
	ppelib_header_t *header1 = ppelib_get_header(pe);
	ppelib_header_t *header2 = ppelib_get_header(pe);

	if (memcmp(header, header1, sizeof(ppelib_header_t))) {
		header1->base_of_data = header->base_of_data;
		header1->base_of_code = header->base_of_code;
		header1->size_of_code = header->size_of_code;
		header1->size_of_image = header->size_of_image;
		header1->size_of_uninitialized_data = header->size_of_uninitialized_data;
		header1->size_of_initialized_data = header->size_of_initialized_data;

		if (memcmp(header, header1, sizeof(ppelib_header_t))) {
			printf("%s: Header mismatch for file\n", argv[1]);

			FILE *f = fopen("header_orig.txt", "wb");
			ppelib_fprint_pe_header(f, header);
			fclose(f);

			f = fopen("header_new.txt", "wb");
			ppelib_fprint_pe_header(f, header2);
			fclose(f);

			retval = 1;
		} else {
			printf("%s: Header match after accounting for unstable fields\n", argv[1]);
		}
	} else {
		printf("%s: Header match\n", argv[1]);
	}

	ppelib_free_header(header);
	ppelib_free_header(header1);
	ppelib_free_header(header2);

	ppelib_destroy(pe);

	return retval;
}
