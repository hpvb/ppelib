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

#include <ppelib/ppelib-low-level.h>
#include <ppelib/ppelib.h>

void write_header(ppelib_handle *pe, ppelib_header *header, const char *filename) {
	FILE *f = fopen(filename, "wb");
	ppelib_header_fprint(f, header);

	uint16_t sections = ppelib_header_get_number_of_sections(header);
	for (uint16_t i = 0; i < sections; ++i) {
		ppelib_section_fprint(f, ppelib_section_get(pe, i));
		fprintf(f, "\n");
	}
	fclose(f);
}

void write_headers(ppelib_handle *pe, ppelib_handle *pe2) {
	ppelib_header *header_new = ppelib_header_get(pe);
	;
	ppelib_header *header_orig = ppelib_header_get(pe2);

	write_header(pe, header_new, "header_new.txt");
	write_header(pe2, header_orig, "header_orig.txt");
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		printf("Usage: %s <filename>\n", argv[0]);
		return 1;
	}

	int retval = 0;

	ppelib_handle *pe = ppelib_create_from_file(argv[1]);
	if (ppelib_error()) {
		printf("PElib-error: %s\n", ppelib_error());
		return (1);
	}

	ppelib_handle *pe2 = ppelib_create_from_file(argv[1]);

	// Get original header
	ppelib_header *header_orig = ppelib_header_get(pe2);

	// Recalculate headers
	ppelib_recalculate(pe);
	ppelib_header *header_new = ppelib_header_get(pe);

	if (ppelib_header_compare(header_orig, header_new)) {
		printf("%s: Header mismatch for simple recalculate\n", argv[1]);

		write_headers(pe, pe2);
		retval = 1;
		goto out;
	} else {
		printf("%s: Header simple match\n", argv[1]);
	}

	// Recalculate headers
	ppelib_recalculate_force(pe);
	// Get new header
	header_new = ppelib_header_get(pe);

	if (ppelib_header_compare(header_orig, header_new)) {
		if (ppelib_header_compare_non_volitile(header_orig, header_new)) {
			printf("%s: Header force mismatch for file\n", argv[1]);

			write_headers(pe, pe2);
			retval = 1;
		} else {
			printf("%s: Header force match after accounting for unstable fields\n", argv[1]);
		}
	} else {
		printf("%s: Header force match\n", argv[1]);
	}

out:

	ppelib_destroy(pe);
	ppelib_destroy(pe2);

	return retval;
}
